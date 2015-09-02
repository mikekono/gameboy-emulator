#define CARRY_FLAG ((*reg.pF << 3) >> 7)


/* This is a data holding class for our registers. The 16 bit registers we need are defined, and 2 byte pointers
 * point to upper and lower halves of the appropriate 16 bit registers. 
 *
 */
class Register {
public:
  Register() {
    AF = 0x0000;
    BC = 0x0000;
    DE = 0x0000;
    HL = 0x0000; 
    SP = 0xFFFE;
    PC = 0x0000;

    pAF = &AF;
    pBC = &BC;
    pDE = &DE;
    pHL = &HL;
    pSP = &SP;
    pPC = &PC;

    // Ugly pointer stuff so that registers can be accessed in their 8 and 16 bit versions. 
    pF = (unsigned char *) pAF;
    pA = pF + 1;
    pC = (unsigned char *) pBC;
    pB = pC + 1;
    pE = (unsigned char *) pDE;
    pD = pE + 1;
    pL = (unsigned char *) pHL;
    pH = pL + 1;
  }

  unsigned short AF, BC, DE, HL, SP, PC; // The 16 bit registers

  unsigned short *pAF, *pBC, *pDE, *pHL, *pSP, *pPC; // Pointers to 16 bit registers
  unsigned char *pA, *pB, *pC, *pD, *pE, *pF, *pH, *pL; // Pointers to 8 bits of the 16 bit registers
};

