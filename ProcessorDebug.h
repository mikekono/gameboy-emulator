#ifndef GBEMU_PROCESSOR_DEBUG_H
#define GBEMU_PROCESSOR_DEBUG_H

#include <string>
#include "Processor.cpp"

class ProcessorDebug : public Processor{
 public:
  int instructionCount;
  Memory lastMem;
  Register lastReg;
  const std::string opcode_description_table [0x100] = {TABLE_OPCODE_DESCRIPTION};
  const std::string opcode_description_table_CB [0x100] = {TABLE_OPCODE_DESCRIPTION_CB};

 ProcessorDebug(Memory& _mem) : Processor(_mem){
    lastMem;
    lastReg;
    instructionCount = 0;
  }

  void loadMem(std::string rom);
  void run();
  void debugTick();
  void printFlags();
  void printOperandValues(byte opcode, byte op1Type, byte op2Type);
  void saveState();
  void compareMem();
  void compareReg();

  void compareRegPtr(); // BANG

  // compares both the current memory and register values with those stored by the last saveState() function
  inline void compareStates(){
    compareReg();
    compareMem();
  }
};

#endif
