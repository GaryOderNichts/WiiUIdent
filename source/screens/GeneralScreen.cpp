#include "GeneralScreen.hpp"
#include "Gfx.hpp"
#include "Utils.hpp"
#include "system/OTP.hpp"
#include "system/SEEPROM.hpp"
#include <map>

#include <coreinit/mcp.h>
#include <coreinit/bsp.h>

extern "C"
{

typedef struct {
    uint32_t major;
    uint32_t minor;
    uint32_t patch;
    char region;
} MCPSystemVersion;

MCPError MCP_GetSystemVersion(int32_t handle, MCPSystemVersion* systemVersion);

}

namespace
{

static const char* const consoleTypeLookup[] = {
    "Invalid", "WUP", "CAT-R", "CAT-DEV",
    "EV board", "Kiosk", "OrchestraX", "WUIH",
    "WUIH_DEV", "CAT_DEV_WUIH",
};

const char* const keysetLookup[] = {
    "Factory",
    "Debug",
    "Retail",
    "Invalid"
};

const std::map<uint32_t, const char*> hardwareVersionMap = {
    { 0x21100010, "LATTE_A11_EV"   },
    { 0x21100020, "LATTE_A11_CAT"  },
    { 0x21200010, "LATTE_A12_EV"   },
    { 0x21200020, "LATTE_A12_CAT"  },
    { 0x22100010, "LATTE_A2X_EV"   },
    { 0x22100020, "LATTE_A2X_CAT"  },
    { 0x23100010, "LATTE_A3X_EV"   },
    { 0x23100020, "LATTE_A3X_CAT"  },
    { 0x23100028, "LATTE_A3X_CAFE" },
    { 0x24100010, "LATTE_A4X_EV"   },
    { 0x24100020, "LATTE_A4X_CAT"  },
    { 0x24100028, "LATTE_A4X_CAFE" },
    { 0x25100010, "LATTE_A5X_EV"   },
    { 0x25100011, "LATTE_A5X_EV_Y" },
    { 0x25100020, "LATTE_A5X_CAT"  },
    { 0x25100028, "LATTE_A5X_CAFE" },
    { 0x26100010, "LATTE_B1X_EV"   },
    { 0x26100011, "LATTE_B1X_EV_Y" },
    { 0x26100020, "LATTE_B1X_CAT"  },
    { 0x26100028, "LATTE_B1X_CAFE" },
};

const char* const sataDeviceLookup[] = {
    "Invalid", "Default", "No device", "ROM drive",
    "R drive", "MION", "SES", "GEN2-HDD",
    "GEN1-HDD",
};

const char* const regionLookup[] = {
    "JPN", "USA", "EUR", "AUS",
    "CHN", "KOR", "TWN",
};

}

