#include <platform.h>

#include <drivers/pmic_keys.h>

#define MT6392_CHRSTATUS 0x0142

const struct pmic_keys_regs g_pmic_keys_regs[PMIC_KEY_NR] = {
	[PMIC_KEY_POWER] =
		MTK_PMIC_KEY_REGS(MT6392_CHRSTATUS, 0x2),
	[PMIC_KEY_HOME] =
		MTK_PMIC_KEY_REGS(MT6392_CHRSTATUS, 0x4),
};
