#pragma once

#include <cstdint>
#include <vector>
#include <array>
#include <string>

class MemoryDevice
{
public:
    virtual ~MemoryDevice() = default;
    MemoryDevice(const MemoryDevice&) = default; 	

    static bool Init();
    static const std::vector<MemoryDevice>& GetDevices() { return memoryDevices; }

public:
    enum Type {
        TYPE_MLC        = 0x05,
        TYPE_SD_CARD    = 0x06,
        // used instead of MLC if bsp_variant == 0x29 || bsp_variant == 0x21
        TYPE_UNKNOWN    = 0x12,
    };

    Type GetType() const { return type; }
    uint16_t GetMID() const { return mid; }
    uint16_t GetPRV() const { return prv; }
    std::string GetName() const { return name; }

    uint64_t GetNumBlocks() const { return numBlocks; }
    uint32_t GetBlockSize() const { return blockSize; }
    uint64_t GetTotalSize() const { return GetNumBlocks() * GetBlockSize(); }

    const std::array<uint8_t, 0x10>& GetCID() const { return cid; }
    const std::array<uint8_t, 0x10>& GetCSD() const { return csd; }

    enum CardType {
        CARD_TYPE_MMC,
        CARD_TYPE_SD,
        CARD_TYPE_UNKNOWN,
    };

    CardType GetCardType() const;
    std::string GetManufacturerName() const;
    std::string GetProductionDate() const;

private:
    MemoryDevice() = default;

    static inline std::vector<MemoryDevice> memoryDevices = {};

private:
    Type type;
    uint16_t mid;
    uint16_t prv;
    std::string name;
    uint64_t numBlocks;
    uint32_t blockSize;
    std::array<uint8_t, 0x10> cid;
    std::array<uint8_t, 0x10> csd;
};
