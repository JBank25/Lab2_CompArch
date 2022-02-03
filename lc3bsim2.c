/*
    Name 1: Joshua Urbank
    Name 2: Full name of the second partner
    UTEID 1: jbu234
    UTEID 2: UT EID of the second partner
*/

/***************************************************************/
/*                                                             */
/*   LC-3b Instruction Level Simulator                         */
/*                                                             */
/*   EE 460N                                                   */
/*   The University of Texas at Austin                         */
/*                                                             */
/***************************************************************/

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/***************************************************************/
/*                                                             */
/* Files: isaprogram   LC-3b machine language program file     */
/*                                                             */
/***************************************************************/

/***************************************************************/
/* These are the functions you'll have to write.               */
/***************************************************************/

void process_instruction();

enum instructions{ADD, AND, HALT, JMP, JSR, JSRR, LDB, LDW, LEA,            //enum type used for switch statement when decoding
                NOP, NOT, RET, LSHF, RSHFL, RSHFA, RTI, STB, STW,           //opcodes 
                TRAP, XOR, BRN, BRZ, BRP, BR, BRZP, BRNP, BRNZ, BRNZP} code;


void addInstruction();
void andInstruction();
void xorInstruction();
void haltInstruction();

/***************************************************************/
/* A couple of useful definitions.                             */
/***************************************************************/
#define FALSE 0
#define TRUE  1

/***************************************************************/
/* Use this to avoid overflowing 16 bits on the bus.           */
/***************************************************************/
#define Low16bits(x) ((x) & 0xFFFF)
#define test(x,y) x + y
#define SetN(A) (A & 0x8000) > 1
#define SetP(A) (A & 0x8000) != 1
#define SetZ(A) (A == 0)

/***************************************************************/
/* Main memory.                                                */
/***************************************************************/
/* MEMORY[A][0] stores the least significant byte of word at word address A
   MEMORY[A][1] stores the most significant byte of word at word address A 
*/

#define WORDS_IN_MEM    0x08000 
int MEMORY[WORDS_IN_MEM][2];

/***************************************************************/

/***************************************************************/

/***************************************************************/
/* LC-3b State info.                                           */
/***************************************************************/
#define LC_3b_REGS 8

int RUN_BIT;	/* run bit */


typedef struct System_Latches_Struct{

  int PC,		/* program counter */
    N,		/* n condition bit */
    Z,		/* z condition bit */
    P;		/* p condition bit */
  int REGS[LC_3b_REGS]; /* register file. */
} System_Latches;

/* Data Structure for Latch */

System_Latches CURRENT_LATCHES, NEXT_LATCHES;

/***************************************************************/
/* A cycle counter.                                            */
/***************************************************************/
int INSTRUCTION_COUNT;

/***************************************************************/
/*                                                             */
/* Procedure : help                                            */
/*                                                             */
/* Purpose   : Print out a list of commands                    */
/*                                                             */
/***************************************************************/
void help() {                                                    
  printf("----------------LC-3b ISIM Help-----------------------\n");
  printf("go               -  run program to completion         \n");
  printf("run n            -  execute program for n instructions\n");
  printf("mdump low high   -  dump memory from low to high      \n");
  printf("rdump            -  dump the register & bus values    \n");
  printf("?                -  display this help menu            \n");
  printf("quit             -  exit the program                  \n\n");
}

/***************************************************************/
/*                                                             */
/* Procedure : cycle                                           */
/*                                                             */
/* Purpose   : Execute a cycle                                 */
/*                                                             */
/***************************************************************/
void cycle() {                                                

  process_instruction();
  CURRENT_LATCHES = NEXT_LATCHES;
  INSTRUCTION_COUNT++;
}

/***************************************************************/
/*                                                             */
/* Procedure : run n                                           */
/*                                                             */
/* Purpose   : Simulate the LC-3b for n cycles                 */
/*                                                             */
/***************************************************************/
void run(int num_cycles) {                                      
  int i;

  if (RUN_BIT == FALSE) {
    printf("Can't simulate, Simulator is halted\n\n");
    return;
  }

  printf("Simulating for %d cycles...\n\n", num_cycles);
  for (i = 0; i < num_cycles; i++) {
    if (CURRENT_LATCHES.PC == 0x0000) {
	    RUN_BIT = FALSE;
	    printf("Simulator halted\n\n");
	    break;
    }
    cycle();
  }
}

