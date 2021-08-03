# makfile configuration
CPU             	= msp430g2553
CFLAGS          	= -mmcu=${CPU} -Os -I../h
LDFLAGS		= -L../lib -L/opt/ti/msp430_gcc/include/ 

#switch the compiler (for the internal make rules)
CC              = msp430-elf-gcc
AS              = msp430-elf-gcc -mmcu=${CPU} -c

all:wakedemo.elf

#additional rules for files
wakedemo.elf: ${COMMON_OBJECTS} wakedemo.o wdt_handler.o buzzer.o #stateMachines.o
	${CC} ${CFLAGS} ${LDFLAGS} -o $@ $^ -lTimer -lLcd 

buzzer.o: buzzer.c buzzer.h

#stateMachines.o: stateMachines.s

load: wakedemo.elf
	msp430loader.sh $^

clean:
	rm -f *.o *.elf
