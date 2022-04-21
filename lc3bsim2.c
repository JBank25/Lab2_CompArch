/*
    Name 1: Joshua Urbank
    Name 2: Patrick Brown
    UTEID 1: jbu234
    UTEID 2: pbb468
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

/***************************************************************/
/* A couple of useful definitions.                             */
/***************************************************************/
#define FALSE 0
#define TRUE  1

/***************************************************************/
/* Use this to avoid overflowing 16 bits on the bus.           */
/***************************************************************/
#define Low16bits(x) ((x) & 0xFFFF)


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
  //prog = fopen("input.txt", "r");
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
#define SetN(A) ((A & 0x8000) == 0x8000)
#define SetP(A) ((A & 0x8000) != 0x8000 && (A != 0))
#define SetZ(A) (A == 0)
enum instructions{ADD = 0b0001, AND = 0b0101, JMPRET = 0b1100,
                   JSR = 0b0100, LDB = 0b0010, LDW = 0b0110, LEA = 0b1110,            //enum type used for switch statement when decoding
                   SHF = 0b1101, RTI = 0b1000, STB = 0b0011, STW = 0b0111,
                  TRAP = 0b1111, XOR = 0b1001, BR = 0b0000} code;


int getSrcReg(int upperMem, int lowerMem);
int getDesReg(int upperMem);
int imm5Sext(int value);
int imm6Sext(int value);
int imm9Sext(int value);
int imm16Sext(int value);
void setNextRegs(int skip);

void addInstruction();
void andInstruction();
void haltInstruction();
void retJmpInstruction();
void jsrJsrrrInstruction();
void ldbInstruction();
void ldwInstruction();
void leaInstruction();
void shfInstruction();
void rtiInstruction();
void stbInstruction();
void stwInstruction();
void brInstruction();
void trapInstruction();
void xorNotInstruction();


void process_instruction(){
  /*  function: process_instruction
   *  
   *    Process one instruction at a time  
   *       -Fetch one instruction
   *       -Decode 
   *       -Execute
   *       -Update NEXT_LATCHES
   */     
  int currentInstruction = MEMORY[CURRENT_LATCHES.PC>>1][0] | (MEMORY[CURRENT_LATCHES.PC][1] << 8);
  int upperMemory = (MEMORY[CURRENT_LATCHES.PC>>1][1]);
  int lowerMemory =(MEMORY[CURRENT_LATCHES.PC>>1][0]);
  int opcode = ((MEMORY[CURRENT_LATCHES.PC>>1][1]) & 0xF0) >> 4;//extracting opcode


  switch(opcode)
  {
      case ADD:
          addInstruction();
          break;
      case AND:
          andInstruction();
          break;
      case JMPRET:
          retJmpInstruction();
          break;
      case JSR:
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
      case SHF:
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
          xorNotInstruction();
          break;
      case BR:
          brInstruction();
          break;
  }
}
int getSrcReg(int upperMem, int lowerMem)
{
  return (((upperMem &0x1) <<2) | ((lowerMem &0xC0) >> 6));
}
/**
 * Get bits [3:1] from the upper memory address
 * and shift them right 1x.
 */
