#include "SubmitScreen.hpp"
#include "Gfx.hpp"
#include "Utils.hpp"
#include "system/OTP.hpp"
#include "system/SEEPROM.hpp"
#include "system/MemoryDevice.hpp"

#include <coreinit/bsp.h>
#include <coreinit/mcp.h>

#include <curl/curl.h>
#include <cacert_pem.h>

#include <mocha/mocha.h>

struct IOSCEccSignedCert {
    uint32_t signature_type;    // [0x000] 0x00010002 or 0x00010005
    uint8_t signature[0x3C];    // [0x004] ECC signature
    uint8_t reserved1[0x40];    // [0x040] All zeroes
    char issuer[0x40];          // [0x080] Issuer
    uint32_t key_type;          // [0x0C0] Key type (2)
    char console_id[0x40];      // [0x0C4] Console ID ("NGxxxxxxxx")
    uint32_t ng_id;             // [0x104] NG ID
    uint8_t pub_key[0x3C];      // [0x108] ECC public key
    uint8_t reserved2[0x3C];    // [0x10C] All zeroes
};
WUT_CHECK_SIZE(IOSCEccSignedCert, 0x180);

// POST data
struct post_data {
    char system_model[16];      // [0x000] seeprom[0xB8]
    char system_serial[16];     // [0x010] seeprom[0xAC] + seeprom[0xB0] - need to mask the last 3 digits
    uint8_t mfg_date[6];        // [0x020] seeprom[0xC4]
    uint8_t productArea;        // [0x026]
    uint8_t gameRegion;         // [0x027]
    uint32_t sec_level;         // [0x028] otp[0x080]
    uint16_t boardType;         // [0x02C] seeprom[0x21]
    uint16_t boardRevision;     // [0x02E] seeprom[0x22]
    uint16_t bootSource;        // [0x030] seeprom[0x23]
    uint16_t ddr3Size;          // [0x032] seeprom[0x24]
    uint16_t ddr3Speed;         // [0x034] seeprom[0x25]
    uint16_t ddr3Vendor;        // [0x036] seeprom[0x29]
    uint16_t sataDevice;        // [0x038] seeprom[0x2C]
    uint16_t consoleType;       // [0x03A] seeprom[0x2D]
    uint32_t bsp_rev;           // [0x03C] bspGetHardwareVersion();
    uint32_t wiiu_root_ms_id;   // [0x040] otp[0x280]
    uint32_t wiiu_root_ca_id;   // [0x044] otp[0x284]
    uint32_t wiiu_ng_id;        // [0x048] otp[0x21C]
    uint32_t wiiu_ng_key_id;    // [0x04C] otp[0x288]
    uint8_t reserved2[48];      // [0x050]

    // [0x080]
    struct {
        uint32_t cid[4];        // [0x080] CID
        uint32_t mid_prv;       // [0x090] Manufacturer and product revision
        uint32_t blockSize;     // [0x094] Block size
        uint64_t numBlocks;     // [0x098] Number of blocks
    } mlc;

    // [0x0A0]
    uint8_t otp_sha256[32];     // [0x0A0] OTP SHA-256 hash (to prevent duplicates)

    // [0x0C0]
    IOSCEccSignedCert device_cert;  // [0x0C0] Device client certificate
};  // size == 0x240 (576)
WUT_CHECK_SIZE(post_data, 0x240);

struct post_data_hashed {
    struct post_data data;
    uint8_t post_sha256[32];    // [0x140] SHA-256 hash of post_data, with adjustments
};  // size == 0x260 (608)
WUT_CHECK_SIZE(post_data_hashed, 0x260);

namespace
{

const char desc[] =
    "This will submit statistical data to the developers of WiiUIdent,\n"
    "which will help to determine various statistics about Wii U consoles,\n"
    "e.g. eMMC manufacturers. The submitted data may be publicly accessible\n"
    "but personally identifying information will be kept confidential.\n"
    "\n"
    "Information that will be submitted:\n"
    "\uff65 System model and serial number (excluding the last 3 digits)\n"
    "\uff65 Manufacturing date\n"
    "\uff65 Region information\n"
    "\uff65 Security level (keyset), sataDevice, consoleType, BSP revision\n"
    "\uff65 boardType, boardRevision, bootSource, ddr3Size, ddr3Speed, ddr3Vendor\n"
    "\uff65 MLC manufacturer, revision, name, size, and CID\n"
    "\uff65 Device certificate and SHA-256 hash of OTP (to prevent duplicates)\n"
    "\uff64 MS, CA, NG, and NG key IDs\n"
    "\n"
    "Do you want to submit your console's system data?\n";
}

SubmitScreen::SubmitScreen()
{
}

SubmitScreen::~SubmitScreen()
{
}

