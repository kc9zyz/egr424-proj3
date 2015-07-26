# Modify as appropriate
STELLARISWARE=C:/StellarisWare

CC=arm-none-eabi-gcc -Wall -Os -march=armv7-m -mcpu=cortex-m3 -mthumb -mfix-cortex-m3-ldrd -Wl,--gc-sections
lockdemo.bin: lockdemo.elf
	arm-none-eabi-objcopy -O binary lockdemo.elf lockdemo.bin
lockdemo.elf: lockdemo.c startup_gcc.c syscalls.c rit128x96x4.c create.S lock.S locking.c threads.c
	${CC} -o $@ -I${STELLARISWARE} -L${STELLARISWARE}/driverlib/gcc-cm3 -Tlinkscript.x -Wl,-Map,lockdemo.map -Wl,--entry,ResetISR lockdemo.c startup_gcc.c syscalls.c rit128x96x4.c create.S lock.S locking.c threads.c -ldriver-cm3

.PHONY: clean
clean:
	rm -f *.elf *.map

# vim: noexpandtab
