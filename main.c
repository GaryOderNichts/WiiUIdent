#include <whb/proc.h>
#include <whb/log.h>
#include <whb/log_udp.h>
#include <whb/log_console.h>

#include <iosuhax.h>

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
    uint8_t unk2[204];
};
static_assert(sizeof(struct MDBlkDriver) == 724, "MDBlkDriver: wrong size");

#define MDBLK_DRIVER_ADDRESS 0x11c39e78

struct MDBlkDriver blkDrivers[2] = { 0 };

int main(int argc, char const *argv[])
{
    WHBProcInit();
    WHBLogUdpInit();
    WHBLogConsoleInit();

    if (IOSUHAX_Open(NULL) >= 0) {
        if (IOSUHAX_kern_read32(MDBLK_DRIVER_ADDRESS, (uint32_t *) blkDrivers, sizeof(blkDrivers) / 4) >= 0) {
            for (int i = 0; i < 2; ++i) {
                struct MDBlkDriver *drv = &blkDrivers[i];
                WHBLogPrintf("** Instance %d: (%s) **", i + 1, drv->registered ? "Attached" : "Detached");
                if (drv->registered) {
                    uint16_t mid = drv->params.mid_prv >> 16;
                    uint16_t prv = drv->params.mid_prv & 0xff;
                    struct ids_database *db = find_by_id(mid);

                    WHBLogPrintf("Manufacturer ID: 0x%02x, Product revision: 0x%02x", mid, prv);
                    WHBLogPrintf(" -> Manufacturer: '%s' Type: '%s'", db->manufacturer, db->type);
                    WHBLogPrintf("Name 0: '%s' Name 1: '%s' Name 2: '%s'", drv->params.name0, drv->params.name1, drv->params.name2);

                    /*
                    for (int j = 0; j < 4; ++j) {
                        WHBLogPrintf("Unk %d: %08x %08x %08x %08x", j, drv->params.unk[j * 4 + 0], drv->params.unk[j * 4 + 1], drv->params.unk[j * 4 + 2], drv->params.unk[j * 4 + 3]);
                    }
                    */
                }

                WHBLogPrintf("=================================================");
            }
        } else {
            WHBLogPrintf("Failed to read driver data");
        }

        IOSUHAX_Close();
    } else {
        WHBLogPrintf("Failed to open IOSUHAX");
    }

    while (WHBProcIsRunning()) {
        WHBLogConsoleDraw();
    }

    WHBLogConsoleFree();
    WHBLogUdpDeinit();
    WHBProcShutdown();
    return 0;
}