/***************************************************************/
/*                                                             */
/* Procedure : go                                              */
/*                                                             */
/* Purpose   : Simulate the LC-3b until HALTed                 */
/*                                                             */
/***************************************************************/
void go() {                                                     
  if (RUN_BIT == FALSE) {
    printf("Can't simulate, Simulator is halted\n\n");
    return;
  }

  printf("Simulating...\n\n");
  while (CURRENT_LATCHES.PC != 0x0000)
    cycle();
  RUN_BIT = FALSE;
  printf("Simulator halted\n\n");
}

/***************************************************************/ 
/*                                                             */
/* Procedure : mdump                                           */
/*                                                             */
/* Purpose   : Dump a word-aligned region of memory to the     */
/*             output file.                                    */
/*                                                             */
/***************************************************************/
void mdump(FILE * dumpsim_file, int start, int stop) {          
  int address; /* this is a byte address */

  printf("\nMemory content [0x%.4x..0x%.4x] :\n", start, stop);
  printf("-------------------------------------\n");
  for (address = (start >> 1); address <= (stop >> 1); address++)
    printf("  0x%.4x (%d) : 0x%.2x%.2x\n", address << 1, address << 1, MEMORY[address][1], MEMORY[address][0]);
  printf("\n");

  /* dump the memory contents into the dumpsim file */
  fprintf(dumpsim_file, "\nMemory content [0x%.4x..0x%.4x] :\n", start, stop);
  fprintf(dumpsim_file, "-------------------------------------\n");
  for (address = (start >> 1); address <= (stop >> 1); address++)
    fprintf(dumpsim_file, " 0x%.4x (%d) : 0x%.2x%.2x\n", address << 1, address << 1, MEMORY[address][1], MEMORY[address][0]);
  fprintf(dumpsim_file, "\n");
  fflush(dumpsim_file);
}

/***************************************************************/
/*                                                             */
/* Procedure : rdump                                           */
/*                                                             */
/* Purpose   : Dump current register and bus values to the     */   
/*             output file.                                    */
/*                                                             */
/***************************************************************/
void rdump(FILE * dumpsim_file) {                               
  int k; 

  printf("\nCurrent register/bus values :\n");
  printf("-------------------------------------\n");
  printf("Instruction Count : %d\n", INSTRUCTION_COUNT);
  printf("PC                : 0x%.4x\n", CURRENT_LATCHES.PC);
  printf("CCs: N = %d  Z = %d  P = %d\n", CURRENT_LATCHES.N, CURRENT_LATCHES.Z, CURRENT_LATCHES.P);
  printf("Registers:\n");
  for (k = 0; k < LC_3b_REGS; k++)
    printf("%d: 0x%.4x\n", k, CURRENT_LATCHES.REGS[k]);
  printf("\n");

  /* dump the state information into the dumpsim file */
  fprintf(dumpsim_file, "\nCurrent register/bus values :\n");
  fprintf(dumpsim_file, "-------------------------------------\n");
  fprintf(dumpsim_file, "Instruction Count : %d\n", INSTRUCTION_COUNT);
  fprintf(dumpsim_file, "PC                : 0x%.4x\n", CURRENT_LATCHES.PC);
  fprintf(dumpsim_file, "CCs: N = %d  Z = %d  P = %d\n", CURRENT_LATCHES.N, CURRENT_LATCHES.Z, CURRENT_LATCHES.P);
  fprintf(dumpsim_file, "Registers:\n");
  for (k = 0; k < LC_3b_REGS; k++)
    fprintf(dumpsim_file, "%d: 0x%.4x\n", k, CURRENT_LATCHES.REGS[k]);
  fprintf(dumpsim_file, "\n");
  fflush(dumpsim_file);
}

/***************************************************************/
/*                                                             */
/* Procedure : get_command                                     */
/*                                                             */
/* Purpose   : Read a command from standard input.             */  
/*                                                             */
/***************************************************************/
void get_command(FILE * dumpsim_file) {                         
  char buffer[20];
  int start, stop, cycles;

  printf("LC-3b-SIM> ");

  scanf("%s", buffer);
  printf("\n");

  switch(buffer[0]) {
  case 'G':
  case 'g':
    go();
    break;

  case 'M':
  case 'm':
    scanf("%i %i", &start, &stop);
    mdump(dumpsim_file, start, stop);
    break;

  case '?':
    help();
    break;
  case 'Q':
  case 'q':
    printf("Bye.\n");
    exit(0);

  case 'R':
  case 'r':
    if (buffer[1] == 'd' || buffer[1] == 'D')
	    rdump(dumpsim_file);
    else {
	    scanf("%d", &cycles);
	    run(cycles);
    }
    break;

  default:
    printf("Invalid Command\n");
    break;
  }
}

