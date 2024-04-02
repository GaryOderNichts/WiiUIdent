#include "DRXInfoScreen.hpp"
#include "Utils.hpp"
#include <span>

#include <nsysccr/cdc.h>
#include <nsysccr/cfg.h>

namespace {

bool GetEepromValue(uint32_t offset, std::span<uint8_t> value)
{
    uint8_t data[value.size() + 2];
    if (CCRCFGGetCachedEeprom(0, offset, data, value.size() + 2) != 0) {
        return false;
    }

    uint16_t crc = (uint16_t) data[value.size() + 1] << 8 | data[value.size()];
    if (CCRCDCCalcCRC16(data, value.size()) != crc) {
        return false;
    }

    std::copy(data, data + value.size(), value.begin());
    return true;
}

// from gamepad firmare @0x000b2990
const char* kBoardMainVersions[] = {
    "DK1",
    "DK1 / EP / DK2",
    "DP1",
    "DP2",
    "DK3",
    "DK4",
    "PreDP3 / DP3",
    "DK5",
    "DP4",
    "DKMP",
    "DP5",
    "MASS",
    "DKMP2",
    "DRC-I",
    "DKTVMP",
};

// from gamepad firmare @0x000b29cc
const char* kBoardSubVersions[] = {
    "DK1 / EP / DK2",
    "DP1 / DK3",
    "DK4",
    "DP3",
    "DK5",
    "DP4",
    "DKMP",
    "DP5",
    "MASS",
    "DKMP2",
    "DRC-I",
    "DKTVMP"
};

// from gamepad firmare @0x000b29fc
const char* kRegionStrings[] = {
    "JAPAN",
    "AMERICA",
    "EUROPE",
    "CHINA",
    "SOUTH KOREA",
    "TAIWAN",
    "AUSTRALIA",
};

}

DRXInfoScreen::DRXInfoScreen()
{
    CCRCDCSoftwareVersion softwareVersion;
    if (CCRCDCSoftwareGetVersion(CCR_CDC_DESTINATION_DRC0, &softwareVersion) == 0) {
        uint32_t v = softwareVersion.runningVersion;
        mDRCList.push_back({"Running Version:", Utils::sprintf("%d.%d.%d", v >> 24 & 0xff, v >> 16 & 0xff, v & 0xffff)});
        mDRCList.push_back({"", {Utils::sprintf("(0x%08x)", v), true}});
        v = softwareVersion.activeVersion;
        mDRCList.push_back({"Active Version:", Utils::sprintf("%d.%d.%d", v >> 24 & 0xff, v >> 16 & 0xff, v & 0xffff)});
        mDRCList.push_back({"", {Utils::sprintf("(0x%08x)", v), true}});
    } else {
        mDRCList.push_back({"CCRCDCSoftwareGetVersion failed", ""});
    }

    uint8_t boardInfo;
    if (GetEepromValue(0x100, std::span(std::addressof(boardInfo), 1))) {
        uint8_t mainVersion = boardInfo & 0xf;
        uint8_t subVersion = boardInfo >> 4;
        mDRCList.push_back({"Board Version:", Utils::sprintf("%d.%d (0x%02x)", mainVersion, subVersion, boardInfo)});
        mDRCList.push_back({"", Utils::sprintf("(%s / %s)",
            mainVersion < 0xf ? kBoardMainVersions[mainVersion] : "UNKNOWN",
            subVersion < 0xc ? kBoardSubVersions[subVersion] : "UNKNOWN")});
    } else {
        mDRCList.push_back({"GetEepromValue failed", ""});
    }

    uint8_t region;
    if (GetEepromValue(0x103, std::span(std::addressof(region), 1))) {
        mDRCList.push_back({"Region:", Utils::sprintf("%s (0x%02x)",
            region < 0x7 ? kRegionStrings[region] : "UNKNOWN", region)});
    } else {
        mDRCList.push_back({"GetRegion failed", ""});
    }

    if (CCRCDCSoftwareGetVersion(CCR_CDC_DESTINATION_DRH, &softwareVersion) == 0) {
        uint32_t v = softwareVersion.runningVersion;
        mDRHList.push_back({"Running Version:", Utils::sprintf("%d.%d.%d", v >> 24 & 0xff, v >> 16 & 0xff, v & 0xffff)});
        mDRHList.push_back({"", {Utils::sprintf("(0x%08x)", v), true}});
        v = softwareVersion.activeVersion;
        mDRHList.push_back({"Active Version:", Utils::sprintf("%d.%d.%d", v >> 24 & 0xff, v >> 16 & 0xff, v & 0xffff)});
        mDRHList.push_back({"", {Utils::sprintf("(0x%08x)", v), true}});
    } else {
        mDRHList.push_back({"CCRCDCSoftwareGetVersion failed", ""});
    }

    uint32_t extId;
    if (CCRCDCSoftwareGetExtId(CCR_CDC_DESTINATION_DRC0, CCR_CDC_EXT_LANGUAGE, &extId) == 0) {
        mExtIdList.push_back({"Language:", {Utils::sprintf("0x%08x", extId), true}});
        mExtIdList.push_back({"", Utils::sprintf("(version: %04d bank: %02d)", extId >> 8 & 0xffff, extId >> 24)});
    } else {
        mExtIdList.push_back({"CCRCDCSoftwareGetExtId(CCR_CDC_EXT_LANGUAGE) failed", ""});
    }

    if (CCRCDCSoftwareGetExtId(CCR_CDC_DESTINATION_DRC0, CCR_CDC_EXT_RC_DATABASE, &extId) == 0) {
        mExtIdList.push_back({"RC Database:", {Utils::sprintf("0x%08x", extId), true}});
    } else {
        mExtIdList.push_back({"CCRCDCSoftwareGetExtId(CCR_CDC_EXT_RC_DATABASE) failed", ""});
    }

    for (int i = CCR_CDC_EXT_UNK2; i <= CCR_CDC_EXT_UNK4; i++) {
        if (CCRCDCSoftwareGetExtId(CCR_CDC_DESTINATION_DRC0, (CCRCDCExt) i, &extId) == 0) {
            mExtIdList.push_back({Utils::sprintf("ID %d:", i), {Utils::sprintf("0x%08x", extId), true}});
        } else {
            mExtIdList.push_back({Utils::sprintf("CCRCDCSoftwareGetExtId(%d) failed", i), ""});
        }
    }
}

DRXInfoScreen::~DRXInfoScreen()
{
}

void DRXInfoScreen::Draw()
{
    DrawTopBar("DRC/DRH Information");

    int yOff = 128;
    yOff = DrawHeader(32, yOff, 896, 0xf11b, "DRC Info");
    yOff = DrawList(32, yOff, 896, mDRCList);
    yOff = DrawHeader(32, yOff, 896, 0xf0cb, "DRC Ext IDs");
    yOff = DrawList(32, yOff, 896, mExtIdList);

    yOff = 128;
    yOff = DrawHeader(992, yOff, 896, 0xf2db, "DRH Info");
    yOff = DrawList(992, yOff, 896, mDRHList);

    DrawBottomBar(nullptr, "\ue044 Exit", "\ue001 Back");
}

bool DRXInfoScreen::Update(VPADStatus& input)
{
    if (input.trigger & VPAD_BUTTON_B) {
        return false;
    }

    return true;
}
