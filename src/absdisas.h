//
//  absdisas.h
//

#ifndef ABSDISAS_INCLUDED
#define ABSDISAS_INCLUDED

#include "misc1.h"

enum
{
    DA_JUMP = 1,        // next instruction will not be processed
    DA_SUB = 2,         // jump into a subroutine
    DA_COMPUTED_GOTO = 4, // an instruction containing a computed goto
    DA_ILLEGAL = 8,     // illegal instruction
    DA_NOOP = 16,       // no operation
    DA_JUMP_ADDR = 32,  // return a jump target address
    DA_LABEL_ADDR = 64  // return a label address
};

class AbstractDisassembler
{

public:
    virtual ~AbstractDisassembler() { };
    virtual int Disassemble(
                  const Byte * const pMemory,
                  DWord pc,
                  DWord *pFlags,
                  DWord *pJumpAddr,
                  char **pCode,
                  char **pMnemonic) = 0;
    virtual void set_use_undocumented(bool value) = 0;
};  // class AbstractDisassembler

#endif // ABSDISAS_INCLUDED