/***************************************************************/
/*                                                             */
/* Procedure : init_memory                                     */
/*                                                             */
/* Purpose   : Zero out the memory array                       */
/*                                                             */
/***************************************************************/
void init_memory() {                                           
  int i;

  for (i=0; i < WORDS_IN_MEM; i++) {
    MEMORY[i][0] = 0;
    MEMORY[i][1] = 0;
  }
}

/**************************************************************/
/*                                                            */
/* Procedure : load_program                                   */
/*                                                            */
/* Purpose   : Load program and service routines into mem.    */
/*                                                            */
/**************************************************************/
void load_program(char *program_filename) {                   
  FILE * prog;
  int ii, word, program_base;

  /* Open program file. */
  prog = fopen(program_filename, "r");
  if (prog == NULL) {
    printf("Error: Can't open program file %s\n", program_filename);
    exit(-1);
  }

  /* Read in the program. */
  if (fscanf(prog, "%x\n", &word) != EOF)
    program_base = word >> 1;
  else {
    printf("Error: Program file is empty\n");
    exit(-1);
  }

  ii = 0;
  while (fscanf(prog, "%x\n", &word) != EOF) {
    /* Make sure it fits. */
    if (program_base + ii >= WORDS_IN_MEM) {
	    printf("Error: Program file %s is too long to fit in memory. %x\n",
             program_filename, ii);
	    exit(-1);
    }

    /* Write the word to memory array. */
    MEMORY[program_base + ii][0] = word & 0x00FF;
    MEMORY[program_base + ii][1] = (word >> 8) & 0x00FF;
    ii++;
  }

  if (CURRENT_LATCHES.PC == 0) CURRENT_LATCHES.PC = (program_base << 1);

  printf("Read %d words from program into memory.\n\n", ii);
}

/************************************************************/
/*                                                          */
/* Procedure : initialize                                   */
/*                                                          */
/* Purpose   : Load machine language program                */ 
/*             and set up initial state of the machine.     */
/*                                                          */
/************************************************************/
void initialize(char *program_filename, int num_prog_files) { 
  int i;

  init_memory();
  for ( i = 0; i < num_prog_files; i++ ) {
    load_program(program_filename);
    while(*program_filename++ != '\0');
  }
  CURRENT_LATCHES.Z = 1;  
  NEXT_LATCHES = CURRENT_LATCHES;
    
  RUN_BIT = TRUE;
}

/***************************************************************/
/*                                                             */
/* Procedure : main                                            */
/*                                                             */
/***************************************************************/
int main(int argc, char *argv[]) {                              
  FILE * dumpsim_file;

  /* Error Checking */
  if (argc < 2) {
    printf("Error: usage: %s <program_file_1> <program_file_2> ...\n",
           argv[0]);
    exit(1);
  }

  printf("LC-3b Simulator\n\n");

  initialize(argv[1], argc - 1);

  if ( (dumpsim_file = fopen( "dumpsim", "w" )) == NULL ) {
    printf("Error: Can't open dumpsim file\n");
    exit(-1);
  }

  while (1)
    get_command(dumpsim_file);
    
}

/***************************************************************/
/* Do not modify the above code.
   You are allowed to use the following global variables in your
   code. These are defined above.

   MEMORY

   CURRENT_LATCHES
   NEXT_LATCHES

   You may define your own local/global variables and functions.
   You may use the functions to get at the control bits defined
   above.

   Begin your code here 	  			       */

/***************************************************************/



