#include "Processor.h"

void Processor::setInterruptFlag(byte interrupt){
  mem.write(INTERRUPT_FLAG_REG, mem.read(INTERRUPT_FLAG_REG) | interrupt);
}

void Processor::checkKeypress(){
  byte joyp = mem.read(0xff00);
  //printf("%x\n", joyp);
  const Uint8 * keyState = SDL_GetKeyboardState(NULL);
  while (SDL_PollEvent(&event)){
    if (event.type == SDL_KEYDOWN){
      switch(event.key.keysym.sym){
      case SDLK_ESCAPE:
    	printf("EXITING\n");
    	//exit(0);
        quit = true;
        break;
      }
    }
  }
    // P15 out port was selected, we can scan for a, b, select, start keypress
  switch(joyp & 0x30){
  case 0x10:
	//printf("SCANNING JOYP!! \n");
	if (keyState[SDL_SCANCODE_RSHIFT]){
	    //printf("KEYPRESS!! \n");
	    mem.writeKeyPress(0x0B);
	    //printf("FF00 value: %x!!!\n", mem.memory[0xff00]);
	    mem.write(INTERRUPT_FLAG_REG, mem.read(INTERRUPT_FLAG_REG) | 0x10);
	}
	if (keyState[SDL_SCANCODE_RETURN]){
	    //printf("KEYPRESS!! \n");
	    mem.writeKeyPress(0x07);
	    //printf("FF00 value: %x!!!\n", mem.memory[0xff00]);
	    mem.write(INTERRUPT_FLAG_REG, mem.read(INTERRUPT_FLAG_REG) | 0x10);
	}
	if (keyState[SDL_SCANCODE_B]){
	    //printf("KEYPRESS!! \n");
	    mem.writeKeyPress(0x0D);
	    //printf("FF00 value: %x!!!\n", mem.memory[0xff00]);
	    mem.write(INTERRUPT_FLAG_REG, mem.read(INTERRUPT_FLAG_REG) | 0x10);
	}
	if (keyState[SDL_SCANCODE_A]){
	    //printf("KEYPRESS!! \n");
	    mem.writeKeyPress(0x0E);
	    //printf("FF00 value: %x!!!\n", mem.memory[0xff00]);
	    mem.write(INTERRUPT_FLAG_REG, mem.read(INTERRUPT_FLAG_REG) | 0x10);
	}
	break;
  
  case 0x20:
	//printf("SCANNING JOYP!! \n");
	if (keyState[SDL_SCANCODE_RIGHT]){
	    //printf("KEYPRESS!! \n");
	    mem.writeKeyPress(0x0E);
	    //printf("FF00 value: %x!!!\n", mem.memory[0xff00]);
	    mem.write(INTERRUPT_FLAG_REG, mem.read(INTERRUPT_FLAG_REG) | 0x10);
	}
	if (keyState[SDL_SCANCODE_LEFT]){
	    //printf("KEYPRESS!! \n");
	    mem.writeKeyPress(0x0D);
	    //printf("FF00 value: %x!!!\n", mem.memory[0xff00]);
	    mem.write(INTERRUPT_FLAG_REG, mem.read(INTERRUPT_FLAG_REG) | 0x10);
	}
	if (keyState[SDL_SCANCODE_UP]){
	    //printf("KEYPRESS!! \n");
	    mem.writeKeyPress(0x0B);
	    //printf("FF00 value: %x!!!\n", mem.memory[0xff00]);
	    mem.write(INTERRUPT_FLAG_REG, mem.read(INTERRUPT_FLAG_REG) | 0x10);
	}
	if (keyState[SDL_SCANCODE_DOWN]){
	    //printf("KEYPRESS!! \n");
	    mem.writeKeyPress(0x07);
	    //printf("FF00 value: %x!!!\n", mem.memory[0xff00]);
	    mem.write(INTERRUPT_FLAG_REG, mem.read(INTERRUPT_FLAG_REG) | 0x10);
	}
	break;
  case 0x30:
      mem.write(0xFF00, 0xFF);
      break;
  }
    

}

