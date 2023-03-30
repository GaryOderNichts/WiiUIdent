#pragma once

#include <string>
#include <memory>

namespace Utils
{

template<typename ...Args>
std::string sprintf(const std::string& format, Args ...args)
{
    int size = std::snprintf(nullptr, 0, format.c_str(), args ...) + 1;

    std::unique_ptr<char[]> buf(new char[size]);
    std::snprintf(buf.get(), size, format.c_str(), args ...);

    return std::string(buf.get(), buf.get() + size - 1);
}

std::string ToHexString(const void* data, size_t size);

int AESDecrypt(const void* key, uint32_t keybits, const void* iv, const void* in, void* out, size_t dataSize);

int SHA256(const void* in, size_t inSize, void* out, size_t outSize);

}