void process_instruction(){
  /*  function: process_instruction
   *  
   *    Process one instruction at a time  
   *       -Fetch one instruction
   *       -Decode 
   *       -Execute
   *       -Update NEXT_LATCHES
   */     
  int currentInstruction = MEMORY[CURRENT_LATCHES.PC][0] | (MEMORY[CURRENT_LATCHES.PC][1] << 8);
  int opcode = ((MEMORY[CURRENT_LATCHES.PC][1]) & 0xF0) >> 4;//extracting opcode
  switch(opcode.PC)
  {
      case ADD:
          addInstruction();
          break;
      case AND:
          andInstruction();
          break;
      case HALT:
          haltInstruction();
          break;
      case JMP:
          jmpInstruction();
          break;
      case JSR:
          jsrJsrrrInstruction();
          break;
      case JSRR:
          jsrJsrrrInstruction();          
          break;
      case LDB:
          ldbInstruction();
          break;
      case LDW:
          ldwInstruction();
          break;
      case LEA:
          leaInstruction();
          break;
      case NOP:
          nopInstruction();
          break;
      case NOT:
          notInstruction();
          break;
      case RET:
          retInstruction();
          break;
      case LSHF:
          shfInstruction();
          break;
      case RSHFL:
          shfInstruction();
          break;
      case RSHFA:
          shfInstruction();
          break;
      case RTI:
          rtiInstruction();
          break;
      case STB:
          stbInstruction();
          break;
      case STW:
          stwInstruction();
          break;
      case TRAP:
          trapInstruction();
          break;
      case XOR:
          xorInstruction();
          break;
      case BRN:
          brInstruction();
          break;
      case BRZ:
          brInstruction();
          break;
      case BRP:
          brInstruction();
          break;
      case BR:
          brInstruction();
          break;
      case BRZP:
          brInstruction();
          break;
      case BRNP:
          brInstruction();
          break;
      case BRNZ:
          brInstruction();
          break;
      case BRNZP:
          brInstruction();
          break;
  }
}

int getSrcReg(int upperMem, int lowerMem)
{
  return (((upperMem)) &0x1 <<2) | ((lowerMem &0xC0) >> 6);
}
int getDesReg(int upperMem)
{
  return (upperMem & 0xE) >> 1;
}
void addInstruction()
{
  int desReg = getDesReg(MEMORY[CURRENT_LATCHES.PC][1]);
  int srcReg = getSrcReg(MEMORY[CURRENT_LATCHES.PC][1], MEMORY[CURRENT_LATCHES.PC][0]);
  int desRegValue = CURRENT_LATCHES.REGS[desReg];
  int srcRegValue = CURRENT_LATCHES.REGS[srcReg];
  int finalArg;
  int finalArgValue;
  int valueToStore;
  if(MEMORY[CURRENT_LATCHES.PC][0] & 0x2)
  {
    finalArg = MEMORY[CURRENT_LATCHES.PC][0] & 0x1F;
    finalArgValue = finalArgValue;
  }
  else
  {
    finalArg = MEMORY[CURRENT_LATCHES.PC][0] & 0x7;
    finalArgValue = CURRENT_LATCHES.REGS[finalArgValue];
  }
  valueToStore = (Low16bits(srcRegValue) + Low16bits(finalArgValue));
  NEXT_LATCHES.N = SetN(valueToStore);
  NEXT_LATCHES.P = SetP(valueToStore);
  NEXT_LATCHES.Z = SetZ(valueToStore);
  NEXT_LATCHES.REGS[desReg] = Low16bits(valueToStore);
  NEXT_LATCHES.PC = CURRENT_LATCHES.PC +=0x2;
  return;
}
void andInstruction()
{
  int desReg = getDesReg(MEMORY[CURRENT_LATCHES.PC][1]);
  int srcReg = getSrcReg(MEMORY[CURRENT_LATCHES.PC][1], MEMORY[CURRENT_LATCHES.PC][0]);
  int desRegValue = CURRENT_LATCHES.REGS[desReg];
  int srcRegValue = CURRENT_LATCHES.REGS[srcReg];
  int finalArg;
  int finalArgValue;
  int valueToStore;
  if(MEMORY[CURRENT_LATCHES.PC][0] & 0x2)
  {
    finalArg = MEMORY[CURRENT_LATCHES.PC][0] & 0x1F;
    finalArgValue = finalArgValue;
  }
  else
  {
    finalArg = MEMORY[CURRENT_LATCHES.PC][0] & 0x7;
    finalArgValue = CURRENT_LATCHES.REGS[finalArgValue];
  }
  valueToStore = (Low16bits(srcRegValue) & Low16bits(finalArgValue));
  NEXT_LATCHES.N = SetN(valueToStore);
  NEXT_LATCHES.P = SetP(valueToStore);
  NEXT_LATCHES.Z = SetZ(valueToStore);
  NEXT_LATCHES.REGS[desReg] = Low16bits(valueToStore);
  NEXT_LATCHES.PC = CURRENT_LATCHES.PC +=0x2;
  return;
}
void xorInstruction()
{
  int desReg = getDesReg(MEMORY[CURRENT_LATCHES.PC][1]);
  int srcReg = getSrcReg(MEMORY[CURRENT_LATCHES.PC][1], MEMORY[CURRENT_LATCHES.PC][0]);
  int desRegValue = CURRENT_LATCHES.REGS[desReg];
  int srcRegValue = CURRENT_LATCHES.REGS[srcReg];
  int finalArg;
  int finalArgValue;
  int valueToStore;
  if(MEMORY[CURRENT_LATCHES.PC][0] & 0x2)
  {
    finalArg = MEMORY[CURRENT_LATCHES.PC][0] & 0x1F;
    finalArgValue = finalArgValue;
  }
  else
  {
    finalArg = MEMORY[CURRENT_LATCHES.PC][0] & 0x7;
    finalArgValue = CURRENT_LATCHES.REGS[finalArgValue];
  }
  valueToStore = (Low16bits(srcRegValue) ^ Low16bits(finalArgValue));
  NEXT_LATCHES.N = SetN(valueToStore);
  NEXT_LATCHES.P = SetP(valueToStore);
  NEXT_LATCHES.Z = SetZ(valueToStore);
  NEXT_LATCHES.REGS[desReg] = Low16bits(valueToStore);
  NEXT_LATCHES.PC = CURRENT_LATCHES.PC +=0x2;
  return;
}