GeneralScreen::GeneralScreen()
{
    const auto& otp = OTP::Get();
    const auto& seeprom = SEEPROM::Get();

    WUT_ALIGNAS(0x40) MCPSysProdSettings sysProd{};
    WUT_ALIGNAS(0x40) MCPSystemVersion sysVer{};
    int32_t mcpHandle = MCP_Open();
    if (mcpHandle >= 0) {
        // Don't bother checking res here, if it fails sysProd is zeroed
        MCP_GetSysProdSettings(mcpHandle, &sysProd);
        MCP_GetSystemVersion(mcpHandle, &sysVer);
        MCP_Close(mcpHandle);
    }

    sysIdentList.push_back({"Model:", Utils::sprintf("%.*s", sizeof(sysProd.model_number), sysProd.model_number)});
    sysIdentList.push_back({"Serial:", Utils::sprintf("%.*s%.*s", sizeof(sysProd.code_id), sysProd.code_id, sizeof(sysProd.serial_id), sysProd.serial_id)});
    sysIdentList.push_back({"Production Date:", 
        Utils::sprintf("%04x/%02x/%02x %02x:%02x",
            seeprom.prod_info.prod_year,
            seeprom.prod_info.prod_month_day >> 8, 
            seeprom.prod_info.prod_month_day & 0xff,
            seeprom.prod_info.prod_hour_minute >> 8, 
            seeprom.prod_info.prod_hour_minute & 0xff
        )
    });
    if (seeprom.bc.consoleType < 10) {
        hardwareList.push_back({"Type:", Utils::sprintf("%s (0x%02x)", consoleTypeLookup[seeprom.bc.consoleType], seeprom.bc.consoleType)});
    }
    sysIdentList.push_back({"Keyset:", keysetLookup[(otp.wiiUBank.securityLevel & 0x18000000) >> 27]});

    BSPHardwareVersion hardwareVersion = 0;
    bspGetHardwareVersion(&hardwareVersion);
    const char* hardwareVersionString = 
        hardwareVersionMap.contains(hardwareVersion) ? hardwareVersionMap.at(hardwareVersion)
        : "Unknown";
    hardwareList.push_back({"Version:", Utils::sprintf("%s", hardwareVersionString)});
    hardwareList.push_back({"", Utils::sprintf("(0x%08x)", hardwareVersion)});
    hardwareList.push_back({"Board Type:", Utils::sprintf("%c%c", seeprom.bc.boardType >> 8, seeprom.bc.boardType & 0xff)});
    if (seeprom.bc.sataDevice < 9) {
        hardwareList.push_back({"SATA Device:", Utils::sprintf("%s (0x%02x)", sataDeviceLookup[seeprom.bc.sataDevice], seeprom.bc.sataDevice)});
    }
    hardwareList.push_back({"DDR3 Size:", Utils::sprintf("%d MiB", seeprom.bc.ddr3Size)});
    hardwareList.push_back({"DDR3 Speed:", Utils::sprintf("%d", seeprom.bc.ddr3Speed)});
    hardwareList.push_back({"DDR3 Vendor:", Utils::sprintf("0x%02X", seeprom.bc.ddr3Vendor)});

    const int productAreaId = sysProd.product_area ? __builtin_ctz(sysProd.product_area) : 7;
    if (productAreaId < 7) {
        regionList.push_back({"Product Area:", regionLookup[productAreaId]});
    }
    regionList.push_back({"Game Region:", ""});
    regionList.push_back({"",
        Utils::sprintf("%s %s %s %s %s %s (%u)",
            (sysProd.game_region & MCP_REGION_JAPAN)  ? regionLookup[0] : "---",
            (sysProd.game_region & MCP_REGION_USA)    ? regionLookup[1] : "---",
            (sysProd.game_region & MCP_REGION_EUROPE) ? regionLookup[2] : "---",
            (sysProd.game_region & MCP_REGION_CHINA)  ? regionLookup[4] : "---",
            (sysProd.game_region & MCP_REGION_KOREA)  ? regionLookup[5] : "---",
            (sysProd.game_region & MCP_REGION_TAIWAN) ? regionLookup[6] : "---", sysProd.game_region)
    });

    versionList.push_back({"System Version:", Utils::sprintf("%u.%u.%u%c", sysVer.major, sysVer.minor, sysVer.patch, sysVer.region)});

    // Need to decrypt boot1 info before using
    SEEPROM::Boot1Info boot1_info[2]{};
    if (Utils::AESDecrypt(otp.wiiUBank.seepromKey, 128, nullptr, &seeprom.boot1_info[0], &boot1_info[0], sizeof(SEEPROM::Boot1Info)) == 0 &&
        Utils::AESDecrypt(otp.wiiUBank.seepromKey, 128, nullptr, &seeprom.boot1_info[1], &boot1_info[1], sizeof(SEEPROM::Boot1Info)) == 0) {
        versionList.push_back({"Boot1 Version (0/1):", Utils::sprintf("0x%04x (%u)", boot1_info[0].version, boot1_info[0].version)});
        versionList.push_back({"Boot1 Version (1/1):", Utils::sprintf("0x%04x (%u)", boot1_info[1].version, boot1_info[1].version)});
    }
}

GeneralScreen::~GeneralScreen()
{

}

void GeneralScreen::Draw()
{
    DrawTopBar("General System Information");

    int yOff = 128;
    yOff = DrawHeader(32, yOff, 896, 0xf02a, "Identification");
    yOff = DrawList(32, yOff, 896, sysIdentList);

    yOff = DrawHeader(32, yOff, 896, 0xf538, "Hardware");
    yOff = DrawList(32, yOff, 896, hardwareList);

    yOff = 128;
    yOff = DrawHeader(992, yOff, 896, 0xf0ac, "Region");
    yOff = DrawList(992, yOff, 896, regionList);

    yOff = DrawHeader(992, yOff, 896, 0xf886, "Versions");
    yOff = DrawList(992, yOff, 896, versionList);

    DrawBottomBar(nullptr, "\ue044 Exit", "\ue001 Back");
}

bool GeneralScreen::Update(VPADStatus& input)
{
    if (input.trigger & VPAD_BUTTON_B) {
        return false;
    }

    return true;
}
