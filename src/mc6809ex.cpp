/*
    mc6809ex.cpp

    execution of one processor instruction

    flexemu, an MC6809 emulator running FLEX
    Copyright (C) 1997-2022  W. Schwotzer

    This file is based on usim-0.91 which is
    Copyright (C) 1994 by R. B. Bellis
*/

// this file should not be compiled separately
// it will be included


/* Select instruction */
switch (memory.read_byte(pc++))
{
case 0x01:
    if (!use_undocumented)
    {
        pc--;
        invalid("instruction");
        break;
    }
    FALLTHROUGH;

case 0x00:
    neg(fetch_ea_dir());
    cycles +=  6;
    break;

case 0x02:
    if (!use_undocumented)
    {
        pc--;
        invalid("instruction");
        break;
    }

    negcom(fetch_ea_dir());
    cycles +=  6;
    break;

case 0x03:
    com(fetch_ea_dir());
    cycles +=  6;
    break;

case 0x05:
    if (!use_undocumented)
    {
        pc--;
        invalid("instruction");
        break;
    }
    FALLTHROUGH;

case 0x04:
    lsr(fetch_ea_dir());
    cycles +=  6;
    break;

case 0x06:
    ror(fetch_ea_dir());
    cycles +=  6;
    break;

case 0x07:
    asr(fetch_ea_dir());
    cycles +=  6;
    break;

case 0x08:
    lsl(fetch_ea_dir());
    cycles +=  6;
    break;

case 0x09:
    rol(fetch_ea_dir());
    cycles +=  6;
    break;

case 0x0b:
    if (!use_undocumented)
    {
        pc--;
        invalid("instruction");
        break;
    }
    FALLTHROUGH;

case 0x0a:
    dec(fetch_ea_dir());
    cycles +=  6;
    break;

case 0x0c:
    inc(fetch_ea_dir());
    cycles +=  6;
    break;

case 0x0d:
    tst(fetch_ea_dir());
    cycles +=  6;
    break;

case 0x0e:
    jmp(fetch_ea_dir());
    cycles +=  3;
    break;

case 0x0f:
    clr(fetch_ea_dir());
    cycles +=  6;
    break;

case 0x10:
    switch (memory.read_byte(pc++))
    {
        case 0x21:
            cycles += lbrn();
            break;

        case 0x22:
            cycles += lbhi();
            break;

        case 0x23:
            cycles += lbls();
            break;

        case 0x24:
            cycles += lbcc();
            break;

        case 0x25:
            cycles += lbcs();
            break;

        case 0x26:
            cycles += lbne();
            break;

        case 0x27:
            cycles += lbeq();
            break;

        case 0x28:
            cycles += lbvc();
            break;

        case 0x29:
            cycles += lbvs();
            break;

        case 0x2a:
            cycles += lbpl();
            break;

        case 0x2b:
            cycles += lbmi();
            break;

        case 0x2c:
            cycles += lbge();
            break;

        case 0x2d:
            cycles += lblt();
            break;

        case 0x2e:
            cycles += lbgt();
            break;

        case 0x2f:
            cycles += lble();
            break;

        case 0x3f:
            swi2();
            cycles += 20;
            break;

        case 0x83:
            cmp(d, fetch_imm_16());
            cycles += 5;
            break;

        case 0x8c:
            cmp(y, fetch_imm_16());
            cycles += 5;
            break;

        case 0x8e:
            ld(y, fetch_imm_16());
            cycles += 4;
            break;

        case 0x93:
            cmp(d, fetch_dir_16());
            cycles += 7;
            break;

        case 0x9c:
            cmp(y, fetch_dir_16());
            cycles += 7;
            break;

        case 0x9e:
            ld(y, fetch_dir_16());
            cycles += 6;
            break;

        case 0x9f:
            st(y, fetch_ea_dir());
            cycles += 6;
            break;

        case 0xa3:
            cmp(d, fetch_idx_16(cycles));
            cycles += 7;
            break;

        case 0xac:
            cmp(y, fetch_idx_16(cycles));
            cycles += 7;
            break;

        case 0xae:
            ld(y, fetch_idx_16(cycles));
            cycles += 6;
            break;

        case 0xaf:
            st(y, fetch_ea_idx(cycles));
            cycles += 6;
            break;

        case 0xb3:
            cmp(d, fetch_ext_16());
            cycles += 8;
            break;

        case 0xbc:
            cmp(y, fetch_ext_16());
            cycles += 8;
            break;

        case 0xbe:
            ld(y, fetch_ext_16());
            cycles += 7;
            break;

        case 0xbf:
            st(y, fetch_ea_ext());
            cycles += 7;
            break;

        case 0xce:
            ld(s, fetch_imm_16());
            cycles += 4;
            break;

        case 0xde:
            ld(s, fetch_dir_16());
            cycles += 6;
            break;

        case 0xdf:
            st(s, fetch_ea_dir());
            cycles += 6;
            break;

        case 0xee:
            ld(s, fetch_idx_16(cycles));
            cycles += 6;
            break;

        case 0xef:
            st(s, fetch_ea_idx(cycles));
            cycles += 6;
            break;

        case 0xfe:
            ld(s, fetch_ext_16());
            cycles += 7;
            break;

        case 0xff:
            st(s, fetch_ea_ext());
            cycles += 7;
            break;

        default:
            pc -= 2;
            invalid("instruction");
            break;
    }

    break;

    //case 0x11: post11(); break;

case 0x11:
    switch (memory.read_byte(pc++))
    {
        case 0x3f:
            swi3();
            cycles += 20;
            break;

        case 0x83:
            cmp(u, fetch_imm_16());
            cycles +=  5;
            break;

        case 0x8c:
            cmp(s, fetch_imm_16());
            cycles +=  5;
            break;

        case 0x93:
            cmp(u, fetch_dir_16());
            cycles +=  7;
            break;

        case 0x9c:
            cmp(s, fetch_dir_16());
            cycles +=  7;
            break;

        case 0xa3:
            cmp(u, fetch_idx_16(cycles));
            cycles +=  7;
            break;

        case 0xac:
            cmp(s, fetch_idx_16(cycles));
            cycles +=  7;
            break;

        case 0xb3:
            cmp(u, fetch_ext_16());
            cycles +=  8;
            break;

        case 0xbc:
            cmp(s, fetch_ext_16());
            cycles +=  8;
            break;

        default:
            pc -= 2;
            invalid("instruction");
            break;
    }

    break;

case 0x12:
    nop();
    cycles +=  2;
    break;

case 0x13:
    sync();
    cycles +=  2;
    break;

case 0x16:
    lbra();
    cycles +=  5;
    break;

case 0x17:
    lbsr();
    cycles +=  9;
    break;

case 0x19:
    daa();
    cycles +=  2;
    break;

case 0x1a:
    orcc(fetch_imm_08());
    cycles +=  3;
    break;

case 0x1c:
    andcc(fetch_imm_08());
    cycles +=  3;
    break;

case 0x1d:
    sex();
    cycles +=  2;
    break;

case 0x1e:
    exg();
    cycles +=  8;
    break;

case 0x1f:
    tfr();
    cycles +=  6;
    break;

case 0x20:
    bra();
    cycles +=  3;
    break;

case 0x21:
    brn();
    cycles +=  3;
    break;

case 0x22:
    bhi();
    cycles +=  3;
    break;

case 0x23:
    bls();
    cycles +=  3;
    break;

case 0x24:
    bcc();
    cycles +=  3;
    break;

case 0x25:
    bcs();
    cycles +=  3;
    break;

case 0x26:
    bne();
    cycles +=  3;
    break;

case 0x27:
    beq();
    cycles +=  3;
    break;

case 0x28:
    bvc();
    cycles +=  3;
    break;

case 0x29:
    bvs();
    cycles +=  3;
    break;

case 0x2a:
    bpl();
    cycles +=  3;
    break;

case 0x2b:
    bmi();
    cycles +=  3;
    break;

case 0x2c:
    bge();
    cycles +=  3;
    break;

case 0x2d:
    blt();
    cycles +=  3;
    break;

case 0x2e:
    bgt();
    cycles +=  3;
    break;

case 0x2f:
    ble();
    cycles +=  3;
    break;

case 0x30:
    lea(x, fetch_ea_idx(cycles));
    cycles +=  4;
    break;

case 0x31:
    lea(y, fetch_ea_idx(cycles));
    cycles +=  4;
    break;

case 0x32:
    lea_nocc(s, fetch_ea_idx(cycles));
    cycles +=  4;
    break;

case 0x33:
    lea_nocc(u, fetch_ea_idx(cycles));
    cycles +=  4;
    break;

case 0x34:
    cycles += psh(fetch_imm_08(), s, u);
    break;

case 0x35:
    cycles += pul(fetch_imm_08(), s, u);
    break;

case 0x36:
    cycles += psh(fetch_imm_08(), u, s);
    break;

case 0x37:
    cycles += pul(fetch_imm_08(), u, s);
    break;

case 0x39:
    rts();
    cycles +=  5;
    break;

case 0x3a:
    abx();
    cycles +=  3;
    break;

case 0x3b:
    cycles += rti();
    break;

case 0x3c:
    cwai();
    cycles += 20;
    break;

case 0x3d:
    mul();
    cycles += 11;
    break;

case 0x3e:
    if (!use_undocumented)
    {
        pc--;
        invalid("instruction");
        break;
    }

    rst();
    cycles += 19;
    break;

case 0x3f:
    swi();
    cycles += 19;
    break;

case 0x41:
    if (!use_undocumented)
    {
        pc--;
        invalid("instruction");
        break;
    }
    FALLTHROUGH;

case 0x40:
    neg(a);
    cycles +=  2;
    break;

case 0x43:
    com(a);
    cycles +=  2;
    break;

case 0x45:
    if (!use_undocumented)
    {
        pc--;
        invalid("instruction");
        break;
    }
    FALLTHROUGH;

case 0x44:
    lsr(a);
    cycles +=  2;
    break;

case 0x46:
    ror(a);
    cycles +=  2;
    break;

case 0x47:
    asr(a);
    cycles +=  2;
    break;

case 0x48:
    lsl(a);
    cycles +=  2;
    break;

case 0x49:
    rol(a);
    cycles +=  2;
    break;

case 0x4b:
    if (!use_undocumented)
    {
        pc--;
        invalid("instruction");
        break;
    }
    FALLTHROUGH;

case 0x4a:
    dec(a);
    cycles +=  2;
    break;

case 0x4c:
    inc(a);
    cycles +=  2;
    break;

case 0x4d:
    tst(a);
    cycles +=  2;
    break;

case 0x4e:
    if (!use_undocumented)
    {
        pc--;
        invalid("instruction");
        break;
    }

    clr1(a);
    cycles +=  2;
    break;

case 0x4f:
    clr(a);
    cycles +=  2;
    break;

case 0x51:
    if (!use_undocumented)
    {
        pc--;
        invalid("instruction");
        break;
    }
    FALLTHROUGH;

case 0x50:
    neg(b);
    cycles +=  2;
    break;

case 0x53:
    com(b);
    cycles +=  2;
    break;

case 0x55:
    if (!use_undocumented)
    {
        pc--;
        invalid("instruction");
        break;
    }
    FALLTHROUGH;

case 0x54:
    lsr(b);
    cycles +=  2;
    break;

case 0x56:
    ror(b);
    cycles +=  2;
    break;

case 0x57:
    asr(b);
    cycles +=  2;
    break;

case 0x58:
    lsl(b);
    cycles +=  2;
    break;

case 0x59:
    rol(b);
    cycles +=  2;
    break;

case 0x5b:
    if (!use_undocumented)
    {
        pc--;
        invalid("instruction");
        break;
    }
    FALLTHROUGH;

case 0x5a:
    dec(b);
    cycles +=  2;
    break;

case 0x5c:
    inc(b);
    cycles +=  2;
    break;

case 0x5d:
    tst(b);
    cycles +=  2;
    break;

case 0x5e:
    if (!use_undocumented)
    {
        pc--;
        invalid("instruction");
        break;
    }

    clr1(b);
    cycles +=  2;
    break;

case 0x5f:
    clr(b);
    cycles +=  2;
    break;

case 0x61:
    if (!use_undocumented)
    {
        pc--;
        invalid("instruction");
        break;
    }
    FALLTHROUGH;

case 0x60:
    neg(fetch_ea_idx(cycles));
    cycles +=  6;
    break;

case 0x63:
    com(fetch_ea_idx(cycles));
    cycles +=  6;
    break;

case 0x65:
    if (!use_undocumented)
    {
        pc--;
        invalid("instruction");
        break;
    }
    FALLTHROUGH;

case 0x64:
    lsr(fetch_ea_idx(cycles));
    cycles +=  6;
    break;

case 0x66:
    ror(fetch_ea_idx(cycles));
    cycles +=  6;
    break;

case 0x67:
    asr(fetch_ea_idx(cycles));
    cycles +=  6;
    break;

case 0x68:
    lsl(fetch_ea_idx(cycles));
    cycles +=  6;
    break;

case 0x69:
    rol(fetch_ea_idx(cycles));
    cycles +=  6;
    break;

case 0x6b:
    if (!use_undocumented)
    {
        pc--;
        invalid("instruction");
        break;
    }
    FALLTHROUGH;

case 0x6a:
    dec(fetch_ea_idx(cycles));
    cycles +=  6;
    break;

case 0x6c:
    inc(fetch_ea_idx(cycles));
    cycles +=  6;
    break;

case 0x6d:
    tst(fetch_ea_idx(cycles));
    cycles +=  6;
    break;

case 0x6e:
    jmp(fetch_ea_idx(cycles));
    cycles +=  3;
    break;

case 0x6f:
    clr(fetch_ea_idx(cycles));
    cycles +=  6;
    break;

case 0x71:
    if (!use_undocumented)
    {
        pc--;
        invalid("instruction");
        break;
    }
    FALLTHROUGH;

case 0x70:
    neg(fetch_ea_ext());
    cycles +=  7;
    break;

case 0x73:
    com(fetch_ea_ext());
    cycles +=  7;
    break;

case 0x75:
    if (!use_undocumented)
    {
        pc--;
        invalid("instruction");
        break;
    }
    FALLTHROUGH;

case 0x74:
    lsr(fetch_ea_ext());
    cycles +=  7;
    break;

case 0x76:
    ror(fetch_ea_ext());
    cycles +=  7;
    break;

case 0x77:
    asr(fetch_ea_ext());
    cycles +=  7;
    break;

case 0x78:
    lsl(fetch_ea_ext());
    cycles +=  7;
    break;

case 0x79:
    rol(fetch_ea_ext());
    cycles +=  7;
    break;

case 0x7b:
    if (!use_undocumented)
    {
        pc--;
        invalid("instruction");
        break;
    }
    FALLTHROUGH;

case 0x7a:
    dec(fetch_ea_ext());
    cycles +=  7;
    break;

case 0x7c:
    inc(fetch_ea_ext());
    cycles +=  7;
    break;

case 0x7d:
    tst(fetch_ea_ext());
    cycles +=  7;
    break;

case 0x7e:
    jmp(fetch_ea_ext());
    cycles +=  4;
    break;

case 0x7f:
    clr(fetch_ea_ext());
    cycles +=  7;
    break;

case 0x80:
    sub(a, fetch_imm_08());
    cycles +=  2;
    break;

case 0x81:
    cmp(a, fetch_imm_08());
    cycles +=  2;
    break;

case 0x82:
    sbc(a, fetch_imm_08());
    cycles +=  2;
    break;

case 0x83:
    sub(d, fetch_imm_16());
    cycles +=  4;
    break;

case 0x84:
    and_(a, fetch_imm_08());
    cycles +=  2;
    break;

case 0x85:
    bit(a, fetch_imm_08());
    cycles +=  2;
    break;

case 0x86:
    ld(a, fetch_imm_08());
    cycles +=  2;
    break;

case 0x88:
    eor(a, fetch_imm_08());
    cycles +=  2;
    break;

case 0x89:
    adc(a, fetch_imm_08());
    cycles +=  2;
    break;

case 0x8a:
    or_(a, fetch_imm_08());
    cycles +=  2;
    break;

case 0x8b:
    add(a, fetch_imm_08());
    cycles +=  2;
    break;

case 0x8c:
    cmp(x, fetch_imm_16());
    cycles +=  4;
    break;

case 0x8d:
    bsr();
    cycles +=  7;
    break;

case 0x8e:
    ld(x, fetch_imm_16());
    cycles +=  3;
    break;

case 0x90:
    sub(a, fetch_dir_08());
    cycles +=  4;
    break;

case 0x91:
    cmp(a, fetch_dir_08());
    cycles +=  4;
    break;

case 0x92:
    sbc(a, fetch_dir_08());
    cycles +=  4;
    break;

case 0x93:
    sub(d, fetch_dir_16());
    cycles +=  6;
    break;

case 0x94:
    and_(a, fetch_dir_08());
    cycles +=  4;
    break;

case 0x95:
    bit(a, fetch_dir_08());
    cycles +=  4;
    break;

case 0x96:
    ld(a, fetch_dir_08());
    cycles +=  4;
    break;

case 0x97:
    st(a, fetch_ea_dir());
    cycles +=  4;
    break;

case 0x98:
    eor(a, fetch_dir_08());
    cycles +=  4;
    break;

case 0x99:
    adc(a, fetch_dir_08());
    cycles +=  4;
    break;

case 0x9a:
    or_(a, fetch_dir_08());
    cycles +=  4;
    break;

case 0x9b:
    add(a, fetch_dir_08());
    cycles +=  4;
    break;

case 0x9c:
    cmp(x, fetch_dir_16());
    cycles +=  6;
    break;

case 0x9d:
    jsr(fetch_ea_dir());
    cycles +=  7;
    break;

case 0x9e:
    ld(x, fetch_dir_16());
    cycles +=  5;
    break;

case 0x9f:
    st(x, fetch_ea_dir());
    cycles +=  5;
    break;

case 0xa0:
    sub(a, fetch_idx_08(cycles));
    cycles +=  4;
    break;

case 0xa1:
    cmp(a, fetch_idx_08(cycles));
    cycles +=  5;
    break;

case 0xa2:
    sbc(a, fetch_idx_08(cycles));
    cycles +=  4;
    break;

case 0xa3:
    sub(d, fetch_idx_16(cycles));
    cycles +=  6;
    break;

case 0xa4:
    and_(a, fetch_idx_08(cycles));
    cycles +=  4;
    break;

case 0xa5:
    bit(a, fetch_idx_08(cycles));
    cycles +=  4;
    break;

case 0xa6:
    ld(a, fetch_idx_08(cycles));
    cycles +=  4;
    break;

case 0xa7:
    st(a, fetch_ea_idx(cycles));
    cycles +=  4;
    break;

case 0xa8:
    eor(a, fetch_idx_08(cycles));
    cycles +=  4;
    break;

case 0xa9:
    adc(a, fetch_idx_08(cycles));
    cycles +=  4;
    break;

case 0xaa:
    or_(a, fetch_idx_08(cycles));
    cycles +=  4;
    break;

case 0xab:
    add(a, fetch_idx_08(cycles));
    cycles +=  4;
    break;

case 0xac:
    cmp(x, fetch_idx_16(cycles));
    cycles +=  6;
    break;

case 0xad:
    jsr(fetch_ea_idx(cycles));
    cycles +=  7;
    break;

case 0xae:
    ld(x, fetch_idx_16(cycles));
    cycles +=  5;
    break;

case 0xaf:
    st(x, fetch_ea_idx(cycles));
    cycles +=  5;
    break;

case 0xb0:
    sub(a, fetch_ext_08());
    cycles +=  5;
    break;

case 0xb1:
    cmp(a, fetch_ext_08());
    cycles +=  5;
    break;

case 0xb2:
    sbc(a, fetch_ext_08());
    cycles +=  5;
    break;

case 0xb3:
    sub(d, fetch_ext_16());
    cycles +=  7;
    break;

case 0xb4:
    and_(a, fetch_ext_08());
    cycles +=  5;
    break;

case 0xb5:
    bit(a, fetch_ext_08());
    cycles +=  5;
    break;

case 0xb6:
    ld(a, fetch_ext_08());
    cycles +=  5;
    break;

case 0xb7:
    st(a, fetch_ea_ext());
    cycles +=  5;
    break;

case 0xb8:
    eor(a, fetch_ext_08());
    cycles +=  5;
    break;

case 0xb9:
    adc(a, fetch_ext_08());
    cycles +=  5;
    break;

case 0xba:
    or_(a, fetch_ext_08());
    cycles +=  5;
    break;

case 0xbb:
    add(a, fetch_ext_08());
    cycles +=  5;
    break;

case 0xbc:
    cmp(x, fetch_ext_16());
    cycles +=  7;
    break;

case 0xbd:
    jsr(fetch_ea_ext());
    cycles +=  8;
    break;

case 0xbe:
    ld(x, fetch_ext_16());
    cycles +=  6;
    break;

case 0xbf:
    st(x, fetch_ea_ext());
    cycles +=  6;
    break;

case 0xc0:
    sub(b, fetch_imm_08());
    cycles +=  2;
    break;

case 0xc1:
    cmp(b, fetch_imm_08());
    cycles +=  2;
    break;

case 0xc2:
    sbc(b, fetch_imm_08());
    cycles +=  2;
    break;

case 0xc3:
    add(d, fetch_imm_16());
    cycles +=  4;
    break;

case 0xc4:
    and_(b, fetch_imm_08());
    cycles +=  2;
    break;

case 0xc5:
    bit(b, fetch_imm_08());
    cycles +=  2;
    break;

case 0xc6:
    ld(b, fetch_imm_08());
    cycles +=  2;
    break;

case 0xc8:
    eor(b, fetch_imm_08());
    cycles +=  2;
    break;

case 0xc9:
    adc(b, fetch_imm_08());
    cycles +=  2;
    break;

case 0xca:
    or_(b, fetch_imm_08());
    cycles +=  2;
    break;

case 0xcb:
    add(b, fetch_imm_08());
    cycles +=  2;
    break;

case 0xcc:
    ld(d, fetch_imm_16());
    cycles +=  3;
    break;

case 0xce:
    ld(u, fetch_imm_16());
    cycles +=  3;
    break;

case 0xd0:
    sub(b, fetch_dir_08());
    cycles +=  4;
    break;

case 0xd1:
    cmp(b, fetch_dir_08());
    cycles +=  4;
    break;

case 0xd2:
    sbc(b, fetch_dir_08());
    cycles +=  4;
    break;

case 0xd3:
    add(d, fetch_dir_16());
    cycles +=  6;
    break;

case 0xd4:
    and_(b, fetch_dir_08());
    cycles +=  4;
    break;

case 0xd5:
    bit(b, fetch_dir_08());
    cycles +=  4;
    break;

case 0xd6:
    ld(b, fetch_dir_08());
    cycles +=  4;
    break;

case 0xd7:
    st(b, fetch_ea_dir());
    cycles +=  4;
    break;

case 0xd8:
    eor(b, fetch_dir_08());
    cycles +=  4;
    break;

case 0xd9:
    adc(b, fetch_dir_08());
    cycles +=  4;
    break;

case 0xda:
    or_(b, fetch_dir_08());
    cycles +=  4;
    break;

case 0xdb:
    add(b, fetch_dir_08());
    cycles +=  4;
    break;

case 0xdc:
    ld(d, fetch_dir_16());
    cycles +=  5;
    break;

case 0xdd:
    st(d, fetch_ea_dir());
    cycles +=  5;
    break;

case 0xde:
    ld(u, fetch_dir_16());
    cycles +=  5;
    break;

case 0xdf:
    st(u, fetch_ea_dir());
    cycles +=  5;
    break;

case 0xe0:
    sub(b, fetch_idx_08(cycles));
    cycles +=  4;
    break;

case 0xe1:
    cmp(b, fetch_idx_08(cycles));
    cycles +=  4;
    break;

case 0xe2:
    sbc(b, fetch_idx_08(cycles));
    cycles +=  4;
    break;

case 0xe3:
    add(d, fetch_idx_16(cycles));
    cycles +=  6;
    break;

case 0xe4:
    and_(b, fetch_idx_08(cycles));
    cycles +=  4;
    break;

case 0xe5:
    bit(b, fetch_idx_08(cycles));
    cycles +=  4;
    break;

case 0xe6:
    ld(b, fetch_idx_08(cycles));
    cycles +=  4;
    break;

case 0xe7:
    st(b, fetch_ea_idx(cycles));
    cycles +=  4;
    break;

case 0xe8:
    eor(b, fetch_idx_08(cycles));
    cycles +=  4;
    break;

case 0xe9:
    adc(b, fetch_idx_08(cycles));
    cycles +=  4;
    break;

case 0xea:
    or_(b, fetch_idx_08(cycles));
    cycles +=  4;
    break;

case 0xeb:
    add(b, fetch_idx_08(cycles));
    cycles +=  4;
    break;

case 0xec:
    ld(d, fetch_idx_16(cycles));
    cycles +=  5;
    break;

case 0xed:
    st(d, fetch_ea_idx(cycles));
    cycles +=  5;
    break;

case 0xee:
    ld(u, fetch_idx_16(cycles));
    cycles +=  5;
    break;

case 0xef:
    st(u, fetch_ea_idx(cycles));
    cycles +=  5;
    break;

case 0xf0:
    sub(b, fetch_ext_08());
    cycles +=  5;
    break;

case 0xf1:
    cmp(b, fetch_ext_08());
    cycles +=  5;
    break;

case 0xf2:
    sbc(b, fetch_ext_08());
    cycles +=  5;
    break;

case 0xf3:
    add(d, fetch_ext_16());
    cycles +=  7;
    break;

case 0xf4:
    and_(b, fetch_ext_08());
    cycles +=  5;
    break;

case 0xf5:
    bit(b, fetch_ext_08());
    cycles +=  5;
    break;

case 0xf6:
    ld(b, fetch_ext_08());
    cycles +=  5;
    break;

case 0xf7:
    st(b, fetch_ea_ext());
    cycles +=  5;
    break;

case 0xf8:
    eor(b, fetch_ext_08());
    cycles +=  5;
    break;

case 0xf9:
    adc(b, fetch_ext_08());
    cycles +=  5;
    break;

case 0xfa:
    or_(b, fetch_ext_08());
    cycles +=  5;
    break;

case 0xfb:
    add(b, fetch_ext_08());
    cycles +=  5;
    break;

case 0xfc:
    ld(d, fetch_ext_16());
    cycles +=  6;
    break;

case 0xfd:
    st(d, fetch_ea_ext());
    cycles +=  6;
    break;

case 0xfe:
    ld(u, fetch_ext_16());
    cycles +=  6;
    break;

case 0xff:
    st(u, fetch_ea_ext());
    cycles +=  6;
    break;

default:
    pc--;
    invalid("instruction");
}

