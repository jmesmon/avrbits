# Hey Emacs, this is a -*- makefile -*-

# AVR-GCC Makefile template, derived from the WinAVR template (which
# is public domain), believed to be neutral to any flavor of "make"
# (GNU make, BSD make, SysV make)


MCU = atmega328p
ADMCU = atmega328p
F_CPU = 16000000

FORMAT = ihex
TARGET = arr
SRC  = main.c
SRC += msg_proc.c
SRC += version.c
SRC += ../common/usart.c
SRC += ../common/motor.c
SRC += drive.c
SRC += line.c
SRC += ../common/pid.c
SRC += ../common/adc.c


ASRC = 
OPT = s

srcdir = .

# Name of this Makefile (used for "make depend").
MAKEFILE = Makefile

# Debugging format.
# Native formats for AVR-GCC's -g are stabs [default], or dwarf-2.
# AVR (extended) COFF requires stabs, plus an avr-objcopy run.
DEBUG = stabs

# Compiler flag to set the C Standard level.
# c89   - "ANSI" C
# gnu89 - c89 plus GCC extensions
# c99   - ISO C99 standard (not yet fully implemented)
# gnu99 - c99 plus GCC extensions
CSTANDARD = -std=gnu99

VERSION := $(shell $(srcdir)/../setlocalversion)
CDEFS = -DVERSION="\"$(TARGET)$(VERSION)\""

# Place -I options here
CINCS = -I../common
CLINK = -Wl,--as-needed

CDEBUG = -g$(DEBUG)
CWARN = -Wall -Wstrict-prototypes -Wno-main
CTUNING = -funsigned-char -funsigned-bitfields -fpack-struct -fshort-enums
#CEXTRA = -Wa,-adhlns=$(<:.c=.lst)
CFLAGS += $(CDEBUG) $(CDEFS) $(CINCS) -O$(OPT) $(CWARN) $(CSTANDARD)
CFLAGS += $(CEXTRA) $(CLINK)


#ASFLAGS = -Wa,-adhlns=$(<:.S=.lst),-gstabs 


#Additional libraries.

# Minimalistic printf version
PRINTF_LIB_MIN = -Wl,-u,vfprintf -lprintf_min

# Floating point printf version (requires MATH_LIB = -lm below)
PRINTF_LIB_FLOAT = -Wl,-u,vfprintf -lprintf_flt

PRINTF_LIB = 

# Minimalistic scanf version
SCANF_LIB_MIN = -Wl,-u,vfscanf -lscanf_min

# Floating point + %[ scanf version (requires MATH_LIB = -lm below)
SCANF_LIB_FLOAT = -Wl,-u,vfscanf -lscanf_flt

SCANF_LIB = 

#MATH_LIB = -lm

# External memory options

# 64 KB of external RAM, starting after internal RAM (ATmega128!),
# used for variables (.data/.bss) and heap (malloc()).
#EXTMEMOPTS = -Wl,--section-start,.data=0x801100,--defsym=__heap_end=0x80ffff

# 64 KB of external RAM, starting after internal RAM (ATmega128!),
# only used for heap (malloc()).
#EXTMEMOPTS = -Wl,--defsym=__heap_start=0x801100,--defsym=__heap_end=0x80ffff

EXTMEMOPTS =

#LDMAP = $(LDFLAGS) -Wl,-Map=$(TARGET).map,--cref
LDFLAGS = $(EXTMEMOPTS) $(LDMAP) $(PRINTF_LIB) $(SCANF_LIB) $(MATH_LIB)


# Programming support using avrdude. Settings and variables.

AVRDUDE_PROGRAMMER = arduino
AVRDUDE_PORT = -P /dev/ttyUSB0 -b 57600

AVRDUDE_WRITE_FLASH = -U flash:w:$(TARGET).hex
#AVRDUDE_WRITE_EEPROM = -U eeprom:w:$(TARGET).eep


# Uncomment the following if you want avrdude's erase cycle counter.
# Note that this counter needs to be initialized first using -Yn,
# see avrdude manual.
#AVRDUDE_ERASE_COUNTER = -y

# Uncomment the following if you do /not/ wish a verification to be
# performed after programming the device.
#AVRDUDE_NO_VERIFY = -V

# Increase verbosity level.  Please use this when submitting bug
# reports about avrdude. See <http://savannah.nongnu.org/projects/avrdude> 
# to submit bug reports.
#AVRDUDE_VERBOSE = -v -v

AVRDUDE_BASIC = -p $(ADMCU) $(AVRDUDE_PORT) -c $(AVRDUDE_PROGRAMMER)
AVRDUDE_FLAGS = $(AVRDUDE_BASIC) $(AVRDUDE_NO_VERIFY) $(AVRDUDE_VERBOSE) $(AVRDUDE_ERASE_COUNTER)
AVRDUDE_POWER = -t -p $(ADMCU) -c $(AVRDUDE_PROGRAMMER)

