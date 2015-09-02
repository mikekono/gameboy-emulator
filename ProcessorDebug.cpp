#include "ProcessorDebug.h"
// BANG
#include <unistd.h>

#include <iostream>
#include <fstream>
using namespace std; 


void ProcessorDebug::loadMem(std::string rom){
  ifstream file(rom, ios::in|ios::binary|ios::ate);
  streampos size;
  char * memblock;
  size = file.tellg();
  memblock = new char [size];
  file.seekg (0, ios::beg);
  file.read (memblock, size);
  file.close();
  printf("%i \n", size);
  for (unsigned int i = 0; i < 0x8000; i++){
    //printf("%i \n", (unsigned char)memblock[i]);
    mem.write(i, (unsigned char)memblock[i]);

  }
  reg.PC = 0x100;

  // mem.write(0, 0x80);
  // mem.write(1, 0xAF);
  // mem.write(2, 0xC6);
  // mem.write(3, 0x3C);
  // mem.write(4, 0x3C);
  // mem.write(5, 0x3C);
  // mem.write(6, 0x22);
  // mem.write(7, 0x90);
  // mem.write(8, 0x47);

  // mem.write(9, 0x09);
  // mem.write(10, 0x34);
  // mem.write(11, 0x34);
  // mem.write(12, 0x01);
  // mem.write(13, 0xEA);
  // mem.write(14, 0x0B);
  // mem.write(15, 0xC5);
  // mem.write(16, 0x0B);
  // mem.write(17, 0xC1);

  // mem.write(18, 0xCB);
  // mem.write(19, 0xE2);
  // mem.write(20, 0xCB);
  // mem.write(21, 0x20);  
  // // mem.write(18, 0x2F);
  // // mem.write(19, 0xC4);
  // // mem.write(20, 0x01);
  // // mem.write(21, 0xFF);

  // mem.write(22, 0x09);
  // mem.write(23, 0x34);
  // mem.write(24, 0x34);

  // mem.write(25, 0xCB);
  // mem.write(26, 0x30);

  // mem.write(27, 0xCB);
  // mem.write(28, 0x00);

  // mem.write(29, 0x79);
  // mem.write(30, 0x0F);
  // mem.write(31, 0x20); 
  // mem.write(32, 0xFD); 

  // mem.write(0x01FF, 0xC9);


}

void ProcessorDebug::run(){
  loadMem("tetris.gb");
  for (int i = 0; i < 5000; i++){
    debugTick();
  }
}


// Ticks the processor with the supplied opcode, and also displays debug info
void ProcessorDebug::debugTick(){
  byte opcode = mem.read(reg.PC);
  instructionCount++;
  saveState();
  if (opcode != 0xCB){
    std::string opcode_descr = opcode_description_table[opcode]; // holds the instruction name in english. e.g. 'ADD A, B'
    printf(" -----------------------------------------\n");
    printf("| #%i | OPCODE: %02X -> %s \n", instructionCount, opcode, opcode_descr.c_str());
    printf(" -----------------------------------------\n"); 
    tick();
    printOperandValues(opcode, operand1_type_table[opcode], operand2_type_table[opcode]);
  }
  else {
    byte opcode2 = mem.read(reg.PC + 1);
    std::string opcode_descr = opcode_description_table_CB[opcode2];
    printf(" -----------------------------------------\n");
    printf("| #%i | OPCODE: %02X %02X -> %s \n", instructionCount, opcode, opcode2, opcode_descr.c_str());
    printf(" -----------------------------------------\n"); 
    tick();
    printOperandValues(opcode2, operand1_type_table_CB[opcode], operand2_type_table_CB[opcode]);
  }
  compareStates();
  compareRegPtr();
  printFlags();
  printf("\n");
}

void ProcessorDebug::printFlags(){
  printf("Flags: %02X\n", *reg.pF);
}

// Prints the values of the register / memory operands in the instruction so that we know 
// what values we are adding, loading, etc. 
void ProcessorDebug::printOperandValues(byte opcode, byte op1Type, byte op2Type){
  if (op1Type != 0){
    printf("Operand Values: \n");
    if (op1Type == IMM16_MEM_OP || op1Type == IMM16_OP || op1Type == REG16_OP){
      printf("* 1: %04X\n", wordOperand1);
    }
    else { 
      printf("* 1: %02X\n", byteOperand1);
    }
  }
  if (op2Type != 0){
    if (op2Type == IMM16_MEM_OP || op2Type == IMM16_OP || op2Type == REG16_OP){
      printf("* 2: %04X\n", wordOperand2);
    }
    else { 
      printf("* 2: %02X\n", byteOperand2);
    }
  }
}

