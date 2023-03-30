#pragma once

#include <mocha/otp.h>

class OTP
{
public:
    static bool Init();
    static const WiiUConsoleOTP& Get() { return cachedOTP; }

private:
    OTP() = default;
    virtual ~OTP() = default;

    static inline WiiUConsoleOTP cachedOTP;
};
