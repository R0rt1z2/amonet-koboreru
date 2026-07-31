#include <stdbool.h>
#include <stdint.h>

#include <mmio.h>
#include <platform.h>

#include <drivers/pmic_keys.h>
#include <drivers/pwrap.h>

bool pmic_key_pressed(uint32_t key)
{
    const struct pmic_keys_regs *kregs;
    
    if (key >= PMIC_KEY_NR)
        return false;

    kregs = &g_pmic_keys_regs[key];

    return !(pwrap_read(kregs->deb_reg) & kregs->deb_mask);
}