void SubmitScreen::Draw()
{
    DrawTopBar("Submit System Information");

    if (state == STATE_INFO) {
        Gfx::Print(32, 75 + 32, 40, Gfx::COLOR_TEXT, desc);

        DrawBottomBar(nullptr, "\ue044 Exit", "\ue001 Back / \ue000 Submit");
    } else if (state == STATE_SUBMITTING) {
        Gfx::Print(Gfx::SCREEN_WIDTH / 2, Gfx::SCREEN_HEIGHT / 2, 64, Gfx::COLOR_TEXT, "Submitting info...", Gfx::ALIGN_CENTER);

        DrawBottomBar("Please wait...", nullptr, nullptr);
    } else if (state == STATE_SUBMITTED) {
        if (!error.empty() && response.empty()) {
            Gfx::Print(Gfx::SCREEN_WIDTH / 2, Gfx::SCREEN_HEIGHT / 2, 40, Gfx::COLOR_ERROR, Utils::sprintf("Error!\n%s", error.c_str()), Gfx::ALIGN_CENTER);

            Gfx::Print(Gfx::SCREEN_WIDTH / 2, Gfx::SCREEN_HEIGHT - 75 - 32, 40, Gfx::COLOR_TEXT,
                "Failed to submit system data. Please report a bug on GitHub:\n"
                "https://github.com/GaryOderNichts/WiiUIdent/issues", Gfx::ALIGN_HORIZONTAL | Gfx::ALIGN_BOTTOM);
        } else if (!response.empty()) {
            Gfx::Print(Gfx::SCREEN_WIDTH / 2, Gfx::SCREEN_HEIGHT / 2, 40, Gfx::COLOR_TEXT, response, Gfx::ALIGN_CENTER);

            if (!error.empty()) {
                Gfx::Print(Gfx::SCREEN_WIDTH / 2, Gfx::SCREEN_HEIGHT - 75 - 32, 40, Gfx::COLOR_TEXT,
                    "Failed to submit system data. Please report a bug on GitHub:\n"
                    "https://github.com/GaryOderNichts/WiiUIdent/issues", Gfx::ALIGN_HORIZONTAL | Gfx::ALIGN_BOTTOM);
            } else {
                Gfx::Print(Gfx::SCREEN_WIDTH / 2, Gfx::SCREEN_HEIGHT - 75 - 32, 40, Gfx::COLOR_TEXT,
                    "System data submitted successfully. Check out the Wii U console database at:\n"
                    "https://" DATABASE_URL "/", Gfx::ALIGN_HORIZONTAL | Gfx::ALIGN_BOTTOM);  
            }
        } else {
            Gfx::Print(Gfx::SCREEN_WIDTH / 2, Gfx::SCREEN_HEIGHT / 2, 64, Gfx::COLOR_TEXT, "No response.", Gfx::ALIGN_CENTER);
        }

        DrawBottomBar(nullptr, "\ue044 Exit", "\ue001 Back");
    }
}

bool SubmitScreen::Update(VPADStatus& input)
{
    if (state == STATE_INFO) {
        if (input.trigger & VPAD_BUTTON_A) {
            state = STATE_SUBMITTING;
        } else if (input.trigger & VPAD_BUTTON_B) {
            return false;
        }
    } else if (state == STATE_SUBMITTING) {
        SubmitSystemData();
        state = STATE_SUBMITTED;
    } else if (state == STATE_SUBMITTED) {
        if (input.trigger & VPAD_BUTTON_B) {
            return false;
        }
    }

    return true;
}

size_t SubmitScreen::CurlWriteCallback(void* contents, size_t size, size_t nmemb, void* userp)
{
    size_t realsize = size * nmemb;
    SubmitScreen* screen = (SubmitScreen*)userp;

    if (screen->response.size() < 4096) { // let's add a limit here to be safe
        screen->response += std::string((char*) contents, (char*) contents + realsize);
    }

    return realsize;
}

