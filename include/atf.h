#pragma once

#include <stdint.h>

#include <device.h>

/* 'F','A','D','E','O','U','T','Z'. */
#define ATF_UPSTREAM_MAGIC0     0x45444146
#define ATF_UPSTREAM_MAGIC1     0x5A54554F
#define ATF_UPSTREAM_MAGIC_SIZE 8

int atf_note_image(uint32_t addr, int raw);
void bldr_jump64(uint32_t addr, uint32_t arg1, uint32_t arg2);