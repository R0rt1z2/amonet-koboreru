#include <stdbool.h>
#include <stdint.h>

#include <mmio.h>

#include <drivers/pmic_keys.h>
#include <drivers/pwrap.h>

#define MT6323_CHRSTATUS 0x0142

#define CHRSTATUS_PWRKEY_DEB  BIT(1)
#define CHRSTATUS_HOMEKEY_DEB BIT(2)

bool pmic_key_pressed(uint32_t key)
{
    uint16_t mask;

    switch (key) {
    case PMIC_KEY_POWER:
        mask = CHRSTATUS_PWRKEY_DEB;
        break;
    case PMIC_KEY_HOME:
        mask = CHRSTATUS_HOMEKEY_DEB;
        break;
    default:
        return false;
    }

    return !(pwrap_read(MT6323_CHRSTATUS) & mask);
}
