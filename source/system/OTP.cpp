#include "OTP.hpp"

#include <mocha/mocha.h>

bool OTP::Init()
{
    if (Mocha_ReadOTP(&cachedOTP) != MOCHA_RESULT_SUCCESS) {
        return false;
    }

    return true;
}