int getDesReg(int upperMem)
{
  return ((upperMem & 0xE) >> 1);
}
int imm5Sext(int value)
{
  int maskBits = 0xFFFFFFE0;
  if((value & 0x10) != 0x10)//bit 4 should be set to SEXT
    return value;
  return (value | maskBits);
}
int imm6Sext(int value)
{
  int maskBits = 0xFFFFFF30;
  if((value & 0x20) != 0x20)//bit 4 should be set to SEXT
    return value;
  return (value | maskBits);
}
int imm9Sext(int value)
{
  int maskBits = 0xFFFFFE00;
  if((value & 0x100) != 0x100)//bit 8 should be set to SEXT
    return value;
  return (value | maskBits);
}
int imm16Sext(int value)
{
  int maskBits = 0xFFFF0000;
  if((value & 0x8000) != 0x8000)
    return value;
  return (value | maskBits);
}
void setNextRegs(int skip)
{
  for(int i = 0; i < 8; i++)
  {
    if(i == skip)
      continue;
    else
      NEXT_LATCHES.REGS[i] = CURRENT_LATCHES.REGS[i];
  }
}
void addInstruction()
{
  int desReg = getDesReg(MEMORY[CURRENT_LATCHES.PC>>1][1]);
  int srcReg = getSrcReg(MEMORY[CURRENT_LATCHES.PC>>1][1], MEMORY[CURRENT_LATCHES.PC>>1][0]);
  int srcRegValue = CURRENT_LATCHES.REGS[srcReg];
  srcRegValue = imm16Sext(srcRegValue);
  int finalArg;
  int finalArgValue;
  int valueToStore;
  if((MEMORY[CURRENT_LATCHES.PC>>1][0] & 0x20) == 0x20)//bit5 set means imm5 value
  {
    finalArg = MEMORY[CURRENT_LATCHES.PC>>1][0] & 0x1F;//raw bit values
    finalArgValue = imm5Sext(finalArg);//will SEXT the 16 bit int as needed
  }
  else
  {
    finalArg = MEMORY[CURRENT_LATCHES.PC>>1][0] & 0x7;
    printf("Src Reg 2: %x\n", finalArg);
    finalArgValue = CURRENT_LATCHES.REGS[finalArg];
    finalArgValue = imm16Sext(finalArgValue);
  }
  valueToStore = Low16bits((srcRegValue + finalArgValue));
  printf("des Reg: %d Src Reg: %d srcRegValue: %d finalArg: %d Value Stored: %d\n", desReg, srcReg, srcRegValue, finalArgValue, valueToStore);
  NEXT_LATCHES.N = SetN(valueToStore);
  NEXT_LATCHES.P = SetP(valueToStore);
  NEXT_LATCHES.Z = SetZ(valueToStore);
  printf("N:%d Z:%d P:%d\n", NEXT_LATCHES.N, NEXT_LATCHES.Z, NEXT_LATCHES.P);
  NEXT_LATCHES.REGS[desReg] = Low16bits(valueToStore);
  setNextRegs(desReg);
  NEXT_LATCHES.PC = CURRENT_LATCHES.PC +=0x2;
  return;
}
void andInstruction()
{
  int desReg = getDesReg(MEMORY[CURRENT_LATCHES.PC>>1][1]);
  int srcReg = getSrcReg(MEMORY[CURRENT_LATCHES.PC>>1][1], MEMORY[CURRENT_LATCHES.PC>>1][0]);
  int srcRegValue = CURRENT_LATCHES.REGS[srcReg];
  int finalArg;
  int finalArgValue;
  int valueToStore;
  if((MEMORY[CURRENT_LATCHES.PC>>1][0] & 0x20) == 0x20)//imm5 mode
  {
    finalArg = MEMORY[CURRENT_LATCHES.PC>>1][0] & 0x1F;
    finalArgValue = imm5Sext(finalArg);
  }
  else//reg mode
  {
    finalArg = MEMORY[CURRENT_LATCHES.PC>>1][0] & 0x7;
    finalArgValue = CURRENT_LATCHES.REGS[finalArg];
  }
  valueToStore = Low16bits((Low16bits(srcRegValue) & Low16bits(finalArgValue)));
  printf("des Reg: %d Src Reg: %d srcRegValue: %d finalArg: %d Value Stored: %d\n", desReg, srcReg, srcRegValue, finalArgValue, valueToStore);
  NEXT_LATCHES.N = SetN(valueToStore);
  NEXT_LATCHES.P = SetP(valueToStore);
  NEXT_LATCHES.Z = SetZ(valueToStore);
  NEXT_LATCHES.REGS[desReg] = Low16bits(valueToStore);
  printf("N:%d Z:%d P:%d\n", NEXT_LATCHES.N, NEXT_LATCHES.Z, NEXT_LATCHES.P);
  setNextRegs(desReg);
  NEXT_LATCHES.PC = CURRENT_LATCHES.PC +=0x2;
  return;
}
void xorNotInstruction()
{
  int desReg = getDesReg(MEMORY[CURRENT_LATCHES.PC>>1][1]);
  int srcReg = getSrcReg(MEMORY[CURRENT_LATCHES.PC>>1][1], MEMORY[CURRENT_LATCHES.PC>>1][0]);
  int srcRegValue = CURRENT_LATCHES.REGS[srcReg];
  int finalArg;
  int finalArgValue;
  int valueToStore;
  if((MEMORY[CURRENT_LATCHES.PC>>1][0] & 0x20) == 0x20)//imm5 mode
  {
    finalArg = MEMORY[CURRENT_LATCHES.PC>>1][0] & 0x1F;
    finalArgValue = imm5Sext(finalArg);
  }
  else
  {
    finalArg = MEMORY[CURRENT_LATCHES.PC>>1][0] & 0x7;
    finalArgValue = CURRENT_LATCHES.REGS[finalArg];
    printf("Second Reg: %d Second Reg Value: %d\n", finalArg, finalArgValue);
  }
  valueToStore = Low16bits((Low16bits(srcRegValue) ^ Low16bits(finalArgValue)));
  printf("des Reg: %d Src Reg: %d srcRegValue: %d finalArg: %d Value Stored: %d\n", desReg, srcReg, srcRegValue, finalArgValue, valueToStore);
  NEXT_LATCHES.N = SetN(valueToStore);
  NEXT_LATCHES.P = SetP(valueToStore);
  NEXT_LATCHES.Z = SetZ(valueToStore);
  NEXT_LATCHES.REGS[desReg] = Low16bits(valueToStore);
  setNextRegs(desReg);
  NEXT_LATCHES.PC = CURRENT_LATCHES.PC +0x2;
  return;
}