void SubmitScreen::SubmitSystemData()
{
    const auto& otp = OTP::Get();
    const auto& seeprom = SEEPROM::Get();
    WUT_ALIGNAS(0x40) MCPSysProdSettings sysProd{};
    int32_t mcpHandle = MCP_Open();
    if (mcpHandle >= 0) {
        // Don't bother checking res here, if it fails sysProd is zeroed
        MCP_GetSysProdSettings(mcpHandle, &sysProd);
        MCP_Close(mcpHandle);
    }
    const MemoryDevice* mlcDev = nullptr;
    for (const MemoryDevice& dev : MemoryDevice::GetDevices()) {
        if (dev.GetType() == MemoryDevice::TYPE_MLC) {
            mlcDev = &dev;
        }
    }

    post_data_hashed pdh{};
    post_data* pd = &pdh.data;
    memcpy(pd->system_model, seeprom.sys_prod.model_numer, sizeof(pd->system_model));
    memcpy(pd->mfg_date, &seeprom.prod_info.prod_year, sizeof(pd->mfg_date));
    pd->sec_level = otp.wiiUBank.securityLevel;
    pd->boardType = seeprom.bc.boardType;
    pd->boardRevision = seeprom.bc.boardRevision;
    pd->bootSource = seeprom.bc.bootSource;
    pd->ddr3Size = seeprom.bc.ddr3Size;
    pd->ddr3Speed = seeprom.bc.ddr3Speed;
    pd->ddr3Vendor = seeprom.bc.ddr3Vendor;
    pd->sataDevice = seeprom.bc.sataDevice;
    pd->consoleType = seeprom.bc.consoleType;
    pd->wiiu_root_ms_id = otp.wiiUCertBank.rootCertMSId;
    pd->wiiu_root_ca_id = otp.wiiUCertBank.rootCertCAId;
    pd->wiiu_ng_id = otp.wiiUNGBank.ngId;
    pd->wiiu_ng_key_id = otp.wiiUCertBank.rootCertNGKeyId;

    if (bspGetHardwareVersion(&pd->bsp_rev) != 0) {
        error = "Failed to get BSP revision";
        return;
    }

    // Device certificate
    // IOSC_GetDeviceCertificate() isn't directly accessible from PPC,
    // so we'll cheat by reading a known buffer in the kernel. (2.13.01)
    for (uint32_t i = 0; i < sizeof(pd->device_cert) / 4; i++) {
        if (Mocha_IOSUKernelRead32(0x04024d40 + (i * 4), ((uint32_t*)&pd->device_cert) + i) != MOCHA_RESULT_SUCCESS) {
            error = "Failed to read device certificate";
            return;
        }
    }

    // System serial number
    // NOTE: Assuming code+serial doesn't exceed 15 chars (plus NULL).
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wformat-truncation"
    snprintf(pd->system_serial, sizeof(pd->system_serial), "%s%s",
        seeprom.sys_prod.code_id, seeprom.sys_prod.serial_id);
#pragma GCC diagnostic pop

    // Mask the last 3 digits of the system serial number.
    for (unsigned int i = sizeof(pd->system_serial)-1; i > 3; i--) {
        if (pd->system_serial[i] <= 0x20) {
            pd->system_serial[i] = 0;
            continue;
        }
        // Found printable text.
        // Mask the last three digits.
        pd->system_serial[i-0] = '*';
        pd->system_serial[i-1] = '*';
        pd->system_serial[i-2] = '*';
        break;
    }

    if (sysProd.product_area) {
        pd->productArea = __builtin_ctz(sysProd.product_area);
    }
    pd->gameRegion = sysProd.game_region;

    if (mlcDev) {
        memcpy(pd->mlc.cid, mlcDev->GetCID().data(), sizeof(pd->mlc.cid));
        pd->mlc.mid_prv = mlcDev->GetMID() << 16 | mlcDev->GetPRV();
        pd->mlc.numBlocks = mlcDev->GetNumBlocks();
        pd->mlc.blockSize = mlcDev->GetBlockSize();
        // NOTE: Not copying pd->mlc.name1 because the eMMC device name
        // is fully contained within the MLC CID.
    }

    if (Utils::SHA256(&otp, sizeof(otp), pd->otp_sha256, sizeof(pd->otp_sha256)) != 0) {
        error = "Failed to hash otp data";
        return;
    }

    char hashbuf[sizeof(*pd) + 64];
    memcpy(hashbuf, pd, sizeof(*pd));
    memcpy(hashbuf + sizeof(*pd), "This will submit statistical data to the developers of recovery_menu", 64);
    if (Utils::SHA256(hashbuf, sizeof(hashbuf), pdh.post_sha256, sizeof(pdh.post_sha256)) != 0) {
        error = "Failed to hash data";
        return;
    }

    CURL* curl = curl_easy_init();
    if (!curl) {
        error = "Failed to init curl!";
        return;
    }

    // setup post
    curl_easy_setopt(curl, CURLOPT_USERAGENT, "WiiUIdent/" APP_VERSION);
    curl_easy_setopt(curl, CURLOPT_URL, "https://" DATABASE_URL "/add-system.php");
    curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, sizeof(pdh));
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, &pdh);

    // set content type to octet stream
    struct curl_slist* list = nullptr;
    list = curl_slist_append(list, "Content-Type: application/octet-stream");
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, list);

    // set write callback for response
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, CurlWriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, this);

    // load certs
    struct curl_blob blob{};
    blob.data = (void *) cacert_pem;
    blob.len = cacert_pem_size;
    blob.flags = CURL_BLOB_COPY;
    curl_easy_setopt(curl, CURLOPT_CAINFO_BLOB, &blob);

    CURLcode res = curl_easy_perform(curl);
    if (res != CURLE_OK) {
        error = Utils::sprintf("Failed to upload data:\n%s (%d)", curl_easy_strerror(res), res);
        curl_easy_cleanup(curl);
        return;
    }

    long code;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &code);
    if (code >= 400) {
        error = Utils::sprintf("Server responded with %d!", code);
    }

    curl_easy_cleanup(curl);
    return;
}