void Processor::handleInterrupt(){
  if (interruptMasterEnable){
    byte interruptEnableReg = mem.read(INTERRUPT_ENABLE_REG);
    byte interruptFlagReg = mem.read(INTERRUPT_FLAG_REG);
    
    byte validInterrupts = interruptEnableReg & interruptFlagReg;
    if (validInterrupts != 0){
      // we have an interrupt. disable master enable and save the current stack pointer
      interruptMasterEnable = false;
      mem.writeWord(reg.SP - 2, reg.PC);
      reg.SP -= 2;
      isHalted = false; // unhalt cpu, if applicable
      
      //printf("INTERRUPT %x PC %x\n", validInterrupts, reg.PC);
      if ((validInterrupts & INTERRUPT_VBLANK) != 0){
	reg.PC = INTERRUPT_HDLR_VBLANK;
	mem.write(INTERRUPT_FLAG_REG, interruptFlagReg & 0xfe);
      }
      else if ((validInterrupts &  INTERRUPT_LCDSTAT) != 0){
	reg.PC = INTERRUPT_HDLR_LCDSTAT;
	mem.write(INTERRUPT_FLAG_REG, interruptFlagReg & 0xfd);
      }
      else if ((validInterrupts & INTERRUPT_TIMER) != 0){
	reg.PC = INTERRUPT_HDLR_TIMER;
	mem.write(INTERRUPT_FLAG_REG, interruptFlagReg & 0xfb);
      }
      else if ((validInterrupts & INTERRUPT_SERIAL) != 0){
	reg.PC = INTERRUPT_HDLR_SERIAL;
      	mem.write(INTERRUPT_FLAG_REG, interruptFlagReg & 0xf7);
      }
      else if ((validInterrupts & INTERRUPT_JOYPAD) != 0){
	reg.PC = INTERRUPT_HDLR_JOYPAD;
	mem.write(INTERRUPT_FLAG_REG, interruptFlagReg & 0xef);
	printf("JOYPAD INT \n");
      }
      pcInc = 0;
    }
  }
}

void Processor::determineOperands(byte opcode, byte operand1Type, byte operand2Type, byte * op1Ptr, byte * op2Ptr){
  // The size of the result is always dependent on the size of the first operand
  switch (operand1Type){
  case MEM_OP: 
    memWriteAddr = *(unsigned short *)op1Ptr;
    byteOperand1 = mem.read(memWriteAddr);
    break;
  case REG8_OP: 
    byteOperand1 = *op1Ptr;
    break;
  case REG16_OP: 
    wordOperand1 = *(unsigned short *)op1Ptr;
    break;
  case IMM8_OP: {
    // immediate values are store in memory with the opcodes
    memWriteAddr = 0xFF00 + reg.PC + 1;
    byteOperand1 = mem.read(memWriteAddr);
    pcInc++;
    break;
  }
  case IMM16_OP: {
    memWriteAddr = reg.PC + 1;
    wordOperand1 = mem.readWord(memWriteAddr);
    break;
  }
  case IMM8_MEM_OP: {
    // immediate values are stored in memory with the opcodes
    unsigned short immAddr = reg.PC + 1;
    memWriteAddr = 0xFF00 + mem.read(immAddr);
    byteOperand1 = mem.read(memWriteAddr);
    pcInc++;
    break;
  }
  case IMM16_MEM_OP: {
    unsigned short immAddr = reg.PC + 1;
    memWriteAddr = mem.readWord(immAddr);
    byteOperand1 = mem.read(memWriteAddr);
    pcInc += 2;
    break;
  }
  case IMM16_MEM16_OP: {
    memWriteAddr = mem.readWord(reg.PC + 1);
    unsigned short addrValue = mem.readWord(memWriteAddr);
    wordOperand1 = mem.readWord(memWriteAddr);
    pcInc += 2;
    break;
  }
  case LD_C_FROM_MEM: {
    memWriteAddr = 0xFF00 + *op1Ptr;
    byteOperand1 = mem.read(memWriteAddr);   
    break;
  }
  }

  switch (operand2Type){
  case MEM_OP:
    byteOperand2 = mem.read(*(unsigned short *)op2Ptr);
    break;
  case REG8_OP: 
    byteOperand2 = *op2Ptr;
    break;
  case REG16_OP: 
    wordOperand2 = *(unsigned short *)op2Ptr;
    break;
  case IMM8_OP: {
    // immediate values are store in memory with the opcodes
    unsigned short immAddress = reg.PC + 1;
    byteOperand2 = mem.read(immAddress);
    pcInc++;
    break;
  }
  case IMM16_OP: { 
    unsigned short immAddress = reg.PC + 1;
    wordOperand2 = mem.readWord(immAddress);	
    pcInc += 2;
    break;
  }
  case IMM8_MEM_OP: {
    // immediate values are stored in memory with the opcodes
    unsigned short immAddress = reg.PC + 1;
    unsigned short addrValue = 0xFF00 + mem.read(immAddress);
    byteOperand2 = mem.read(addrValue);
    pcInc++;
    break;
  }
  case IMM16_MEM_OP: {
    unsigned short immAddress = reg.PC + 1;
    unsigned short addrValue = mem.readWord(immAddress);
    byteOperand2 = mem.read(addrValue);
    pcInc += 2;
    break;
  }
  case ABS16_OP: {
    wordOperand2 = *op2Ptr;
    break;
  }
  case LD_C_FROM_MEM: {
    byteOperand2 = mem.read(0xFF00 + *op2Ptr);   
    break;
  }
  case SP_PLUS_R8_OP: {
    signed char r8 = mem.read(reg.PC + 1);
    wordOperand2 = reg.SP + r8; 
    pcInc++;

    // TODO: I'd rather not have this logic here. Instruction behavior was unexpected
    *reg.pF = 0x0; // clear zero and negative flags
    if (r8 > 0){
	if ((((0x000F & reg.SP) + (0x0F & r8)) & 0x0010) != 0){ // half carry
	    *reg.pF |= 0x20;
	}
	if ((((0x00FF & reg.SP) + r8) & 0x0100) != 0){ // carry is from bit 7 to 8
	    *reg.pF |= 0x10;
	}
    }
    else{
	if ((0x000F & wordOperand2) <= (0x000F & reg.SP)){ // half carry
	    *reg.pF |= 0x20;
	}
	if ((0x00FF & wordOperand2) <= (0x00FF & reg.SP)){ // carry is from bit 7 to 8
	    *reg.pF |= 0x10;
	}
    }
    break;
  }
  }
}

