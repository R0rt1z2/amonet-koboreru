CC := arm-none-eabi-gcc
AS := arm-none-eabi-as
LD := arm-none-eabi-gcc
OBJCOPY := arm-none-eabi-objcopy

BUILD_ROOT := ./build
TARGET := payload

DEVICES := $(sort $(patsubst include/devices/%.h,%,$(wildcard include/devices/*.h)))

ifeq ($(DEVICE),)
.DEFAULT_GOAL := all
else
.DEFAULT_GOAL := $(TARGET)
endif

.PHONY: all list-targets clean $(DEVICES)

all: $(DEVICES)

$(DEVICES):
	@$(MAKE) --no-print-directory DEVICE=$@ $(TARGET)

list-targets:
	@echo "Device targets:"
	@$(foreach d,$(DEVICES),printf '  %s\n' '$(d)';)
	@echo ""
	@echo "Other targets:"
	@printf '  %-13s %s\n' 'all'          'build every device (default)'
	@printf '  %-13s %s\n' 'list-targets' 'show this list'
	@printf '  %-13s %s\n' 'clean'        'remove $(BUILD_ROOT), or one device with DEVICE='

clean:
ifeq ($(DEVICE),)
	-rm -rf $(BUILD_ROOT)
else
	-rm -rf $(BUILD_ROOT)/$(DEVICE)
endif

ifneq ($(DEVICE),)
# Okay, this is the real run and DEVICE is set, either from the user
# manually, or when we are invoked as a sub-make from the targets above.

DEVICE_HEADER := include/devices/$(DEVICE).h
DEVICE_SRC := devices/$(DEVICE).c

ifeq ($(wildcard $(DEVICE_HEADER)),)
$(error unknown device '$(DEVICE)', no $(DEVICE_HEADER). Try: make list-targets)
endif

ifeq ($(wildcard $(DEVICE_SRC)),)
$(error unknown device '$(DEVICE)', no $(DEVICE_SRC). Try: make list-targets)
endif

PAYLOAD_ADDR := $(shell sed -n 's/^#define PAYLOAD_ADDR *//p' $(DEVICE_HEADER))
BSS_START := $(shell sed -n 's/^#define BSS_START *//p' $(DEVICE_HEADER))

UART_BASE ?= 0x11002000
WDT_BASE ?= 0x10007000
APXGPT_BASE ?= 0x10008000
PWRAP_BASE ?= 0x1000D000
RTC_BASE ?= 0x8000

INCLUDES := -I./include
DEFINES := -DDEVICE_HEADER=\"devices/$(DEVICE).h\" \
           -DUART_BASE=$(UART_BASE) \
           -DWDT_BASE=$(WDT_BASE) \
           -DAPXGPT_BASE=$(APXGPT_BASE) \
           -DPWRAP_BASE=$(PWRAP_BASE) \
           -DRTC_BASE=$(RTC_BASE)

CFLAGS := $(INCLUDES) $(DEFINES) -std=gnu99 -Os -mthumb -mcpu=cortex-a9 -fno-builtin-printf -fno-builtin-puts -fno-strict-aliasing -fno-builtin-memcpy -mno-unaligned-access -ffunction-sections -fdata-sections -Wall -Wextra -Wno-unused-parameter -Wno-main
CFLAGS += $(EXTRA_CFLAGS)
ASFLAGS := -mthumb -mcpu=cortex-a9
LDFLAGS := -T linker.x -nodefaultlibs -nostdlib -lgcc -Wl,--build-id=none,--no-warn-rwx-segments,--gc-sections
LDFLAGS += -Wl,--defsym=PAYLOAD_ADDR=$(PAYLOAD_ADDR),--defsym=BSS_START=$(BSS_START)

BUILD_DIR := $(BUILD_ROOT)/$(DEVICE)

C_SRC = main.c debug.c patch.c $(DEVICE_SRC) \
        drivers/uart.c drivers/timer.c drivers/wdt.c drivers/rtc.c drivers/devinfo.c \
        lib/libc/memory.c lib/libc/string.c
ASM_SRC = start.S

OBJ = $(C_SRC:%.c=$(BUILD_DIR)/%.o) $(ASM_SRC:%.S=$(BUILD_DIR)/%.o)
DEP = $(OBJ:%.o=%.d)

.PHONY: $(TARGET)
$(TARGET): $(BUILD_DIR)/$(TARGET).bin
	@printf '  %-13s %s (%s bytes) @ %s\n' '$(DEVICE)' '$<' \
	    "$$(stat -c%s $<)" '$(PAYLOAD_ADDR)'

$(BUILD_DIR)/$(TARGET).bin: $(BUILD_DIR)/$(TARGET).elf
	$(OBJCOPY) -O binary $^ $@

$(BUILD_DIR)/$(TARGET).elf: $(OBJ)
	$(LD) -o $@ $^ $(LDFLAGS)

-include $(DEP)

$(BUILD_DIR)/%.o: %.c
	mkdir -p $(@D)
	$(CC) -MMD -c -o $@ $< $(CFLAGS)

$(BUILD_DIR)/%.o: %.S
	mkdir -p $(@D)
	$(AS) $(ASFLAGS) -o $@ $<

endif
