output: main.o RISCV_Instructions.o RISCV_Memory.o
	g++ main.o RISCV_Instructions.o RISCV_Memory.o -o riscv_sim

main.o: main.cpp
	g++ -c main.cpp

RISCV_Instructions.o: RISCV_Instructions.cpp
	g++ -c RISCV_Instructions.cpp

RISCV_Memory.o: RISCV_Memory.cpp
	g++ -c RISCV_Memory.cpp

clean:
	rm *.o riscv_sim 
	