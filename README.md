# Lab2_CompArch
Instruction Level Simulator LC3-b

Simulator takes an assembly object file and implements it at an instruction level. We have a shell and simulation routines which compromise the project.

Shell gives commands to a user to see what is happening on a line by line basis. Shell takes 1+ ISA programs and loads them into a memory image. 
Commands provided by the shell to user: 
  go – simulate the program until a HALT instruction is executed.
  run <n> – simulate the execution of the machine for n instructions
  mdump <low> <high> – dump the contents of memory, from location low to location high to     the screen and the dump file
  rdump – dump the current instruction count, the contents of R0–R7, PC, and condition       codes to the screen and the dump file.
  ? – print out a list of all shell commands.
  quit – quit the shell
  
 
Simulatino routines are meant to carry out instruction level simulation of LC3-b program, takes current architectural state and modifies it according to ISA description. 

The process_instruction is our driver for the code. This is where simulation of instructions will begin. 
Read current system state from the global CURRENT_LATCHES arguement. Result of instruction should go into global variable NEXT_LATCHES. 