void Processor::tick(){
  handleInterrupt();
  if (isHalted){
      numInstCycles += 1;
      handleTimer(numInstCycles);
      timer += numInstCycles;
      checkKeypress();      
  }

  //determineOperands(opcode);
  pcInc = 1; // the default value that the PC is incremented by. Some instructions vary
  byte opcode = mem.read(reg.PC);
  word * pWordResult;
  byte * pByteResult;
  byte opcodeType, operand1Type, operand2Type, resultType;
  // BANG - only do this one way eventually
  bool isCB = false;

  if (opcode != 0xCB){
    determineOperands(opcode, operand1_type_table[opcode], operand2_type_table[opcode], operand1_table[opcode], operand2_table[opcode]); 
    pWordResult = (unsigned short *)operand1_table[opcode];
    pByteResult = operand1_table[opcode];
    opcodeType = opcode_type_table[opcode];
    resultType = result_type_table[opcode];

    numInstCycles = cycle_count_table[opcode];
  }
  else {
    isCB = true;
    pcInc += 1;
    opcode = mem.read(reg.PC + 1);
    determineOperands(opcode, operand1_type_table_CB[opcode], operand2_type_table_CB[opcode], operand1_table_CB[opcode], operand2_table_CB[opcode]); 
    pWordResult = (unsigned short *)operand1_table_CB[opcode];
    pByteResult = operand1_table_CB[opcode];
    opcodeType = opcode_type_table_CB[opcode];
    resultType = result_type_table_CB[opcode];

    numInstCycles = cycle_count_table_CB[opcode];
  }


  //KEYPAD DEBUG
  // if ((reg.PC >= 0x29fa) && (reg.PC <= 0x2a2a)){
  //     printf("PC: %x, FF00: %x, A: %x, B: %x, C: %x\n", reg.PC, mem.read(0xff00), *reg.pA, *reg.pB, *reg.pC);
  // }

  switch (opcodeType){
  case NOP: break;
  case LD:  
    byteResult = byteOperand2;
    //printf("res: %x addr: %x\n", byteResult, memWriteAddr);
    break;
  case ADD: 
    byteResult = byteOperand1 + byteOperand2;
    *reg.pF = ((byteResult == 0) ? 0x80 : 0x00);
    *reg.pF |= (((0x0F & byteOperand1) + (0x0F & byteOperand2)) & 0x10) << 1; // half carry
    if (byteResult < byteOperand2){ // unsigned types roll over to zero if theres a carry
      *reg.pF |= 0x10;
    }
    else { // Set carry flag to zero
      *reg.pF &= 0xEF;
    }
    break;
  case ADC: {
      byte carry_f = ((*reg.pF & 0x10) >> 4);
      byteResult = byteOperand1 + byteOperand2 + carry_f;
      *reg.pF = ((byteResult == 0) ? 0x80 : 0x00);
      *reg.pF |= (((0x0F & byteOperand1) + (0x0F & byteOperand2) + carry_f) & 0x10) << 1; // half carry
      if ((byteResult < byteOperand1) || (((byteResult == byteOperand1) && (byteResult != 0 || carry_f != 0)) && CARRY_FLAG)){ // unsigned types roll over to zero if theres a carry
	  *reg.pF |= 0x10;
      }
      else { // Set carry flag to zero
	  *reg.pF &= 0xEF;
      }
    }
    break;
  case SUB:
    // BANG - DEC doesnt have carry flag changed!!
    // BANG - half carry flag!
    byteResult = byteOperand1 - byteOperand2;

    *reg.pF = ((byteResult == 0) ? 0xC0 : 0x40);
    // deal with half carry (borrow)
    if ((byteResult & 0x0F) > (byteOperand1 & 0x0F)){
      *reg.pF |= 0x20;
    }
    else { // Set half carry flag to zero
      *reg.pF &= 0xDF;
    }
    if (byteResult > byteOperand1){ // check for carry (borrow)
      *reg.pF |= 0x10;
    }
    else { // Set carry flag to zero
      *reg.pF &= 0xEF;
    }
    break;	
  case SBC: {
      byte carry_f = ((*reg.pF & 0x10) >> 4);
      byteResult = byteOperand1 - byteOperand2 - carry_f;

      *reg.pF = ((byteResult == 0) ? 0xC0 : 0x40);
      // deal with half carry (borrow)
      *reg.pF |= (((0x0F & byteOperand1) - (0x0F & byteOperand2) - carry_f) & 0x80) >> 2; // half carry
      if ((byteResult > byteOperand1) || (((byteOperand1 == byteResult) && (byteOperand2 != 0 || carry_f != 0)) && CARRY_FLAG)){ // check for carry (borrow)
	  *reg.pF |= 0x10;
      }
      else { // Set carry flag to zero
	  *reg.pF &= 0xEF;
      }
    }
    break;	
  case AND:
    byteResult = byteOperand1 & byteOperand2;
    *reg.pF = ((byteResult == 0) ? 0xA0 : 0x20);
    break;
  case OR:
    byteResult = byteOperand1 | byteOperand2;
    *reg.pF = ((byteResult == 0) ? 0x80 : 0x00);
    break;
  case XOR:
    byteResult = byteOperand1 ^ byteOperand2;
    *reg.pF = ((byteResult == 0) ? 0x80 : 0x00);
    break;
  case CP:
    byteResult = byteOperand1 - byteOperand2;
    *reg.pF = ((byteResult == 0) ? 0xC0 : 0x40);
    // deal with half carry (borrow)
    if ((byteResult & 0x0F) > (byteOperand1 & 0x0F)){
      *reg.pF |= 0x20;
    }
    else { // Set half carry flag to zero
      *reg.pF &= 0xDF;
    }
    if (byteResult > byteOperand1){ // check for carry (borrow)
      *reg.pF |= 0x10;
    }
    else { // Set carry flag to zero
      *reg.pF &= 0xEF;
    }
    break;
  case ADD16:
    // BANG - half carry fag for 16 bit
    wordResult = wordOperand1 + wordOperand2;

    if ((((0x0FFF & wordOperand1) + (0x0FFF & wordOperand2)) & 0x1000) != 0){ // half carry
      *reg.pF |= 0x20;
    }
    else{
      *reg.pF &= 0xDF;
    }
    if (wordResult < wordOperand2){ // unsigned types roll over to zero if theres a carry
      *reg.pF |= 0x10;
    }
    else { // Set carry flag to zero
      *reg.pF &= 0xEF;
    }	
    *reg.pF &= 0xBF;
    break;
  case SUB16:
    wordResult = wordOperand1 - wordOperand2;
    if (wordResult > wordOperand1){ // check for carry (borrow)
      *reg.pF |= 0x10;
    }
    else { // Set carry flag to zero
      *reg.pF &= 0xEF;
    }	
    break;      
  case LD16:
    wordResult = wordOperand2;
    break; 
  case PUSH:
    //mem.write(reg.SP - 1, wordOperand1);
    wordResult = wordOperand2;
    memWriteAddr = reg.SP - 2;
    reg.SP -= 2;
    break;
  case POP:
    //*wordOperand1 = *mem.readWord(reg.SP + 1);
    wordResult = mem.readWord(reg.SP);
    reg.SP += 2;
    break;
  case CPL: // complement A register 
    byteResult = ~(byteOperand1);
    *reg.pF |= 0x60;
    break;
  case CCF: // complement carry flag
    *reg.pF ^= 0x10;
    *reg.pF &= 0x90;
    break;
  case SCF: // set carry flag
    *reg.pF |= 0x10;
    *reg.pF &= 0x90;   
    break;
  case HALT: // wait for interrupt
    isHalted = interruptMasterEnable;
    break;
  case DI: 
    prepDI = 1;
    break;
  case EI:
    prepEI = 1;
    break;
  case JP: {
    if (determineBranch(byteOperand1)){
      reg.PC = wordOperand2;
      pcInc = 0;
      numInstCycles = 16;

    }
    else {
      numInstCycles = 12;
    }
  }
    break;
  case JR: {
    // The JR instruction adds n to the current address
    if (determineBranch(byteOperand1)){
      pcInc += (char) byteOperand2;
      numInstCycles = 12;
    }
    else{
      numInstCycles = 8;
    }
  }
    break;
  case CALL:
    if (determineBranch(byteOperand1)){
      memWriteAddr = reg.SP - 2;
      reg.SP -= 2;      
      wordResult = reg.PC + pcInc;
      reg.PC = wordOperand2;
      pcInc = 0;
      numInstCycles = 24;
    }
    else {
      numInstCycles = 12;
      // BANG!! Hack to prevent writing to an address when we aren't branching
      // fixes bug - just rewriting the same bit of mem for now...
      memWriteAddr = 0xC000; // has to be RAM
      wordResult = mem.readWord(memWriteAddr);
    }
    break;
  case RET:
    if (determineBranch(byteOperand1)){
      reg.PC = mem.readWord(reg.SP);
      reg.SP += 2;     
      pcInc = 0;
      numInstCycles = 20;
    }
    else {
      numInstCycles = 8;
    }
    break;
  case RETI:
    reg.PC = mem.readWord(reg.SP);
    reg.SP += 2;     
    pcInc = 0;
    interruptMasterEnable = true;
    break;
  case DAA: // BANG - may have to fix later
    {
    int num = byteOperand1;
    if  ((*reg.pF & 0x40) == 0) {
      if ( ((*reg.pF & 0x20) != 0) || ((byteOperand1 & 0x0F) > 9)){
	num += 0x06;
      }
      if (((*reg.pF & 0x10) != 0) || (num > 0x9F)){
        num += 0x60;
      }
    }
    else {
      if ((*reg.pF & 0x20) != 0){
	num = (num - 0x06) & 0xFF;
      }
      if ((*reg.pF & 0x10) != 0){
	num -= 0x60;
      }
    }
    *reg.pF &= 0x5F;
    if ((num & 0x100) == 0x100){
      *reg.pF |= 0x10;
    }
    
    num &= 0xFF;
    *reg.pF |= ((num == 0) ? 0x80 : 0x00);
    
    byteResult = (byte) num;
    }    
    break;
  case RLC: // Put value shifted off into bit 0 AND carry flag
    byteResult = byteOperand1 << 1;
    byteResult |= (byteOperand1 >> 7);

    if ((opcode == 0x07) && (isCB == false)){ // RLCA has different flags
      *reg.pF = 0;
    }
    else{
      *reg.pF = ((byteResult == 0) ? 0x80 : 0x00);
    }
    *reg.pF |= (0x10 & (byteOperand1  >> 3));
    break;
  case RRC: //Put value shifted off into bit 7 AND carry flag
    byteResult = byteOperand1 >> 1;
    byteResult |= (byteOperand1 << 7);
    if ((opcode == 0x0F) && (isCB == false)){ // RRCA has different flags
      *reg.pF = 0;
    }
    else{
      *reg.pF = ((byteResult == 0) ? 0x80 : 0x00);
    }
    *reg.pF |= (0x10 & (byteOperand1 << 4));
    break;
  case RL: // shift through carry
    byteResult = byteOperand1 << 1;
    byteResult |= ((*reg.pF & 0x10) >> 4);

    if ((opcode == 0x17) && (isCB == false)){ // RLA has different flags
      *reg.pF = 0;
    }
    else{
      *reg.pF = ((byteResult == 0) ? 0x80 : 0x00);
    }
    *reg.pF |= (0x10 & (byteOperand1  >> 3));
    break;
  case RR: // shift through carry
    byteResult = byteOperand1 >> 1;
    byteResult |= ((*reg.pF & 0x10) << 3);    
    if ((opcode == 0x1F) && (isCB == false)){ //RRC generates different flags
      *reg.pF = 0;
    }
    else {
      *reg.pF = ((byteResult == 0) ? 0x80 : 0x00);
    }
    *reg.pF |= (0x10 & (byteOperand1 << 4));
    break;
  case SLA:
    byteResult = byteOperand1 << 1;

    *reg.pF = ((byteResult == 0) ? 0x80 : 0x00);
    *reg.pF |= (0x10 & (byteOperand1 >> 3));
    break;
  case SRA:
    byteResult = byteOperand1 >> 1;
    // MSB stays the same
    byteResult |= byteOperand1 & 0x80;
    *reg.pF = ((byteResult == 0) ? 0x80 : 0x00);
    *reg.pF |= (0x10 & (byteOperand1 << 4));
    break;
  case SRL:
    byteResult = byteOperand1 >> 1;
    *reg.pF = ((byteResult == 0) ? 0x80 : 0x00);
    *reg.pF |= (0x10 & (byteOperand1 << 4));
    break;
  case SWAP:
    byteResult = ((0xF0 & byteOperand1) >> 4);
    byteResult |= ((0x0F & byteOperand1) << 4);
    *reg.pF = ((byteResult == 0) ? 0x80 : 0x00);
    break;
  case BIT:
    byteResult = (byteOperand1 & (1 << byteOperand2));
    *reg.pF &= 0x10;    
    *reg.pF |= ((byteResult == 0) ? 0xA0 : 0x20);
    break;
  case SET:
    byteResult = (0x01 << byteOperand2) | byteOperand1;
    break;
  case RES:
    byteResult = (~(0x01 << byteOperand2)) & byteOperand1;
    break;
  case RST:
    mem.writeWord(reg.SP - 2, reg.PC + 1);
    reg.SP -= 2;
    reg.PC = byteOperand1;
    pcInc = 0;
    break;
  case INC:
    byteResult = byteOperand1 + byteOperand2;
    *reg.pF &= 0x1F; // erase all but carry
    *reg.pF |= ((byteResult == 0) ? 0x80 : 0x00);
    // half carry flag
    if (((((0x0F & byteOperand1) + (0x0F & byteOperand2)) & 0x10) << 1) != 0){
      *reg.pF |= 0x20;
    }
    else {
      *reg.pF &= 0xDF;
    }
    break;
  case DEC:
    // BANG - DEC doesnt have carry flag changed!!
    // BANG - half carry flag!
    byteResult = byteOperand1 - byteOperand2;
    *reg.pF &= 0x1F; //erase all but carry
    *reg.pF |= ((byteResult == 0) ? 0xC0 : 0x40);
    // deal with half carry (borrow)
    if ((byteResult & 0x0F) > (byteOperand1 & 0x0F)){
      *reg.pF |= 0x20;
    }
    else { // Set half carry flag to zero
      *reg.pF &= 0xDF;
    }
    break;	
  case INC16:    
    wordResult = wordOperand1 + wordOperand2;
    break;
  case DEC16:
    wordResult = wordOperand1 - wordOperand2;
    break;
  default: break;
  }

  /*
   *  Store result 
   */
  switch(resultType){
  case NO_RESULT: break;
  case REG8_RESULT:
    *pByteResult = byteResult;
    break;
  case REG16_RESULT:
    *(unsigned short *)pWordResult = wordResult;
    break;
  case MEM8_RESULT:
    //printf("WRITING %x to %x\n", byteResult, memWriteAddr);
    mem.write(memWriteAddr, byteResult);
    break;
  case MEM16_RESULT:
    mem.writeWord(memWriteAddr, wordResult);
    break;
  case MEM8_HLPLUS_RESULT:
    mem.write(memWriteAddr, byteResult);
    reg.HL++;
    break;
  case MEM8_HLMINUS_RESULT:
    mem.write(memWriteAddr, byteResult);
    reg.HL--;
    break;
  default: break;
  }
  // BANG - HACK to stop bits in the Flag register from being written to
  *reg.pF &= 0xF0;


  // BANG - HL+ and HL-
  if (isCB == false){
      if ((opcode == 0x2A) && (isCB == false)){
	  reg.HL++;
      }
      if ((opcode == 0x3A) && (isCB == false)){
	  reg.HL--;
      }
  }
  
  reg.PC += pcInc;

  handleTimer(numInstCycles);
  timer += numInstCycles;

  if (memWriteAddr == 0xff00){
    // BANG - JOYPAD HACK!!
    //mem.write(0xff00, 0xef);
    //checkKeypress();
  }
  checkKeypress();

  // BANG - Yet another hack
  // DMA write is special case and should take 160 microseconds
  // low priority for now
  if (memWriteAddr == 0xff46){
      byte dma = mem.read(0xff46); // get the dma base address we write back from mem
      short dma_addr = dma << 8; // shift by 8 to get the starting address of where we copy dma data
      // start copying data - dma only copies fist 0xc bytes of data from the base to OAM
      for (byte i = 0; i < 140; i++){
	  mem.write(0xfe00 + i, mem.read(dma_addr + i));
      }
  }

  memWriteAddr = 0;



    // !!!!BANG - POSSIBLE BUG HERE IF WE GET 2 DI in a row!!!!
  if (prepDI == 1){
    interruptMasterEnable = false;
    prepDI = 0;
  }
  if (prepEI == 1){
    interruptMasterEnable = true;
    prepEI = 0;
  }
}