CC = avr-gcc
OBJCOPY = avr-objcopy
OBJDUMP = avr-objdump
SIZE = avr-size
NM = avr-nm
AVRDUDE = avrdude
RM = rm -f
MV = mv -f

# Define all object files.
OBJ = $(SRC:=.o) $(ASRC:=.o)

# Define all listing files.
LST = $(SRC:=.lst) $(ASRC:=.lst)

# Combine all necessary flags and optional flags.
# Add target processor to flags.
ALL_CFLAGS = -mmcu=$(MCU) -I. $(CFLAGS) -DF_CPU=$(F_CPU)
ALL_ASFLAGS = -mmcu=$(MCU) -I. -x assembler-with-cpp $(ASFLAGS)


# Default target.
all: build

rebuild : | clean build
build: elf hex eep

elf: $(TARGET).elf $(TARGET).u.elf
hex: $(TARGET).hex $(TARGET).u.hex
eep: $(TARGET).eep $(TARGET).u.eep
lss: $(TARGET).elf.lss $(TARGET).u.elf.lss
sym: $(TARGET).sym $(TARGET).u.sym


# Program the device.  
program: $(TARGET).hex $(TARGET).eep
	$(AVRDUDE) $(AVRDUDE_FLAGS) $(AVRDUDE_WRITE_FLASH) $(AVRDUDE_WRITE_EEPROM)


power: 
	$(AVRDUDE) $(AVRDUDE_POWER)

# Convert ELF to COFF for use in debugging / simulating in AVR Studio or VMLAB.
COFFCONVERT=$(OBJCOPY) --debugging \
--change-section-address .data-0x800000 \
--change-section-address .bss-0x800000 \
--change-section-address .noinit-0x800000 \
--change-section-address .eeprom-0x810000 


coff: $(TARGET).cof
extcoff: $(TARGET).extcof

%.cof: %.elf
	$(COFFCONVERT) -O coff-avr $(TARGET).elf $(TARGET).cof


%.extcof: %.elf
	$(COFFCONVERT) -O coff-ext-avr $(TARGET).elf $(TARGET).extcof


.SUFFIXES: .elf .hex .eep
.elf.hex:
	@echo OBJCOPY $@
	@$(OBJCOPY) -O $(FORMAT) -R .eeprom $< $@

.elf.eep:
	@echo OBJCOPY $@
	@-$(OBJCOPY) -j .eeprom --set-section-flags=.eeprom="alloc,load" \
	--change-section-lma .eeprom=0 -O $(FORMAT) $< $@

# Create extended listing file from ELF output file.
%.elf.lss: %.elf
	@echo OBJDUMP $@
	@$(OBJDUMP) -h -S $< > $@

# Create a symbol table from ELF output file.
%.elf.sym: %.elf
	$(NM) -n $< > $@

# Link: create ELF output file from object files.
%.u.elf: $(SRC)
	@echo "CCLD $(@F) $^"
	@$(CC) $(ALL_CFLAGS) $(SRC) --combine -fwhole-program -o $@ $(LDFLAGS)

%.elf: $(OBJ)
	@echo "LD $(@F)"
	@$(CC) $(ALL_CFLAGS) $(OBJ) -o $@ $(LDFLAGS)

# Compile: create object files from C source files.
.SECONDARY:
%.c.o: %.c
	@echo "CC $(@F)"
	@$(CC) $(ALL_CFLAGS) -c -o $@ $<

# Compile: create assembler files from C source files.
%.c.s: %.s
	$(CC) -S $(ALL_CFLAGS) -c -o $@ $<

# Assemble: create object files from assembler source files.
%.S.o: %.S
	$(CC) -c $(ALL_ASFLAGS) $< -o $@


# Target: clean project.
%.clean : 
	$(RM) $*.hex $*.eep $*.cof $*.elf $*.map $*.sym $*.elf.lss

clean: | $(TARGET).clean $(TARGET).u.clean
	$(RM) $(OBJ) \
	$(LST) \
	$(SRC:.c=.s) \
	$(SRC:.c=.d)

depend:
	if grep '^# DO NOT DELETE' $(MAKEFILE) >/dev/null; \
	then \
		sed -e '/^# DO NOT DELETE/,$$d' $(MAKEFILE) > \
			$(MAKEFILE).$$$$ && \
		$(MV) $(MAKEFILE).$$$$ $(MAKEFILE); \
	fi
	echo '# DO NOT DELETE THIS LINE -- make depend depends on it.' \
		>> $(MAKEFILE); \
	$(CC) -M -mmcu=$(MCU) $(CDEFS) $(CINCS) $(SRC) $(ASRC) >> $(MAKEFILE)

.PHONY:	all build elf hex eep lss sym program coff extcoff clean depend \
	rebuild %.clean
