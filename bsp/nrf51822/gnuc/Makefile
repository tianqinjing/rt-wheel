TARGET_NAME      = rt-wheel

GNU_INSTALL_ROOT = /gcc-arm-none-eabi-7-2018-q2-update-win32
GNU_PREFIX       = arm-none-eabi

MK := mkdir
RM := rm -rf

ifeq ("$(VERBOSE)","1")
NO_ECHO := 
else
NO_ECHO := @
endif

# Toolchain commands
CC      := '$(GNU_INSTALL_ROOT)/bin/$(GNU_PREFIX)-gcc'
AS      := '$(GNU_INSTALL_ROOT)/bin/$(GNU_PREFIX)-as'
AR      := '$(GNU_INSTALL_ROOT)/bin/$(GNU_PREFIX)-ar' -r
LD      := '$(GNU_INSTALL_ROOT)/bin/$(GNU_PREFIX)-ld'
NM      := '$(GNU_INSTALL_ROOT)/bin/$(GNU_PREFIX)-nm'
OBJDUMP := '$(GNU_INSTALL_ROOT)/bin/$(GNU_PREFIX)-objdump'
OBJCOPY := '$(GNU_INSTALL_ROOT)/bin/$(GNU_PREFIX)-objcopy'
SIZE    := '$(GNU_INSTALL_ROOT)/bin/$(GNU_PREFIX)-size'

#function for removing duplicates in a list
remduplicates = $(strip $(if $1,$(firstword $1) $(call remduplicates,$(filter-out $(firstword $1),$1))))

#source common to all targets
C_SOURCE_FILES =                                        \
              ../hw_config.c                            \
              ../../../arch/aarch-m/arch_init.c         \
              ../../../arch/aarch-m/task_arch.c         \
              ../../../core/hal/hal_timer.c             \
              ../../../core/hal/hal_uart.c              \
              ../../../core/kernel/critical.c           \
              ../../../core/kernel/event.c              \
              ../../../core/kernel/msg_queue.c          \
              ../../../core/kernel/mutex.c              \
              ../../../core/kernel/sem.c                \
              ../../../core/kernel/task.c               \
              ../../../core/kernel/tick.c               \
              ../../../core/kernel/timer.c              \
              ../../../core/mem/heap.c                  \
              ../../../core/mem/mem.c                   \
              ../../../core/mem/mmu.c                   \
              ../../../core/services/defer.c            \
              ../../../core/services/sysclk.c           \
              ../../../drivers/driver_init.c            \
              ../../../drivers/intc/nvic.c              \
              ../../../utils/rbtree.c                   \
              ../../../main.c                           \
              ../rtc.c                                  \
              ../../../arch/aarch-m/exc.c               \
              ../../../core/hal/hal_int.c               \
              ../../../core/hal/hal_exc.c               \
              ../uart.c                                 \
              ../../../utils/ring.c                     \
              ../../../cmder/cmder.c                    \
              ../../../cmder/cmder_uart.c


#assembly files common to all targets
S_SOURCE_FILES =                                        \
              ./startup.s                               \
              ../../../arch/aarch-m/gnuc/handler.s      \
              ../../../arch/aarch-m/gnuc/context.s

LD_SCRIPT      = nrf51822.ld

#includes common to all targets
INC_PATHS =                                             \
              -I../../../include                        \
              -I..

OUT_DIR = out
LST_DIR = $(OUT_DIR)

# Sorting removes duplicates
DIRS   := $(sort $(OUT_DIR) $(LST_DIR))

DEFINS  = -D__AARCH_M__ -DNR_IRQS=32

#flags common to all targets
CFLAGS += -mcpu=cortex-m0
CFLAGS += -mthumb -mabi=aapcs --std=gnu99
CFLAGS += -Wall -Werror
CFLAGS += -mfloat-abi=soft
# keep every function in separate section. This will allow linker to dump unused functions
CFLAGS += -ffunction-sections -fdata-sections -O3 -fno-strict-aliasing
CFLAGS += -fno-builtin --short-enums
CFLAGS += $(DEFINS)

SFLAGS += -x assembler-with-cpp -mcpu=cortex-m0 -mthumb -mabi=aapcs
SFLAGS += $(DEFINS)

# keep every function in separate section. This will allow linker to dump unused functions
LFLAGS += -Xlinker -Map=$(LST_DIR)/$(TARGET_NAME).map
LFLAGS += -mthumb -mabi=aapcs -T$(LD_SCRIPT)
LFLAGS += -mcpu=cortex-m0
# let linker to dump unused sections
LFLAGS += -Wl,--gc-sections
# use newlib in nano version
LFLAGS += --specs=nano.specs -lc -lnosys

default: $(OUT_DIR)/$(TARGET_NAME).hex

C_OBJS = $(patsubst %.c, $(OUT_DIR)/%.o, $(notdir $(C_SOURCE_FILES)))
S_OBJS = $(patsubst %.s, $(OUT_DIR)/%.o, $(notdir $(S_SOURCE_FILES)))

vpath %.c $(call remduplicates, $(dir $(C_SOURCE_FILES)))
vpath %.s $(call remduplicates, $(dir $(S_SOURCE_FILES)))

OBJS = $(C_OBJS) $(S_OBJS)

## Create build directories
$(DIRS):
	$(MK) $@

$(OUT_DIR)/%.o: %.c
	@echo Compiling file: $(notdir $<)
	$(NO_ECHO)$(CC) $(CFLAGS) $(INC_PATHS) -c -o $@ $<

$(OUT_DIR)/%.o: %.s
	@echo Compiling file: $(notdir $<)
	$(NO_ECHO)$(CC) $(SFLAGS) $(INC_PATHS) -c -o $@ $<

$(OUT_DIR)/$(TARGET_NAME).out: $(DIRS) $(OBJS)
	@echo Linking target: $(TARGET_NAME).out
	$(NO_ECHO)$(CC) $(LFLAGS) $(OBJS) $(LIBS) -o $(OUT_DIR)/$(TARGET_NAME).out
	$(NO_ECHO)$(SIZE) $(OUT_DIR)/$(TARGET_NAME).out

$(OUT_DIR)/$(TARGET_NAME).hex: $(OUT_DIR)/$(TARGET_NAME).out
	@echo Preparing: $(TARGET_NAME).hex
	$(NO_ECHO)$(OBJCOPY) -O ihex $(OUT_DIR)/$(TARGET_NAME).out $(OUT_DIR)/$(TARGET_NAME).hex

clean:
	$(RM) $(DIRS)

