#USBFLAGS = `libusb-config --cflags`
USBFLAGS = -I.
#USBLIBS = `libusb-config --libs`
USBLIBS = -L. -lusb

EXE_SUFFIX = .exe

NAME = cmdline

OBJECTS = opendevice.o $(NAME).o

CC		= gcc
CFLAGS	= $(CPPFLAGS) $(USBFLAGS) -O -g -Wall -std=c99

PROGRAM = $(NAME)$(EXE_SUFFIX)


all: $(PROGRAM)

.c.o:
	$(CC) $(CFLAGS) -c $<

$(PROGRAM): $(OBJECTS)
	$(CC) -o $(PROGRAM) $(OBJECTS) $(USBLIBS)

clean:
	rm -f *.o $(PROGRAM)
