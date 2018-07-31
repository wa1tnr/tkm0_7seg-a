
ifdef SystemRoot
	SHELL = cmd.exe
	MK_DIR = mkdir
else
	ifeq ($(shell uname), Linux)
		MK_DIR = mkdir -p
	endif

	ifeq ($(shell uname | cut -d _ -f 1), CYGWIN)
		MK_DIR = mkdir -p
	endif

	ifeq ($(shell uname | cut -d _ -f 1), MINGW32)
		MK_DIR = mkdir -p
	endif

	ifeq ($(shell uname | cut -d _ -f 1), MINGW64)
		MK_DIR = mkdir -p
	endif
endif

# Programming tool
OPENOCD = openocd

# Target Chip
MCU_DEFINE := __SAMD51J18A__

# Flags
LFLAGS += -mthumb -mabi=aapcs-linux -mlong-calls -mcpu=cortex-m4 -mfloat-abi=softfp -mfpu=fpv4-sp-d16 -DSAMD51
CFLAGS += -x c -DDEBUG -Os -ffunction-sections -g3 -Wall -c -std=gnu99 $(LFLAGS)
INCLUDES += -I"../mkstepper-v011/mkstepper-v011" -I"../samd51" -I"../samd51/CMSIS/Include" -I"../samd51/include" -I"../samd51/startup"

# List the subdirectories for creating object files
SUB_DIRS +=  \
 \
mkstepper-v011/mkstepper-v011 \
samd51/CMSIS/Include \
samd51/include \
samd51/startup \

# List the source files
SRCS = $(wildcard ../mkstepper-v011/mkstepper-v011/*.c)
SRCS += $(wildcard ../samd51/startup/*.c)

# List the object files
OBJS += $(SRCS:%.c=%.o)
OBJS := $(OBJS:../%=%) # detach path
OBJS_AS_ARGS += $(OBJS)

# List the dependency files
DEPS := $(OBJS:%.o=%.d)
DEPS_AS_ARGS += $(DEPS)

OUTPUT_FILE_NAME :=mkstepper
QUOTE := "
OUTPUT_FILE_PATH +=$(OUTPUT_FILE_NAME).elf
OUTPUT_FILE_PATH_AS_ARGS +=$(OUTPUT_FILE_NAME).elf

vpath %.c ..
vpath %.s ..
vpath %.S ..

# All Target
all: $(SUB_DIRS) $(OUTPUT_FILE_PATH)

# Linker target
$(OUTPUT_FILE_PATH): $(OBJS)
	@echo Building target: $@
	@echo Invoking: ARM/GNU Linker
	$(QUOTE)arm-none-eabi-gcc$(QUOTE) -o $(OUTPUT_FILE_NAME).elf $(OBJS_AS_ARGS) -Wl,--start-group -lm -Wl,--end-group $(LFLAGS) \
-Wl,-Map="$(OUTPUT_FILE_NAME).map" --specs=nano.specs -Wl,--gc-sections \
 \
 \
-T"../samd51/startup/samd51j18a_flash.ld" \
-L"../samd51/startup"
	@echo Finished building target: $@

	"arm-none-eabi-objcopy" -O binary "$(OUTPUT_FILE_NAME).elf" "$(OUTPUT_FILE_NAME).bin"
	"arm-none-eabi-objcopy" -O ihex -R .eeprom -R .fuse -R .lock -R .signature  \
        "$(OUTPUT_FILE_NAME).elf" "$(OUTPUT_FILE_NAME).hex"
	"arm-none-eabi-objcopy" -j .eeprom --set-section-flags=.eeprom=alloc,load --change-section-lma \
        .eeprom=0 --no-change-warnings -O binary "$(OUTPUT_FILE_NAME).elf" \
        "$(OUTPUT_FILE_NAME).eep" || exit 0
	"arm-none-eabi-objdump" -h -S "$(OUTPUT_FILE_NAME).elf" > "$(OUTPUT_FILE_NAME).lss"
	"arm-none-eabi-size" "$(OUTPUT_FILE_NAME).elf"

# Compiler targets

%.o: %.c
	@echo Building file: $<
	@echo ARM/GNU C Compiler
	$(QUOTE)arm-none-eabi-gcc$(QUOTE) $(CFLAGS) -D$(MCU_DEFINE) $(INCLUDES) \
-MD -MP -MF "$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -MT"$(@:%.o=%.o)"  -o "$@" "$<"
	@echo Finished building: $<

%.o: %.s
	@echo Building file: $<
	@echo ARM/GNU Assembler
	$(QUOTE)arm-none-eabi-as$(QUOTE) $(CFLAGS) -D$(MCU_DEFINE) $(INCLUDES) \
-MD -MP -MF "$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -MT"$(@:%.o=%.o)"  -o "$@" "$<"
	@echo Finished building: $<

%.o: %.S
	@echo Building file: $<
	@echo ARM/GNU Preprocessing Assembler
	$(QUOTE)arm-none-eabi-gcc$(QUOTE) $(CFLAGS) -D$(MCU_DEFINE) $(INCLUDES) \
-MD -MP -MF "$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -MT"$(@:%.o=%.o)"  -o "$@" "$<"
	@echo Finished building: $<

# Detect changes in the dependent files and recompile the respective object files.
ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(DEPS)),)
-include $(DEPS)
endif
endif

program: $(OUTPUT_FILE_NAME).elf
	$(OPENOCD) -f ../openocd/program.cfg

openocd: $(OUTPUT_FILE_NAME).elf
	$(OPENOCD) -f ../openocd/debug.cfg

debug: $(OUTPUT_FILE_NAME).elf
	arm-none-eabi-gdb -tui -ex "target extended-remote localhost:3333" -ex 'monitor reset halt' -ex 'load' -ex 'monitor reset init' ./mkstepper.elf

$(SUB_DIRS):
	$(MK_DIR) "$@"

clean:
	rm -f $(OBJS_AS_ARGS)
	rm -f $(OUTPUT_FILE_PATH)
	rm -f $(DEPS_AS_ARGS)
	rm -f $(OUTPUT_FILE_NAME).a $(OUTPUT_FILE_NAME).hex $(OUTPUT_FILE_NAME).bin \
        $(OUTPUT_FILE_NAME).lss $(OUTPUT_FILE_NAME).eep $(OUTPUT_FILE_NAME).map \
        $(OUTPUT_FILE_NAME).srec
