#include <unistd.h>
#include <string>
#include <iostream>
#include <fstream>
using namespace std;

#include  "Processor.cpp"

Memory mem;
Processor cpu(mem);
Graphics graphics(mem);


void loadMem(std::string rom){
    ifstream file(rom, ios::in|ios::binary|ios::ate);
    streampos size;
    char * memblock;
    size = file.tellg();
    memblock = new char [size];
    file.seekg (0, ios::beg);
    file.read ((char *) mem.cartridge, size);
    file.close();
    for (unsigned int i = 0; i < 0x8000; i++){
	mem.memory[i] = (unsigned char)mem.cartridge[i];
    }
    // if size is bigger than 32KB, then we use mem swapping
    if (size > 0x8000){
	printf("Cartridge of size: %x\n", size);
	mem.isMbc1Mode = true;
    }  
    printf("boop\n");
    cpu.reg.PC = 0x100;
}

int main(int argc, char* argv[]) {
    if (argc < 2){
	printf("Please specify the rom file.\n");
	return -1;
    }
    char* rom = argv[1];

    graphics.init();
    graphics.draw();
    loadMem(rom);

    while (true){
	if (cpu.quit){
	    break;
	}
        cpu.tick();
	graphics.step(cpu.numInstCycles);
    }
    printf("EXITED infinite loop\n");

    return 0;
}