// saves the current register and memory values in the processor. Useful for later comparison
void ProcessorDebug::saveState() {
  lastMem = mem;
  lastReg = reg;
}

// compares the current memory values with those stored by the last saveState() function
void ProcessorDebug::compareMem(){
  bool memIsChanged = false;
  printf("Mem Write List:\n");
  for (unsigned short i = 0; i < 0xFFFF; i++){
    if (lastMem.read(i) != mem.read(i)){
      printf("* [%04x]: %02x -> %02x\n", i, lastMem.read(i), mem.read(i));
      memIsChanged = true;
    }
  }
  if (memIsChanged == false){
    printf("* NONE\n");
  }
}

// compares the current register values with those stored by the last saveState() function
void ProcessorDebug::compareReg() {
  bool regIsChanged = false;
  printf("Reg Write List:\n");
  if (lastReg.AF != reg.AF){
    printf("* AF: %04x -> %04x \n", lastReg.AF, reg.AF);
    regIsChanged = true;
  }
  if (lastReg.BC != reg.BC){
    printf("* BC: %04x -> %04x \n", lastReg.BC, reg.BC);
    regIsChanged = true;
  }    
  if (lastReg.DE != reg.DE){
    printf("* DE: %04x -> %04x \n", lastReg.DE, reg.DE);
    regIsChanged = true;
  }   
  if (lastReg.HL != reg.HL){
    printf("* HL: %04x -> %04x \n", lastReg.HL, reg.HL);
    regIsChanged = true;
  }   
  if (lastReg.SP != reg.SP){
    printf("* SP: %04x -> %04x \n", lastReg.SP, reg.SP);
    regIsChanged = true;
  }   
  if (lastReg.PC != reg.PC){
    printf("* PC: %04x -> %04x \n", lastReg.PC, reg.PC);
    regIsChanged = true;
  }   
  if (regIsChanged == false){
    printf("* NONE\n");
  }
}

void ProcessorDebug::compareRegPtr() {
  bool regIsChanged = false;
  if (lastReg.pAF != reg.pAF){
    printf("* AF: %04x -> %04x \n", lastReg.AF, reg.AF);
    regIsChanged = true;
  }
  if (lastReg.pBC != reg.pBC){
    printf("* BC: %04x -> %04x \n", lastReg.BC, reg.BC);
    regIsChanged = true;
  }    
  if (lastReg.pDE != reg.pDE){
    printf("* DE: %04x -> %04x \n", lastReg.DE, reg.DE);
    regIsChanged = true;
  }   
  if (lastReg.pHL != reg.pHL){
    printf("* HL: %04x -> %04x \n", lastReg.HL, reg.HL);
    regIsChanged = true;
  }   
  if (lastReg.pSP != reg.pSP){
    printf("* SP: %04x -> %04x \n", lastReg.SP, reg.SP);
    regIsChanged = true;
  }   
  if (lastReg.pPC != reg.pPC){
    printf("* PC: %04x -> %04x \n", lastReg.PC, reg.PC);
    regIsChanged = true;
  }   
  if (regIsChanged == false){
  }
  else {
    printf("***!!!!*** ERROR: REG PTR CHANGE !!!!:\n");
  }
}



int main(int argc, char* argv[]) {
    if (argc < 2){
	printf("Please specify the rom file.\n");
	return -1;
    }
    char* rom = argv[1];
  Memory mem;

  ProcessorDebug debug(mem);

  Graphics graphics(mem);
  graphics.init();
  graphics.draw();
  debug.loadMem(rom);

  while (true){
      //printf("PC: %x AF: %x BC: %x DE: %x HL: %x SP: %x OP: %x %x\n", debug.reg.PC, debug.reg.AF, debug.reg.BC, debug.reg.DE, debug.reg.HL, debug.reg.SP, debug.mem.read(debug.reg.PC), mem.read(debug.reg.PC + 1));
    if (debug.quit){
      break;
    }
    debug.tick();
    graphics.step(debug.numInstCycles);
  }


  return 0;
}
