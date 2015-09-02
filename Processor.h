#ifndef GBEMU_PROCESSOR_H
#define GBEMU_PROCESSOR_H

#include "table_defines.h"
#include "typedef.h"
#include "Memory.cpp"
#include "Register.cpp"
#include "Graphics.cpp"

class Processor {
 public: 
  // The gameboy class passes a reference to the Memory so more classes can use it. 
  Processor(Memory& _mem) : mem(_mem){ // Use Member Initializer List
    incDec = 1;
    interruptMasterEnable = false;
    prepDI = 0;
    prepEI = 0;
    numInstCycles = 0;

    // BANG - PUT IN RST SEQUENCE ASAP!
    reg.AF = 0x01B0;
    reg.BC = 0x0013;
    reg.DE = 0x00d8;
    reg.HL = 0x014d;
    reg.SP = 0xFFFE;
    
    quit = false;
  }
  bool determineBranch(byte branchType);
  void setInterruptFlag(byte interrupt);
  void determineOperands(byte opcode, byte operand1Type, byte operand2Type, byte * op1Ptr, byte * op2Ptr);

  void handleInterrupt();
  void checkKeypress();

  void tick();
  void handleTimer(int numInstCycles);

  inline void setHalfCarryFlag(byte op1, byte op2){
    byte halfCarry = (((0x0F & op1) + (0x0F & op2)) & 0x10);
    if (halfCarry != 0){
      *reg.pF |= 0x20;
    }
    else {
      *reg.pF &= 0xDF;
    }
  }


  Memory& mem;
  Register reg;

  byte byteOperand1;
  byte byteOperand2;
  byte byteResult;

  word wordOperand1;
  word wordOperand2;
  word wordResult;
  word memWriteAddr;

  bool interruptMasterEnable;
  bool isHalted;

  byte prepDI;
  byte prepEI;

  unsigned int timer;
  unsigned int numInstCycles;

  //SDL Event handling
  SDL_Event event;

  // in order to limit the number of instructions in the opcode sequence, I merged INC and DEC into
  // ADD and SUB respectively. All of the instructions take in pointers to memory/reg addresses, so I need
  // to set up this dummy variable to add or subtract by 1. 
  byte incDec;
  char pcInc;

  bool quit;

  // These are arrays generated in gen_opcode.py that group the memory operands and opcode types together
  // The values are auto-generated into table_defines.h
  const byte result_type_table [256] = {TABLE_RESULT_TYPE};
  const byte opcode_type_table [256] = {TABLE_OP_TYPE};
  byte * operand1_table [256] = {TABLE_OPERAND1};
  const byte operand1_type_table [256] = {TABLE_OPERAND1_TYPE};
  byte * operand2_table [256] = {TABLE_OPERAND2};
  const byte operand2_type_table [256] = {TABLE_OPERAND2_TYPE};

  const byte result_type_table_CB [256] = {TABLE_RESULT_TYPE_CB};
  const byte opcode_type_table_CB [256] = {TABLE_OP_TYPE_CB};
  byte * operand1_table_CB [256] = {TABLE_OPERAND1_CB};
  const byte operand1_type_table_CB [256] = {TABLE_OPERAND1_TYPE_CB};
  byte * operand2_table_CB [256] = {TABLE_OPERAND2_CB};
  const byte operand2_type_table_CB [256] = {TABLE_OPERAND2_TYPE_CB};

  const byte cycle_count_table[256] = {TABLE_CYCLE_COUNT};
  const byte cycle_count_table_CB[256] = {TABLE_CYCLE_COUNT_CB};  

  byte const_int_table [15] = {0, 1, 2, 3, 4, 5, 6, 7, 0x08, 0x10, 0x18, 0x20, 0x28, 0x30, 0x38}; // used for shift an immediate instructions and for RST locations
};

#endif // GBEMU_PROCESSOR_H
