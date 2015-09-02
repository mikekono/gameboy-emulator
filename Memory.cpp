#include "Memory.h"

void Memory::initMem(){ // Set up the initial memory on boot
    // set each value to zero first
    for (int i = 0; i < MEM_SIZE; i++){
	memory[i] = 0;
    }
}
byte Memory::read(const unsigned short address){
    if (isMbc1Mode){ // handle rom/ram switching
	if ((address < 0x8000) && (address >= 0x4000)){
	    return cartridge[(mbc1RomOff * mbc1RomBank) + (address & 0x3fff)];
	}
	else if ((address < 0xC000) && (address >= 0xA000)){
	    return ramBank[mbc1RamBank][address & 0x1fff];
	}
    }
    return memory[address];
}
word Memory::readWord(const unsigned short address){
    word wordValue = (this->read(address + 1) << 8) + this->read(address);
    return wordValue;
}


void Memory::write(const unsigned short address, const unsigned char value){
    // ensure we don't write to the ROM
    if (address >= 0x8000){
	// handle the memory that is copied
	if ((address >= 0xE000) && (address <0xFE00)){
	    memory[address - 0x2000] = value;
	}
	if ((address >= 0xC000) && (address <0xDE00)){
	    memory[address + 0x2000] = value;
	}    
	// write to one of the 4 possible ram banks for mbc1
	if (((address < 0xC000) && (address >= 0xA000)) && isMbc1Mode){
	    ramBank[mbc1RamBank][address & 0x1fff] = value;
	}

	// handle writing to the joypad register
	if (address == 0xFF00){
	    byte selection = value & 0x30; // get rid of any unhandled input
	    memory[address] = selection | 0xcf; // select P14/P15 and reset specific pressed keys
	}
	else{
	    memory[address] = value;
	}
    }
    else {
	// writing to sections in the ROM switches ROM/RAM banks for sme cartridges
	if (isMbc1Mode){
	    if (address < 0x2000){
		mbc1RamEnable = (((value & 0x0A) == 0x0A) ? true : false);
	    }
	    else if (address < 0x4000){ // write lower 5 bits of ROM bank
		mbc1RomBank &= 0xE0;
		mbc1RomBank |= (value & 0x1F);
		if (mbc1RomBank == 0){
		    mbc1RomBank = 1; // gameboy treats 0 as 1
		}
	    }
	    else if (address < 0x6000){
		if (romRamMode == false){ // ROM mode
		    mbc1RomBank &= 0x1F;
		    mbc1RomBank |= ((value & 0x03) << 5);
		    if (mbc1RomBank == 0){
			mbc1RomBank = 1; // gameboy treats 0 as 1
		    }
		}
		else { // RAM mode - put in ujp
		    mbc1RamBank = (value & 0x03);
		}
	    }
	    else if (address < 0x8000){
		if ((value == 0) && (romRamMode == true)) { // switch from RAM banking mode to ROM
		    mbc1RomBank &= 0x1F;
		    mbc1RomBank |= ((mbc1RamBank & 0x03) << 5);	// set rom bank using the previously ram value 
		    mbc1RamBank = 0; // reset ram bank to 0
		    romRamMode = false; // set mode to rom mode
		}
		else if ((value == 1) && (romRamMode == false)) { // switch from ROM banking mode to RAM
		    mbc1RamBank = (mbc1RomBank & 0x60) >> 5; // set the ram bank using bits 5 and 6 from ram bank
		    mbc1RomBank &= 0x1F; // clear bits 5 and 6 from rom bank (7 is unused always)
		    if (mbc1RomBank == 0){
			mbc1RomBank = 1; // gameboy treats 0 as 1
		    }
		    romRamMode = true; // set mode to ram mode
		}
	    }
	}
    }

}
void Memory::writeWord(const unsigned short address, const unsigned short value){
    unsigned char value_high = (value >> 8);
    unsigned char value_low = value & 0x00FF;
    write(address + 1, value_high);
    write(address, value_low);
}

void Memory::writeKeyPress(byte keyBitmask){
    memory[0xff00] = (memory[0xff00] & 0xf0) | keyBitmask;
}
    
// increments the divider register (occurs from a timer)
void Memory::incDIV(){
    memory[0xFF04] += 1;
}
  
// increments the timder counter
void Memory::incTIMA(){
    memory[0xFF05] += 1;
    // handle the overflow if we go back to 0
    if (memory[0xFF05] == 0){
	memory[0xFF05] = memory[0xFF06]; // load with TMA
	// set the timer interrupt
	memory[INTERRUPT_FLAG_REG] |= INTERRUPT_TIMER;
    }
}  




