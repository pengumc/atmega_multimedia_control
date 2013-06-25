# Name: Makefile
# Author: Michiel van der Coelen
# date: 2013-06-04
 
MMCU = atmega88
AVRDUDE_MCU = m88
AVRDUDE_PROGRAMMERID = usbasp
#AVRDUDE_PORT = 
F_CPU = 20000000
NAME = main

#for the love of god, keep the linking order!
OBJECTS = usbdrv/usbdrv.o usbdrv/usbdrvasm.o usbdrv/oddebug.o $(NAME).o
CFLAGS = -DF_CPU=$(F_CPU) -std=c99 -Wall -Os -mmcu=$(MMCU) -I. -I./usbdrv
CC = avr-gcc
SIZE = avr-size
OBJCOPY = avr-objcopy

.PHONY: all clean test
all: $(NAME).hex
	$(SIZE) $(NAME).hex

#rebuild everything!
force: clean all

$(NAME).hex: $(NAME).elf
	$(OBJCOPY) -O ihex $(NAME).elf $(NAME).hex
	
$(NAME).elf: $(OBJECTS)
	$(CC) $(CFLAGS) -o $(NAME).elf $(OBJECTS)

#compile src files
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

%.o: %.S
	$(CC) $(CFLAGS) -x assembler-with-cpp -c $< -o $@


clean:
	rm -f $(OBJECTS) $(NAME).elf

program: $(NAME).hex
	avrdude -c $(AVRDUDE_PROGRAMMERID) -p $(AVRDUDE_MCU) -U flash:w:$(NAME).hex

test:
	avrdude -c $(AVRDUDE_PROGRAMMERID) -p $(AVRDUDE_MCU) -v