void haltInstruction()
{
  NEXT_LATCHES.PC = 0; //I think this just needs to set PC to 0 so program will exit
}

void jmpInstruction()
{
  int srcReg = getSrcReg(MEMORY[CURRENT_LATCHES.PC][1], MEMORY[CURRENT_LATCHES.PC][0]);
  int srcRegValue = Low16bits(CURRENT_LATCHES.REGS[srcReg]);
  NEXT_LATCHES.PC = Low16bIts(srcRegValue);
}
void jsrJsrrrInstruction()
{
  int upperMemory = Low16Bits(MEMORY[CURRENT_LATCHES.PC][1]);
  int lowerMemory = Low16Bits(MEMORY[CURRENT_LATCHES.PC][0]);
  if(upperMemory & 0x08)  //jsr
  {
    int pcOffset11 = ((upperMemory & 0x7) << 8) | lowerMemory;
    pcOffset11 = pcOffset11 << 1;
    NEXT_LATCHES.PC = Low16bits(pcOffset11);
  }
  else//jsrr
  {
    int srcReg = getSrcReg(upperMemory, lowerMemory);
    int srcRegValue = CURRENT_LATCHES.REGS[srcReg];
    NEXT_LATCHES.PC = Low16bits(srcRegValue);
  }
  return;
}

void ldbInstruction()
{
  NEXT_LATCHES.PC = CURRENT_LATCHES.PC +0x2;
  int desReg = getDesReg(MEMORY[CURRENT_LATCHES.PC][1]);
  int srcReg = getSrcReg(MEMORY[CURRENT_LATCHES.PC][1], MEMORY[CURRENT_LATCHES.PC][0]);
  int srcRegValue = CURRENT_LATCHES.REGS[srcReg];
  int bOffset6 = MEMORY[CURRENT_LATCHES.PC][0] & 0x3F;
  int finalValue = Low16bits(bOffset6 + srcRegValue);

  NEXT_LATCHES.N = SetN(finalValue);
  NEXT_LATCHES.P = SetP(finalValue);
  NEXT_LATCHES.Z = SetZ(finalValue);
  NEXT_LATCHES.REGS[desReg] = Low16bits(finalValue);
  return;
}

void ldwInstruction()
{
  NEXT_LATCHES.PC = CURRENT_LATCHES.PC +0x2;
  int desReg = getDesReg(MEMORY[CURRENT_LATCHES.PC][1]);
  int srcReg = getSrcReg(MEMORY[CURRENT_LATCHES.PC][1], MEMORY[CURRENT_LATCHES.PC][0]);
  int srcRegValue = CURRENT_LATCHES.REGS[srcReg];
  int bOffset6 = MEMORY[CURRENT_LATCHES.PC][0] & 0x3F;
  int finalValue = Low16bits(bOffset6 + srcRegValue);

  NEXT_LATCHES.N = SetN(finalValue);
  NEXT_LATCHES.P = SetP(finalValue);
  NEXT_LATCHES.Z = SetZ(finalValue);
  NEXT_LATCHES.REGS[desReg] = Low16bits(finalValue);
  return;
}

void leaInstruction()
{
  NEXT_LATCHES.PC = CURRENT_LATCHES.PC +0x2;//increment PC
  int desReg = getDesReg(MEMORY[CURRENT_LATCHES.PC][1]);//get reg 
  int pcOffsetP = MEMORY[CURRENT_LATCHES.PC][0] | ((MEMORY[CURRENT_LATCHES.PC][1] & 0x1) << 8);
  pcOffsetP << 1;//get pcOffset and left shift one
  NEXT_LATCHES.REGS[desReg] = Low16bits(pcOffsetP);//store in desReg of NextLatches
  return;//does NOT setcc's
}