/**
 * 1100 will take srcReg and store its value into PC
 * RET just uses 7th reg only
 */
void retJmpInstruction()
{
  int srcReg = getSrcReg(MEMORY[CURRENT_LATCHES.PC>>1][1], MEMORY[CURRENT_LATCHES.PC>>1][0]);
  int srcRegValue = Low16bits(CURRENT_LATCHES.REGS[srcReg]);
  NEXT_LATCHES.PC = Low16bits(srcRegValue);
  setNextRegs(-1);//do not modify regs
  NEXT_LATCHES.N = CURRENT_LATCHES.N;
  NEXT_LATCHES.P = CURRENT_LATCHES.P;
  NEXT_LATCHES.Z = CURRENT_LATCHES.Z;
}
void jsrJsrrrInstruction()
{
  int upperMemory = Low16bits(MEMORY[CURRENT_LATCHES.PC>>1][1]);
  int lowerMemory = Low16bits(MEMORY[CURRENT_LATCHES.PC>>1][0]);
  int temp = CURRENT_LATCHES.PC + 0x2;
  if((upperMemory & 0x08) == 0x08)  //jsr
  {
    printf("JSR instruciton\n");
    int pcOffset11 = ((upperMemory & 0x7) << 8) | lowerMemory;
    printf("Initial offset: %x %d\n", pcOffset11, pcOffset11);
    //sext as needed
    pcOffset11 = ((pcOffset11 & 0x400) == 0x4000) ? (pcOffset11| 0xF800) : pcOffset11;
    printf("Offset Extended: %x %d\n",pcOffset11,pcOffset11);
    pcOffset11 = pcOffset11 << 1;
    printf("Offset Shifted: %x %d\n",pcOffset11,pcOffset11);
    NEXT_LATCHES.PC = Low16bits(pcOffset11) + Low16bits(temp);
  }
  else//jsrr
  {
    printf("JSRR instruciton\n");
    int srcReg = getSrcReg(upperMemory, lowerMemory);
    printf("Src Reg: %d\n", srcReg);
    int srcRegValue = CURRENT_LATCHES.REGS[srcReg];
    printf("SrcRegValue: %d\n", srcRegValue);
    NEXT_LATCHES.PC = Low16bits(srcRegValue);
  }
  setNextRegs(-1);
  NEXT_LATCHES.N = CURRENT_LATCHES.N;
  NEXT_LATCHES.P = CURRENT_LATCHES.P;
  NEXT_LATCHES.Z = CURRENT_LATCHES.Z;
  return;
}
void ldbInstruction()
{
  NEXT_LATCHES.PC = CURRENT_LATCHES.PC +0x2;
  int desReg = getDesReg(MEMORY[CURRENT_LATCHES.PC>>1][1]);
  int srcReg = getSrcReg(MEMORY[CURRENT_LATCHES.PC>>1][1], MEMORY[CURRENT_LATCHES.PC>>1][0]);
  int srcRegValue = CURRENT_LATCHES.REGS[srcReg];
  //printf("DesReg: %d Src Reg: %d Src Reg Value: %d\n", desReg, srcReg, srcRegValue);
  int bOffset6 = MEMORY[CURRENT_LATCHES.PC>>1][0] & 0x3F;
  //printf("Initial offset: %x\n", bOffset6);
  bOffset6 = imm6Sext(bOffset6);
  //printf("SEXT offset: %x\n", bOffset6);
  srcRegValue = imm16Sext(srcRegValue);
  int finalValue = Low16bits((bOffset6 + srcRegValue));
  int upperLower = finalValue%2;
  finalValue = MEMORY[finalValue>>1][upperLower];
  finalValue = ((finalValue&0x80) == 0x80) ? (finalValue| 0xFF00): finalValue;
  NEXT_LATCHES.N = SetN(finalValue);
  NEXT_LATCHES.P = SetP(finalValue);
  NEXT_LATCHES.Z = SetZ(finalValue);
  NEXT_LATCHES.REGS[desReg] = Low16bits(finalValue);
  setNextRegs(desReg);
  return;
}

