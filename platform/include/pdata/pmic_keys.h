#pragma once

#define MTK_PMIC_KEY_REGS(_deb_reg, _deb_mask)	\
{								\
	.deb_reg		= _deb_reg,			\
	.deb_mask		= _deb_mask,			\
}

struct pmic_keys_regs {
	uint32_t deb_reg;
	uint32_t deb_mask;
};