void Processor::handleTimer(int numInstCycles){
    byte DIV = mem.read(0xFF04);
    byte TIMA = mem.read(0xFF05);
    byte TMA = mem.read(0xFF06);
    byte TAC = mem.read(0xFF07);

    // first increment the DIV register if applicable
    // increment the timer if we pass the 1/16779 of a second threshold for our current instruction
    if ((timer / DIV_INC) != ((timer + numInstCycles) / DIV_INC)){
	mem.incDIV();
    }

    // now deal with incrementing the user-set timer (TIMA)
    if ((TAC & 0x04) != 0) { // if the timer start bit is on
	byte clock_select = TAC & 0x03;
	unsigned int threshold = 0;
	switch(clock_select){
	case 0x00: 
	    threshold = 64;
	    break;
	case 0x01:
	    threshold = 1;
	    break;
	case 0x02:
	    threshold = 4;
	    break;
	case 0x03:
	    threshold = 16;
	    break;
	default: break;
	}
        
	unsigned int incRate = 16 *  threshold;
	if ((timer / incRate) != ((timer + numInstCycles) / incRate)){
	    mem.incTIMA();
	}
    }
}

bool Processor::determineBranch(byte branchType){
    switch (branchType){
    case JP_ALWAYS:
      return true;
      break;
    case JP_Z:
      if ((*reg.pF & 0x80) != 0){
	return true;
      }
      break;
    case JP_C:
      if ((*reg.pF & 0x10) != 0){
	return true;
      }
      break;
    case JP_NZ:
      if ((*reg.pF & 0x80) == 0){
	return true;
      }
      break;
    case JP_NC: 
      if ((*reg.pF & 0x10) == 0){
	return true;
      }
      break;
    }
    return false;
}