void ldwInstruction()
{
  NEXT_LATCHES.PC = CURRENT_LATCHES.PC +0x2;
  int desReg = getDesReg(MEMORY[CURRENT_LATCHES.PC>>1][1]);
  int srcReg = getSrcReg(MEMORY[CURRENT_LATCHES.PC>>1][1], MEMORY[CURRENT_LATCHES.PC>>1][0]);
  int srcRegValue = CURRENT_LATCHES.REGS[srcReg];
  srcRegValue = imm16Sext(srcRegValue);

  int bOffset6 = MEMORY[CURRENT_LATCHES.PC>>1][0] & 0x3F;
  bOffset6 = imm6Sext(bOffset6);
  bOffset6 = bOffset6 << 1;

  printf("des Reg: %d Src Reg: %d srcRegValue: %d bOffset6: %d\n", desReg, srcReg, srcRegValue, bOffset6);

  int finalValue = Low16bits((bOffset6 + srcRegValue));
  int upperWord = MEMORY[finalValue>>1][1]&0xFF;
  upperWord = upperWord << 8;
  int lowerWord = MEMORY[finalValue>>1][0]&0xFF;
  int wordToStore = upperWord | lowerWord;

  printf("finalValue: %d lowerWord: %d upperWord: %d wordToStore: %d\n", finalValue, lowerWord, upperWord, wordToStore);

  setNextRegs(desReg);
  NEXT_LATCHES.N = SetN(wordToStore);
  NEXT_LATCHES.P = SetP(wordToStore);
  NEXT_LATCHES.Z = SetZ(wordToStore);
  NEXT_LATCHES.REGS[desReg] = Low16bits(wordToStore);
  return;
}

