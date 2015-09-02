#define MEM_SIZE 0x10000

class Memory {
public:
    byte memory[MEM_SIZE];
    byte cartridge[0x200000]; // used when switching cartidge ram in and out of memory - 2MB max
    byte ramBank[4][0x2000];
    bool isMbc1Mode;
    bool mbc1RamEnable; //0x0000 to 0x1fff
    byte mbc1RomBank; // 0x2000 to 0x3fff for lower 5 bits, ox4000 to 0x5fff for upper 2 bits
    byte mbc1RamBank; // 0x4000 to 0x5fff - 2 bits
    bool romRamMode; //0x6000 to 0x7fff - 0 is ROM, 1 is RAM

    unsigned int mbc1RomOff;
    unsigned int mbc1RamOff;    

    Memory(){
	memory[MEM_SIZE];
	cartridge[0x200000]; // BANG - get accurate size
	ramBank[4][0x2000];
	initMem();

	isMbc1Mode = false;
	mbc1RamEnable = false;
	mbc1RomBank = 1;
	mbc1RamBank = 0;
	romRamMode = false;

	mbc1RomOff = 0x4000;
    }

    void initMem();
    
    byte read(const unsigned short address);
    word readWord(const unsigned short address);

    void write(const unsigned short address, const unsigned char value);
    void writeWord(const unsigned short address, const unsigned short value);
    void writeKeyPress(byte keyBitmask);

    void incDIV();
    void incTIMA();

};
