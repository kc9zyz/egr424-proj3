# Modify as appropriate
STELLARISWARE=C:/StellarisWare

CC=arm-none-eabi-gcc -Wall -Os -march=armv7-m -mcpu=cortex-m3 -mthumb -mfix-cortex-m3-ldrd -Wl,--gc-sections
proj3.bin: proj3.elf
	arm-none-eabi-objcopy -O binary proj3.elf proj3.bin
proj3.elf: proj3.c startup_gcc.c syscalls.c rit128x96x4.c create.S lock.S locking.c threads.c reg_save.S reg_restore.S
	${CC} -o $@ -I${STELLARISWARE} -L${STELLARISWARE}/driverlib/gcc-cm3 -Tlinkscript.x -Wl,-Map,proj3.map -Wl,--entry,ResetISR proj3.c startup_gcc.c syscalls.c rit128x96x4.c create.S reg_save.S reg_restore.S lock.S locking.c threads.c -ldriver-cm3

.PHONY: clean
clean:
	rm -f *.elf *.map

# vim: noexpandtab