void nopInstruction()
{
  NEXT_LATCHES.PC = CURRENT_LATCHES.PC +0x2;
  return;
}

void notInstruction()
{
  NEXT_LATCHES.PC = CURRENT_LATCHES.PC+0x2;
  int desReg = getDesReg(MEMORY[CURRENT_LATCHES.PC][1]);
  int srcReg = getSrcReg(MEMORY[CURRENT_LATCHES.PC][1], MEMORY[CURRENT_LATCHES.PC][0]);
  int srcRegValue = CURRENT_LATCHES.REGS[srcReg];
  srcRegValue = ~srcRegValue;
  srcRegValue = Low16bits(srcRegValue);
  NEXT_LATCHES.REGS[desReg] = Low16bits(srcRegValue);
  NEXT_LATCHES.N = SetP(srcRegValue);
  NEXT_LATCHES.P = SetN(srcRegValue);
  NEXT_LATCHES.Z = SetZ(srcRegValue);
  return;
}

void retInstruction()
{
  NEXT_LATCHES.PC = CURRENT_LATCHES.REGS[7];
}

void shfInstruction()
{
  NEXT_LATCHES.PC = CURRENT_LATCHES.PC +0x2;//increment PC
  int desReg = getDesReg(MEMORY[CURRENT_LATCHES.PC][1]);
  int srcReg = getSrcReg(MEMORY[CURRENT_LATCHES.PC][1], MEMORY[CURRENT_LATCHES.PC][0]);
  int srcRegValue = CURRENT_LATCHES.REGS[srcReg];
  int lowerMemory = MEMORY[CURRENT_LATCHES.PC][0];
  int shfAmount = MEMORY[CURRENT_LATCHES.PC][0] & 0x04;
  if(lowerMemory & 0x10 == 0x10)//lshf
  {
    srcRegValue = Low16Bits(srcRegValue << shfAmount);
    NEXT_LATCHES.REGS[desReg] = srcRegValue;
  }
  else
  {
    if(lowerMemory& 0x20 == 0x20)//rshf 0's shifted into vacant positions
    {
      srcRegValue = Low16Bits(srcRegValue);//clears upper bits thus 0's will be shifted by >>
      srcRegValue >> shfAmount;
    }
    else
    {
      srcRegValue >> shfAmount;         
      srcRegValue = Low16Bits(srcRegValue);
    }
  }
  NEXT_LATCHES.N = SetN(srcRegValue);//set cc's
  NEXT_LATCHES.P = SetP(srcRegValue);
  NEXT_LATCHES.Z = SetZ(srcRegValue);
  NEXT_LATCHES.REGS[desReg] = srcRegValue;//change reg value
}

void rtiInstruction()
{
  return;//dummy function, file will NOT have RTI but
        //deleting this would make me have to change my enum Types
        //for opcode and would be more trouble than it is worth
}

void stbInstruction()
{
  NEXT_LATCHES.PC = CURRENT_LATCHES.PC+0x2;
  int lowerMemory = MEMORY[CURRENT_LATCHES.PC][0];
  int offset6 = lowerMemory & 0x3F;

  int srcReg = getDesReg(MEMORY[CURRENT_LATCHES.PC][1]);
  int srcRegValue = CURRENT_LATCHES.REGS[srcReg];

}

void brInstruction()
{
  int currentN = CURRENT_LATCHES.N;
  int currentP = CURRENT_LATCHES.P;
  int currentZ = CURRENT_LATCHES.Z;
  int inputN = (MEMORY[CURRENT_LATCHES.PC][1] & 0x08) >> 3;
  int inputZ = (MEMORY[CURRENT_LATCHES.PC][1] & 0x04) >> 2;
  int inputP = (MEMORY[CURRENT_LATCHES.PC][1] & 0x02) >> 1;
  int unconditionalBranch = !(inputN || inputZ || inputP);
  int pcOffset9 = MEMORY[CURRENT_LATCHES.PC][0] | (MEMORY[CURRENT_LATCHES.PC][1]) & 0x01 << 8;
  if((currentN && inputN) || (currentP && inputP) || (currentZ && inputZ) || unconditionalBranch)
  {
    NEXT_LATCHES.PC = pcOffset9 << 1;
  }
  return;
}