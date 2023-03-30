#include "StorageScreen.hpp"
#include "system/MemoryDevice.hpp"
#include "Utils.hpp"
#include <functional>

StorageScreen::StorageScreen()
{
    for (const MemoryDevice& dev : MemoryDevice::GetDevices()) {
        ScreenList* list = nullptr;
        if (dev.GetType() == MemoryDevice::TYPE_MLC) {
            list = &mlcList;
        } else if (dev.GetType() == MemoryDevice::TYPE_SD_CARD) {
            list = &sdList;
        } else {
            continue;
        }

        list->push_back({"Type:", dev.GetCardType() == MemoryDevice::CARD_TYPE_SD ? "SD" : "MMC"});
        list->push_back({"Manufacturer:", Utils::sprintf("%s (0x%02x)", dev.GetManufacturerName().c_str(), dev.GetMID())});
        list->push_back({"Product Name:", {dev.GetName(), true}});
        list->push_back({"Product Revision:", Utils::sprintf("%d.%d (0x%02x)", dev.GetPRV() >> 4, dev.GetPRV() & 0xf, dev.GetPRV())});
        if (dev.GetType() == MemoryDevice::TYPE_MLC)
            list->push_back({"Production Date:", dev.GetProductionDate()});
        list->push_back({"Size:", Utils::sprintf("%llu MiB", dev.GetTotalSize() / 1024ull / 1024ull)});
        list->push_back({"CID:", {Utils::ToHexString(dev.GetCID().data(), dev.GetCID().size()), true}});
        list->push_back({"CSD:", {Utils::ToHexString(dev.GetCSD().data(), dev.GetCSD().size()), true}});
    }

    if (mlcList.empty()) {
        mlcList.push_back({"Not Attached", ""});
    }

    if (sdList.empty()) {
        sdList.push_back({"Not Attached", ""});
    }
}

StorageScreen::~StorageScreen()
{
}

void StorageScreen::Draw()
{
    DrawTopBar("Storage Information");

    int yOff = 128;
    yOff = DrawHeader(32, yOff, 896, 0xf2db, "MLC");
    yOff = DrawList(32, yOff, 896, mlcList);

    yOff = 128;
    yOff = DrawHeader(992, yOff, 896, 0xf7c2, "SD Card");
    yOff = DrawList(992, yOff, 896, sdList);

    DrawBottomBar(nullptr, "\ue044 Exit", "\ue001 Back");
}

bool StorageScreen::Update(VPADStatus& input)
{
    if (input.trigger & VPAD_BUTTON_B) {
        return false;
    }

    return true;
}
