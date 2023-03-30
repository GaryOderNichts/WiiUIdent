#include "MemoryDevice.hpp"
#include "Utils.hpp"
#include <map>
#include <algorithm>

#include <mocha/mocha.h>

struct WUT_PACKED SALDeviceParams {
    uint32_t usrptr;
    uint32_t mid_prv;
    uint32_t device_type;
    WUT_UNKNOWN_BYTES(0x1c);
    uint64_t numBlocks;
    uint32_t blockSize;
    WUT_UNKNOWN_BYTES(0x18);
    char name0[128];
    char name1[128];
    char name2[128];
    uint32_t functions[12];
};

struct WUT_PACKED MDBlkDrv {
    int32_t registered;
    WUT_UNKNOWN_BYTES(0x8);
    SALDeviceParams params;
    int sal_handle;
    int deviceId;
    WUT_UNKNOWN_BYTES(0xc4);
};
WUT_CHECK_SIZE(MDBlkDrv, 0x2d4);

namespace
{

constexpr uint32_t MDBLK_DRIVER_ADDRESS         = 0x11c39e78;
constexpr uint32_t MD_DEVICE_POINTERS_ADDRESS   = 0x10899308;

// TODO turn this into a struct eventually
constexpr uint32_t MD_DEVICE_CID_OFFSET         = 0x58;
constexpr uint32_t MD_DEVICE_CSD_OFFSET         = 0x68;

const std::map<uint16_t, std::string> sdMidMap = {
    // from https://kernel.googlesource.com/pub/scm/utils/mmc/mmc-utils/+/refs/heads/master/lsmmc.c
	{ 0x01, "Panasonic" },
    { 0x02, "Toshiba/Kingston/Viking" },
    { 0x03, "SanDisk" },
    { 0x08, "Silicon Power" },
    { 0x18, "Infineon" },
    { 0x1b, "Transcend/Samsung" },
    { 0x1c, "Transcend" },
    { 0x1d, "Corsair/AData" },
    { 0x1e, "Transcend" },
    { 0x1f, "Kingston" },
    { 0x27, "Delkin/Phison" },
    { 0x28, "Lexar" },
    { 0x30, "SanDisk" },
    { 0x31, "Silicon Power" },
    { 0x33, "STMicroelectronics" },
    { 0x41, "Kingston" },
    { 0x6f, "STMicroelectronics" },
    { 0x74, "Transcend" },
    { 0x76, "Patriot" },
    { 0x82, "Gobe/Sony" },
    { 0x89, "Unknown" },
    { 0x9e, "PNY" },
};

const std::map<uint16_t, std::string> mmcMidMap = {
    { 0x11, "Toshiba" },
    // technically shared with Samsung/SanDisk/LG but Nintendo only used Samsung in Wii Us
    { 0x15, "Samsung" },
    { 0x90, "Hynix" }
};

}

bool MemoryDevice::Init()
{
    memoryDevices.clear();

    // read driver structs from IOS-FS
    MDBlkDrv blkDrvs[2]{};
    for (uint32_t i = 0; i < sizeof(blkDrvs) / 4; i++) {
        if (Mocha_IOSUKernelRead32(MDBLK_DRIVER_ADDRESS + (i * 4), ((uint32_t*) &blkDrvs) + i) != MOCHA_RESULT_SUCCESS) {
            return false;
        }
    }

    // read all SAL device pointers
    uint32_t devicePointers[8]{};
    for (uint32_t i = 0; i < sizeof(devicePointers) / 4; i++) {
        if (Mocha_IOSUKernelRead32(MD_DEVICE_POINTERS_ADDRESS + (i * 4), devicePointers + i) != MOCHA_RESULT_SUCCESS) {
            return false;
        }
    }

    // Create a MemoryDevice instance for each struct
    for (MDBlkDrv& drv : blkDrvs) {
        // don't bother adding it if not attached
        if (!drv.registered) {
            continue;
        }

        MemoryDevice dev{};
        dev.type = (Type) drv.params.device_type;
        dev.mid = drv.params.mid_prv >> 16;
        dev.prv = drv.params.mid_prv & 0xffff;
        dev.name = drv.params.name1;
        dev.numBlocks = drv.params.numBlocks;
        dev.blockSize = drv.params.blockSize;

        // some manufacturers place weird characters into their names?
        dev.name.erase(std::remove_if(dev.name.begin(), dev.name.end(), [](char c){return !(c >= 0 && c < 128);}), dev.name.end());

        // read cid and csd
        int idx = drv.deviceId - 0x42;
        if (idx >= 0 && idx < 8 && devicePointers[idx]) {
            for (uint32_t i = 0; i < dev.cid.size() / 4; i++) {
                if (Mocha_IOSUKernelRead32(devicePointers[idx] + MD_DEVICE_CID_OFFSET + (i * 4), (uint32_t*) dev.cid.data() + i) != MOCHA_RESULT_SUCCESS) {
                    return false;
                }
            }

            for (uint32_t i = 0; i < dev.csd.size() / 4; i++) {
                if (Mocha_IOSUKernelRead32(devicePointers[idx] + MD_DEVICE_CSD_OFFSET + (i * 4), (uint32_t*) dev.csd.data() + i) != MOCHA_RESULT_SUCCESS) {
                    return false;
                }
            }
        }

        memoryDevices.push_back(dev);
    }

    return true;
}

MemoryDevice::CardType MemoryDevice::GetCardType() const
{
    // if we have a mmc manufacturer for this mid it's most likely a mmc
    if (mmcMidMap.contains(GetMID())) {
        return CARD_TYPE_MMC;
    }

    // otherwise if we have a sd manufacturer it's probably an sd card
    if (sdMidMap.contains(GetMID())) {
        return CARD_TYPE_SD;
    }

    return CARD_TYPE_UNKNOWN;
}

std::string MemoryDevice::GetManufacturerName() const
{
    CardType cardType = GetCardType();
    if (cardType == CARD_TYPE_MMC) {
        return mmcMidMap.at(GetMID());
    } else if (cardType == CARD_TYPE_SD) {
        return sdMidMap.at(GetMID());
    }

    return "Unknown";
}

std::string MemoryDevice::GetProductionDate() const
{
    if (GetCardType() != CARD_TYPE_MMC) {
        // we currently don't read the production data for non mmc types
        return "";
    }

    uint8_t month = (cid[14] >> 4) & 0xf;
    uint16_t year = cid[14] & 0xf;
    year += 1997;
    if(year < 2005)
        year += 0x10;
    
    return Utils::sprintf("%u/%02u", year, month);
}