void leaInstruction()
{
  NEXT_LATCHES.PC = CURRENT_LATCHES.PC +0x2;//increment PC
  int desReg = getDesReg(MEMORY[CURRENT_LATCHES.PC>>1][1]);//get reg 
  int pcOffsetP = MEMORY[CURRENT_LATCHES.PC>>1][0] | ((MEMORY[CURRENT_LATCHES.PC>>1][1] & 0x1) << 8);
  pcOffsetP = imm9Sext(pcOffsetP);
  pcOffsetP = pcOffsetP << 1;//get pcOffset and left shift one
  NEXT_LATCHES.REGS[desReg] =Low16bits(NEXT_LATCHES.PC+ Low16bits(pcOffsetP));//store in desReg of NextLatches
  NEXT_LATCHES.N = CURRENT_LATCHES.N;
  NEXT_LATCHES.P = CURRENT_LATCHES.P;
  NEXT_LATCHES.Z = CURRENT_LATCHES.Z;
  setNextRegs(desReg);
  return;//does NOT setcc's
}

void shfInstruction()
{
  NEXT_LATCHES.PC = CURRENT_LATCHES.PC +0x2;//increment PC
  int desReg = getDesReg(MEMORY[CURRENT_LATCHES.PC>>1][1]);
  int srcReg = getSrcReg(MEMORY[CURRENT_LATCHES.PC>>1][1], MEMORY[CURRENT_LATCHES.PC>>1][0]);
  int srcRegValue = CURRENT_LATCHES.REGS[srcReg];
  int lowerMemory = MEMORY[CURRENT_LATCHES.PC>>1][0];
  int shfAmount = MEMORY[CURRENT_LATCHES.PC>>1][0] & 0x0F;
  printf("Des Reg: %d Src Reg: %d Src Reg Value: %d/%x Shift amount: %d\n", desReg, srcReg, srcRegValue, srcRegValue, shfAmount);
  if((lowerMemory & 0x30) == 0x00)//lshf
  {
    printf("Left shift\n");
    srcRegValue = Low16bits((srcRegValue << shfAmount));
    printf("SrcRegValue after: %x\n");
    NEXT_LATCHES.REGS[desReg] = srcRegValue;
  }
  else
  {
    if((lowerMemory& 0x30) == 0x10)//rshf 0's shifted into vacant positions
    {
      printf("Right shift\n");
      srcRegValue = Low16bits(srcRegValue);//clears upper bits thus 0's will be shifted by >>
      srcRegValue = srcRegValue >> shfAmount;
      printf("SrcRegValue after: %x\n");
      NEXT_LATCHES.REGS[desReg] = srcRegValue;
    }
    else
    {
      printf("Right arithmetic shift\n");
      srcRegValue = ((srcRegValue&0x8000) == 0x8000) ? (srcRegValue | 0xFFFF0000) : srcRegValue; 
      printf("SrcRegValue after SEXT: %x\n", srcRegValue);
      srcRegValue = srcRegValue >> shfAmount;         
      srcRegValue = Low16bits(srcRegValue);
      printf("SrcRegValue after shift: %x\n", srcRegValue);
      NEXT_LATCHES.REGS[desReg] = srcRegValue;
    }
  }
  NEXT_LATCHES.N = SetN(srcRegValue);//set cc's
  NEXT_LATCHES.P = SetP(srcRegValue);
  NEXT_LATCHES.Z = SetZ(srcRegValue);
  setNextRegs(desReg);
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
  int lowerMemory = MEMORY[CURRENT_LATCHES.PC>>1][0];
  int offset6 = lowerMemory & 0x3F;
  offset6 = imm6Sext(offset6);
  int srcReg = getDesReg(MEMORY[CURRENT_LATCHES.PC>>1][1]);
  int srcRegValue = CURRENT_LATCHES.REGS[srcReg];
  int baseReg = getSrcReg(MEMORY[CURRENT_LATCHES.PC>>1][1], lowerMemory);
  int baseRegValue = CURRENT_LATCHES.REGS[baseReg];
  baseRegValue = imm16Sext(baseRegValue);
  int finalMemLoc = Low16bits((baseRegValue + offset6));
  int upperLower = (finalMemLoc %2 == 0);//check if upper or lower mem loc
  MEMORY[finalMemLoc>>1][upperLower] = (0xFF &srcRegValue);
  setNextRegs(-1);
  NEXT_LATCHES.N = CURRENT_LATCHES.N;
  NEXT_LATCHES.P = CURRENT_LATCHES.P;
  NEXT_LATCHES.Z = CURRENT_LATCHES.Z;

}

