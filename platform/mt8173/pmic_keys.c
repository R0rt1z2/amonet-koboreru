#include <platform.h>

#include <drivers/pmic_keys.h>

#define MT6397_CHRSTATUS 0x0144
#define MT6397_OCSTATUS2 0x014A

const struct pmic_keys_regs g_pmic_keys_regs[PMIC_KEY_NR] = {
	[PMIC_KEY_POWER] =
		MTK_PMIC_KEY_REGS(MT6397_CHRSTATUS, 0x8),
	[PMIC_KEY_HOME] =
		MTK_PMIC_KEY_REGS(MT6397_OCSTATUS2, 0x10),
};
