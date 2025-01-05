# RISC-V Simulator

This RISC-V Simulator supports the **RISC-V ISA 64I variant** (only base instructions) and provides an environment to load, execute, and debug RISC-V assembly programs. The simulator initializes all registers and memory to default values and provides functionalities to run code, view register states, and manage memory. It also supports Cache Simulations. The RISCV instruction manual can be found [here](./RISCV_instruction_manual.pdf)

The program operates as a command-line interface (CLI) and continuously runs in an interactive loop, waiting for user commands to be entered in the terminal.

## Specifications
- All registers, instructions are 64 bit and address space is 32 bit
- The text section spans from `0x00000 -> 0x10000`
- The data section spans from `0x10000 -> 0x50000`

---

## Compilation and Cleaning:
- **To compile the simulator:**
  `make`

- **To clean the build files:**
  `make clean`

- **To execute**
  `.\riscv_sim`

---

# Commands of Simulator (Basic)
- **load `<filename>`**: Loads the file containing RISC-V assembly code. (all registers and memory gets initialized to the default value.)
- **run**: Execute a given RISC-V code and update registers, memory, etc. It runs the given
RISC-V code till the end.
- **step**: Runs one instruction and prints  “Executed `<instruction>`; PC=`<address>`"
- **exit**: Exit the simulator 
- **regs**: Print the values of all registers in hex format. (64-bit registers).
- **mem `<addr>` `<count>`**: Print count memory locations starting from address addr in the
data section. (Little Endian format).
- **data `<size>`**: View data memory contents: starting at `0x10000` in 64 bit format
-  **stack `<size>`**: View stack memory contents: ending at `0x50000` in 64 bit format

- **show-stack**: Prints the stack information, and the line elements are pushed onto and
popped from the stack frame. The stack frame is only updated on function invocations.
- **break `<line number>`**: Sets a mark to stop the code execution once the line is reached,
preserving registers and memory state.
- **del break `<line number>`**: Deletes the breakpoint at the specified line. If no breakpoint is
present, a meaningful error message can be printed.

---
# Cache Configuration 

## Specifications
- Cache size is a power of 2. MAX 1 MB
- Assosciativity is a power of 2. MAX 16 (0 - Fully Associative, 1 - Direct Mapped, else Set Associative)
- FIFO, LRU, RANDOM replacement policy supported
- Write Trough and Write Back policy supported
- Whenever a `<file>.s` program is run a `<file>.output` is created that logs cache state in the format

<div align="center">

**R:** **Address:** 0x20202, **Set:** 0x02, **Miss**, **Tag:** 0x202, **Clean**  
**W:** **Address:** 0x10306, **Set:** 0x06, **Hit**, **Tag:** 0x103, **Dirty**  
**R:** **Address:** 0x20511, **Set:** 0x11, **Miss**, **Tag:** 0x205, **Clean**  

</div>

--- 

 ## Commands
- **cache_sim enable `<config_file>`** - Enables cache simulation for D-cache within the
simulator. Uses parameters from `config_file` for cache settings .`config_file` should have the following format
<div align="left">

`<SIZE_OF_CACHE>` (in bytes
)  
`<BLOCK_SIZE>` (in bytes)  
`<ASSOCIATIVITY>` (number)  
`<REPLACEMENT_POLICY>` (FIFO or LRU or RANDOM)  
`<WRITEBACK_POLICY>` (Write Back or Write Through)

</div>

- **cache_sim disable** - Disables cache simulation. This is the default state at the start of the
simulator.
**enable and disable commands can not be executed while a file is being executed or loaded**.

- **cache_sim status** - prints the enable/disable status of the D-cache simulation. If cache
simulation is enabled, it also prints the cache configuration values in the format like:

<div align="center">

**Cache Size:** 32768  
**Block Size:** 16  
**Associativity:** 8  
**Replacement Policy:** LRU  
**Write Back Policy:** WT  

</div>

- **cache_sim invalidate** - invalidates all entries of the D-cache.
- **cache_sim dump `<file>`** - writes all the current D-cache entries to the file
`<file>` in the following format. Notice that only the valid entries have been printed
in order of their Set value.

<div align="center">

**Set:** 0x00, **Tag:** 0x100, **Status:** Clean  
**Set:** 0x01, **Tag:** 0x123, **Status:** Clean  
**Set:** 0x01, **Tag:** 0x736, **Status:** Dirty  
**Set:** 0x02, **Tag:** 0x145, **Status:** Dirty  
**Set:** 0x10, **Tag:** 0x321, **Status:** Clean  

</div>

- **log**: Prints the contents of the cache

- **cache_sim stats** - prints the cache statistics for the executing code at the current
instance in the format:
D-cache statistics: Accesses=100, Hit=93, Miss=7, Hit Rate=0.93

---







