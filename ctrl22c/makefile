# Makefile for agccs-ctrl22 AVR firmware

# location of avr toolchain
# -- this is the Mac OSX variant, where we borrow the toolchain from an Arduiono installation;
#    if you are with Linux you can use the system toolchain and do not need this line
PATH	       = /Applications/Arduino.app/Contents/Java/hardware/tools/avr/bin/

# location of avrdude config
# -- again, we borrow this from Arduiono for Mac OSX; avrdude.conf is critical for the comperative
#    new ATmega4808; on a recent Linux distribution you may get away with out-oft-he-box avrdude
DUDECONF	= -C /Applications/Arduino.app/Contents/Java/hardware/tools/avr/etc/avrdude.conf

# ###################################################
#
# below this line: std. makefile, nothing to edit
#

# program to build
PRG            = ctrl22c
SRC	       = $(PRG).c
OBJ            = $(PRG).o
DEFS           =

# configure compiler/linker/loader
# -- most relevant: LDFLAGS "-Wl,--section-start=.text=0x200" to load just after Optiboot; remove
#    this flag if you are running bare-bone without boorloader;
MCU_TARGET     = atmega4808
OPTIMIZE       = -Os
CFLAGS         = -g -Wall $(OPTIMIZE) -mmcu=$(MCU_TARGET) -std=gnu11 -ffunction-sections -fdata-sections -funsigned-char -fshort-enums $(DEFS)
LDFLAGS        = -g -Wl,-Map,$(PRG).map $(OPTIMIZE) -mmcu=$(MCU_TARGET) -Wl,--start-group -Wl,--end-group -Wl,--gc-sections -Wl,--section-start=.text=0x200 

# toolchain
CC             = $(PATH)avr-gcc
OBJCOPY        = $(PATH)avr-objcopy
OBJDUMP        = $(PATH)avr-objdump

# default target
all: $(PRG).hex $(PRG).bin

$(PRG).o: $(PRG).c
	$(CC) -c $(CFLAGS) -o $@ $^ 

$(PRG).elf: $(PRG).o
	$(CC) $(LDFLAGS) -o $@ $^ $(LIBS)

$(PRG).hex: $(PRG).elf
	$(OBJCOPY) -O ihex $< $@

$(PRG).bin: $(PRG).elf
	$(OBJCOPY) -O binary $< $@

$(PRG).lst: $(PRG).elf
	$(OBJDUMP) -h -S $< > $@


# clean up
clean:
	rm -rf $(PRG).o $(PRG).elf $(PRG).hex $(PRG).lst $(PRG).bin  $(PRG).map

# flash via ESP32 debug server
flash: $(PRG).hex
	$(PATH)avrdude -patmega4808  -carduino -Pnet:192.168.4.1:2323 -U flash:w:$(PRG).hex -D $(DUDECONF)



