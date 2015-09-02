#include <cstddef>


// Defines for the various instruction 'types'
#define NOP 0
#define LD 1
#define ADD 2
#define ADC 3
#define SUB 4
#define SBC 5
#define AND 6
#define OR 7
#define XOR 8
#define CP 9
#define ADD16 10
#define SUB16 11
#define LD16 12
#define PUSH 13
#define POP 14
#define CPL 15
#define CCF 16
#define SCF 17
#define HALT 18 
#define DI 19
#define EI 20
#define RLCA 21
#define RLA 22
#define RRCA 23
#define RRA 24
#define JP 25
#define JR 26
#define CALL 27
#define RST 28
#define RET 29
#define RETI 30
#define DAA 31

#define RLC 32
#define RRC 33
#define RL 34
#define RR 35
#define SLA 36
#define SRA 37
#define SRL 38
#define SWAP 39
#define BIT 40
#define RES 41
#define SET 42
#define INC 43
#define DEC 44
#define INC16 45
#define DEC16 46

#define UNDEFINED 99
// other instructions - remove and fix later
#define STOP 100
#define PREFIX 102
#define LDH 103



// defines for the different types of memory operations we can do
#define MEM_OP 1
#define REG8_OP 2
#define REG16_OP 3
#define IMM8_OP 4
#define IMM16_OP 5
#define INC_DEC_OP 6
#define IMM8_MEM_OP 7
#define IMM16_MEM_OP 8
#define IMM16_MEM16_OP 9
#define ABS16_OP 10
#define LD_C_FROM_MEM 11
#define SP_PLUS_R8_OP 12


// defines for where to store the result of our operation
#define NO_RESULT 0
#define REG8_RESULT 1
#define REG16_RESULT 2
#define MEM8_RESULT 3
#define MEM16_RESULT 4
#define MEM8_HLPLUS_RESULT 5
#define MEM8_HLMINUS_RESULT 6

// defines for the different types of interrupts
#define INTERRUPT_NONE 0x00
#define INTERRUPT_VBLANK 0x01
#define INTERRUPT_LCDSTAT 0x02
#define INTERRUPT_TIMER 0x04
#define INTERRUPT_SERIAL 0x08
#define INTERRUPT_JOYPAD 0x10

#define INTERRUPT_HDLR_VBLANK 0x40
#define INTERRUPT_HDLR_LCDSTAT 0x48
#define INTERRUPT_HDLR_TIMER 0x50
#define INTERRUPT_HDLR_SERIAL 0x58
#define INTERRUPT_HDLR_JOYPAD 0x60

#define INTERRUPT_ENABLE_REG 0xFFFF
#define INTERRUPT_FLAG_REG 0xFF0F

// defines for jump operations
#define JP_ALWAYS 0
#define JP_Z 1
#define JP_C 2
#define JP_NZ 3
#define JP_NC 4

// others
#define CLOCK_RATE 4194304
#define DIV_INC 16384 // defines how often the divider register (DIV) increments

// pixel colors
#define COLOR_BLACK 0x081820
#define COLOR_DARK_GRAY 0x88c070 
#define COLOR_LIGHT_GRAY 0x346856
#define COLOR_WHITE 0xe0f8d0
