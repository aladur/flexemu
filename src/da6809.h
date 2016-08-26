//
//  da6809.h
//

#ifndef __da6809_h__
#define __da6809_h__

#include "misc1.h"
#include "absdisas.h"


class Da6809 : public AbstractDisassembler
{

protected:

    char        code_buf[28];       // buffer for machinecode
    char        mnem_buf[28];       // buffer for mnemonic
    bool        use_undocumented;

public:
    Da6809();   // public constructor
    virtual ~Da6809();      // public destructor

    int     Disassemble(const Byte *pMemory, DWord pc, DWord *pFlags,
                        DWord *pAddr, char **pb1, char **pb2);
    void        set_use_undocumented(bool b);

private:

    inline Byte D_Page10(DWord *, Word, const Byte *pMemory, DWord *pAddr);
    inline Byte     D_Page11(DWord *, Word, const Byte *pMemory, DWord *pAddr);
    inline Byte     D_Illegal(const char *, Word, Byte, const Byte *);
    inline Byte     D_Direct(const char *, Word, Byte, const Byte *);
    inline Byte     D_Immediat(const char *, Word, Byte, const Byte *);
    inline Byte     D_ImmediatL(const char *, Word, Byte, const Byte *,
                                DWord *pAddr);
    inline Byte     D_Inherent(const char *, Word, Byte, const Byte *);
    Byte     D_Indexed(const char *, Word, Byte, const Byte *);
    inline Byte     D_Extended(const char *, Word, Byte, const Byte *,
                               DWord *pAddr);
    inline Byte     D_Relative(const char *, Word, Byte, const Byte *,
                               DWord *pAddr);
    inline Byte     D_RelativeL(const char *, Word, Byte, const Byte *,
                                DWord *pAddr);
    inline Byte     D_Register0(const char *, Word, Byte, const Byte *);
    inline Byte     D_Register1(const char *, Word, Byte, const Byte *);
    inline Byte     D_Register2(const char *, Word, Byte, const Byte *);

    const char  *IndexedRegister(Byte which);
    const char      *InterRegister(Byte which);
    const char  *StackRegister(Byte which, const char *not_stack);
    const char  *FlexLabel(Word);
};  // class Da6809

#endif // __da6809_h__
