#include <whb/proc.h>
#include <whb/log.h>
#include <whb/log_udp.h>
#include <whb/log_console.h>

#include <mocha/mocha.h>

#include "idsdb.h"

struct SALDeviceParams {
    uint32_t usrptr;
    uint32_t mid_prv;
    uint32_t device_type;
    uint32_t unk[16];
    char name0[128];
    char name1[128];
    char name2[128];
    uint32_t functions[12];
};

struct MDBlkDriver {
    int32_t registered;
    int32_t unk[2];
    struct SALDeviceParams params;
    int sal_handle;
    int deviceId;
    uint8_t unk2[196];
};
static_assert(sizeof(struct MDBlkDriver) == 724, "MDBlkDriver: wrong size");

#define MDBLK_DRIVER_ADDRESS 0x11c39e78
#define MD_DEVICE_POINTERS_ADDRESS 0x10899308

struct MDBlkDriver blkDrivers[2] = { 0 };

MochaUtilsStatus iosuKernRead32(uint32_t address, uint32_t* outBuffer, int count)
{
    MochaUtilsStatus status = MOCHA_RESULT_SUCCESS;

    for (int i = 0; i < count; ++i) {
        status = Mocha_IOSUKernelRead32(address + (i * 4), outBuffer + i);
        if (status != MOCHA_RESULT_SUCCESS) {
            break;
        }
    }

    return status;
}

static uint32_t getDeviceAddressByID(int id)
{
    if (id - 0x42 >= 8) {
        return 0;
    }

    uint32_t devicePointers[8];
    if (iosuKernRead32(MD_DEVICE_POINTERS_ADDRESS, devicePointers, 8) != MOCHA_RESULT_SUCCESS) {
        return 0;
    }

    // virtual matches physical address for IOS-FS, no need to conversion
    return devicePointers[id - 0x42];
}

int main(int argc, char const *argv[])
{
    WHBProcInit();
    WHBLogUdpInit();
    WHBLogConsoleInit();

    if (Mocha_InitLibrary() == MOCHA_RESULT_SUCCESS) {
        if (iosuKernRead32(MDBLK_DRIVER_ADDRESS, (uint32_t *) blkDrivers, sizeof(blkDrivers) / 4) == MOCHA_RESULT_SUCCESS) {
            for (int i = 0; i < 2; ++i) {
                struct MDBlkDriver *drv = &blkDrivers[i];
                WHBLogPrintf("** Instance %d: (%s) Type: %d **", i + 1, drv->registered ? "Attached" : "Detached", drv->params.device_type);
                if (drv->registered) {
                    uint16_t mid = drv->params.mid_prv >> 16;
                    uint16_t prv = drv->params.mid_prv & 0xff;
                    struct ids_database *db = find_by_id(mid);

                    WHBLogPrintf("Manufacturer ID: 0x%02x, Product revision: 0x%02x", mid, prv);
                    WHBLogPrintf(" -> Manufacturer: '%s' Type: '%s'", db->manufacturer, db->type);
                    WHBLogPrintf("Name 0: '%s' Name 1: '%s' Name 2: '%s'", drv->params.name0, drv->params.name1, drv->params.name2);

                    uint32_t deviceAddress = getDeviceAddressByID(drv->deviceId);
                    if (!deviceAddress) {
                        continue;
                    }

                    uint32_t cid[4];
                    if (iosuKernRead32(deviceAddress + 0x58, cid, 4) != MOCHA_RESULT_SUCCESS) {
                        continue;
                    }

                    WHBLogPrintf("CID: %08x%08x%08x%08x", cid[0], cid[1], cid[2], cid[3]);

                    uint32_t csd[4];
                    if (iosuKernRead32(deviceAddress + 0x68, csd, 4) != MOCHA_RESULT_SUCCESS) {
                        continue;
                    }

                    WHBLogPrintf("CSD: %08x%08x%08x%08x", csd[0], csd[1], csd[2], csd[3]);
                }

                WHBLogPrintf("=================================================");
            }
        } else {
            WHBLogPrintf("Failed to read driver data");
        }

        Mocha_DeInitLibrary();
    } else {
        WHBLogPrintf("Failed to initialize Mocha");
    }

    while (WHBProcIsRunning()) {
        WHBLogConsoleDraw();
    }

    WHBLogConsoleFree();
    WHBLogUdpDeinit();
    WHBProcShutdown();
    return 0;
}
