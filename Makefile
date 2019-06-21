processor: single-cycle.o cpu.o memory.o syscall.o 
	gcc single-cycle.o cpu.o memory.o syscall.o -o processor -lm

single-cycle.o: single-cycle.c
	gcc -c single-cycle.c

cpu.o: cpu.c
	gcc -c cpu.c

memory.o: memory.c
	gcc -c memory.c

syscall.o: syscall.c
	gcc -c syscall.c

clean:
	rm *.o processor