void stwInstruction()
{
  NEXT_LATCHES.PC = CURRENT_LATCHES.PC+0x2;
  int lowerMemory = MEMORY[CURRENT_LATCHES.PC>>1][0];
  int offset6 = lowerMemory & 0x3F;
  offset6 = imm6Sext(offset6);
  offset6 = offset6 << 1;
  int srcReg = getDesReg(MEMORY[CURRENT_LATCHES.PC>>1][1]);
  int srcRegValue = CURRENT_LATCHES.REGS[srcReg];
  int baseReg = getSrcReg(MEMORY[CURRENT_LATCHES.PC>>1][1], lowerMemory);
  int baseRegValue = CURRENT_LATCHES.REGS[baseReg];
  baseRegValue = imm16Sext(baseRegValue);
  int finalMemLoc = Low16bits((baseRegValue + offset6));
  MEMORY[finalMemLoc>>1][0] = (0x00FF & (srcRegValue));
  MEMORY[finalMemLoc>>1][1] = ((0xFF00 & (srcRegValue) >> 8));
  setNextRegs(-1);
  NEXT_LATCHES.N = CURRENT_LATCHES.N;
  NEXT_LATCHES.P = CURRENT_LATCHES.P;
  NEXT_LATCHES.Z = CURRENT_LATCHES.Z;
}
void brInstruction()
{
  int currentN = CURRENT_LATCHES.N;
  int currentP = CURRENT_LATCHES.P;
  int currentZ = CURRENT_LATCHES.Z;
  int inputN = (MEMORY[CURRENT_LATCHES.PC>>1][1] & 0x08) >> 3;
  int inputZ = (MEMORY[CURRENT_LATCHES.PC>>1][1] & 0x04) >> 2;
  int inputP = (MEMORY[CURRENT_LATCHES.PC>>1][1] & 0x02) >> 1;
  int unconditionalBranch = !(inputN || inputZ || inputP);
  int pcOffset9 = MEMORY[CURRENT_LATCHES.PC>>1][0] | ((MEMORY[CURRENT_LATCHES.PC>>1][1] & 0x01) << 8);
  pcOffset9 = imm9Sext(pcOffset9);
  pcOffset9 = pcOffset9 << 1;
  if((currentN && inputN) || (currentP && inputP) || (currentZ && inputZ) || unconditionalBranch)
  {
    NEXT_LATCHES.PC = pcOffset9 + CURRENT_LATCHES.PC + 0x2;
  }
  else
  {
    NEXT_LATCHES.PC = CURRENT_LATCHES.PC + 0x2;
  }
  NEXT_LATCHES.N = CURRENT_LATCHES.N;
  NEXT_LATCHES.P = CURRENT_LATCHES.P;
  NEXT_LATCHES.Z = CURRENT_LATCHES.Z;
  setNextRegs(-1);
  return;
}

void trapInstruction()
{
  NEXT_LATCHES.REGS[7] = CURRENT_LATCHES.PC+0x2;
  int trapVector = MEMORY[CURRENT_LATCHES.PC>>1][0];
  trapVector = Low16bits(trapVector);
  trapVector = Low16bits((trapVector << 1));
  NEXT_LATCHES.PC = MEMORY[trapVector>>1][0];
  NEXT_LATCHES.N = CURRENT_LATCHES.N;
  NEXT_LATCHES.P = CURRENT_LATCHES.P;
  NEXT_LATCHES.Z = CURRENT_LATCHES.Z;
  setNextRegs(-1);
  return;
}