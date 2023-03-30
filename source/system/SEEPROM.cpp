#include "SEEPROM.hpp"

#include <mocha/mocha.h>

bool SEEPROM::Init()
{
    if (Mocha_SEEPROMRead((uint8_t*) &cachedSEEPROMData, 0, sizeof(Data)) != sizeof(Data)) {
        return false;
    }

    return true;
}
