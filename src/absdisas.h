//
//  absdisas.h
//

#ifndef __absdisas_h__
#define __absdisas_h__

#include <misc1.h>

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
    virtual     ~AbstractDisassembler() { };
    int     virtual Disassemble(
        const Byte *pMemory, DWord pc, DWord *pFlags, DWord *pAddr,
        char **pb1, char **pb2) = 0;
    void        virtual set_use_undocumented(bool b) = 0;
};  // class AbstractDisassembler

#endif // __absdisas_h__

