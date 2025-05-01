; TTL "NEUMON VERS. 5.4 SPECIAL"
;
;
;
; monitor-program for Eurocom- microcomputer with
; MC 6809-CPU for floppydisk, with Boot for
; singledensity and RAM extension
;
; History:
; ========
;
;
; 20.11.84 W. Schwotzer		last changes on real platform
; 27.06.96 W. Schwotzer		adapted to as6809, a unix cross assembler for
;				microcomputers
; 23.02.97 W. Schwotzer		added commands INSLIN and DELLIN for better
;				editor support
; 24.05.97 W. Schwotzer		support for serial I/O after boot (INITSP)
; 31.08.98 W. Schwotzer		support for insert mode cursor with CTRL-F
;				and CTRL-G
; 09.02.2004 W. Schwotzer       After Reset clear up to 2 x 288K RAM extension
; 16.09.2018 W. Schwotzer       Make it assemble with:
;                               A09 V1.37 (https://github.com/Arakula/A09)
;                               asm6809 V2.11 (http://www.6809.org.uk/asm6809/)
; 17.09.2018 W. Schwotzer       Fully translated to english
; 16.04.2025 W. Schwotzer       Support input of NUL key (hex 00)
; 01.05.2025 W. Schwotzer       Read FLEX boot sector:
;                               Abort on any of these errors:
;                               not ready, record not found, crc error,
;                               lost data

; SYM 6
; OPT -G,P,M,E,-C,

; Command line for asm6809:
;    asm6809 --setdp=0 -H -l neumon54.lst -o neumon54.hex neumon54.asm
; Command line for a09:
;    a09 -Xneumon54.hex -Lneumon54.lst neumon54.asm
;


; PAG
VERS   EQU  4      ;for ELTEC compatibility

ANFANG EQU  $F000
DIRPAG EQU  $EF00

DRCTPG EQU  DIRPAG/$100
;       SETDP  DRCTPG

; Zero-Page Locations
       ORG  DIRPAG

DUMMY  RMB  1          ;fill byte
INSRTF RMB  1          ;insert mode flag
ESCFLG RMB  1          ;escape sequence flag
DBASE  RMB  2          ;display base address pointer
XADDR  RMB  2          ;x-coordinate of cursor
YADDR  RMB  2          ;y-coordinate of cursor
EOLNX  RMB  2          ;end of line pointer
SCROFS RMB  1          ;screen offset from display
BOTTOM RMB  2          ;bottom line pointer
INVFLG RMB  1          ;invert flag
NEWST1 RMB  1          ;second entry flag
NEWST2 RMB  1          ;new stack active
SAVEST RMB  2          ;save user stack
GYADDR RMB  2          ;dot flag
GXADDR RMB  2          ;save for x-coordinate
PGEFLG RMB  1          ;page flag 511/256 lines
COLTAB RMB  6*4+1      ;color table FF ends the
       RMB  1          ;dummy
NRLINS RMB  1          ;number of lines per char
EMPTY1 RMB  1          ;empty lines
EMPTY2 RMB  1
EMPTY3 RMB  1
LINES  RMB  1          ;lines of char's per page
NRCHR  RMB  1          ;no of char's per line
SCRLIN RMB  2          ;used lines per screen
MULINS RMB  1          ;lines per chr. without e??
       RMB  1          ;dummy
CHROFS RMB  1          ;chara. offset
INTPAG RMB  1          ;init on page
CTLTA  RMB  2          ;control character table
CHATAB RMB  2          ;character table
SCREN1 RMB  2          ;RAM bank for screen
SCREN2 RMB  2          ;second page
ESCVC  RMB  2          ;escape vector
ESCTBL RMB  2          ;escape table
SOFT   RMB  2          ;softscroll
XBOTOM RMB  2          ;bottom pointer
BUMPV  RMB  2          ;bump vector
;
;       OPT  NOL
ORIG   SET  *
       IF   ORIG>=DIRPAG+$58
       ERR  "Direct page before $58 too long!"
       ENDIF
;       OPT  LIS

       ORG  DIRPAG+$58

HIGHLI RMB  1         ;highlight flag
DEUTSC RMB  1         ;FlAG for characterset
NEWCHR RMB  1         ;last input from parallel keyboard
CURSOR RMB  1         ;Cursor visible / unvisible
ISNEWC RMB  1         ;FLAG for new input in NEWCHR
COMMAN RMB  18        ;command at interrupt
BRPEND EQU  *

       ORG  DIRPAG+$70

SW3VEC RMB  2          ;---
SW2VEC RMB  2
FIRQVC RMB  2          ;Interrupt-
IRQVEC RMB  2          ;vectors
SWIVEC RMB  2
NMIVEC RMB  2          ;---
SAVUST RMB  2          ;user Stack
MEMADR RMB  2          ;Memory-Address
TEMPX  RMB  2          ;temporary
TEMPY  RMB  2          ;memory
TEMPA  RMB  1          ;
LODFLG RMB  1          ;loadingflag
SERPAR RMB  1          ;seriell/parallel flag
LOCCNT RMB  1          ;memorycell counter
BYTANZ RMB  1          ;bytecount
CHECKS RMB  1          ;checksumcounter
TEMP   RMB  1
UL     RMB  1          ;upper/lower case flag

       ORG  DIRPAG+$DE

SAVE   RMB  2          ;pointer for MMU STACK
MMURAM RMB  16         ;RAM for MMU contents

; HARDWARE ADRESSEN

MMU    EQU  $FFE0      ;Memory Management Unit
;
PIA1AD EQU  $FCF0      ;Pia A data- and
PIA1AC EQU  $FCF1      ;controlregister
PIA1BD EQU  $FCF2      ;Pia B Data- and
PIA1BC EQU  $FCF3      ;controlregister
PIA2BD EQU  $FCFA      ;Pia 2B Dataregister

ACIACO EQU  $FCF4      ;ACIA control- und
ACIADA EQU  $FCF5      ;dataregister

VIDPAG EQU  $FCF6      ;videopageregister
BILANF EQU  $FCF7      ;scrollregister

;
; floppycontroler registers
;
FLCPAG EQU  $FD00      ;direct page for fdc
FLCOMM EQU  $FD30      ;commandregister
FLSEKT EQU  $FD32      ;sectorregister
FLDATA EQU  $FD33      ;dataregister
FLDRIV EQU  $FD38      ;driveselectregister
;
; emulator command register
;
EMUCMD EQU  $FD3C

; System equates for FLEX 9.1
;
WARMS  EQU  $CD03      ;warmstart entry point
DOCMND EQU  $CD4B      ;call DOS as a subroutine
LINBUF EQU  $C080      ;line buffer
BUFPNT EQU  $CC14      ;line buffer pointer

CLS    EQU  $0C        ;form feed
ESC    EQU  $1B        ;Escape
ENDE   EQU  $1A
BLANK  EQU  $20

ZCASEU EQU  $C0
ZUNUBI EQU  $F03F
UNUBI2 EQU  $FC0F
STACK  EQU  $EF00
STACK2 EQU  STACK-$C0
PIXLIN EQU  84*6       ;Pixels/Line
SCRN1  EQU  $0C0C      ;Video page
SCRN2  EQU  $0808      ;Video page if double res.
BOTOM1 EQU  $00FE
LPCNT  EQU  17
USERST EQU  STACK-$120


       ORG  ANFANG
;
; Einsprung Tabelle
;
ZRESTA JMP  BEGINN    ;Enter from restart
ZFROMT JMP  FROMTO    ;Input start- end-adr.
       ORCC #%1000     ;convert HEX to BCD
       RTS
ZCHEXL JMP  CHEXL     ;Conv. MS-BCD to HEX
ZCHEXR JMP  CHEXR     ;Conv. LS-BCD to HEX
ZIN4HX JMP  IN4HX0    ;Input address
ZUINCH JMP  INCH      ;Input 1 character
ZINCHA JMP  INCHA     ;Input 1 char. without echo
ZOUTC2 JMP  OUTCH     ;Output 1 character
ZOUT2H JMP  OUT2HX    ;Print 2 HEX-Char.+SP
ZOUT4H JMP  OUT4HX    ;Print 4 HEX-Char.+SP
ZPCRLF JMP  PCRLF     ;Print <CR>, <LF>
ZPDATA JMP  PDATA     ;Print <CR>, <LF> + String
ZPDAT1 JMP  PDATA1    ;Print String
ZSPACE JMP  PSPACE    ;Print Space
TO_HKS JMP  HKS       ;Warmstart
ZLOAD0 JMP  LOAD0     ;Input from ACIA
ZRECD0 JMP  OUTCHS    ;Output on ACIA
ZSWTCH FDB  SWITCH    ;Switch MMU
ZSWBAK FDB  SWTCHB    ;Switch back MMU
ZCONVT FDB  CNVERT    ;Convert upper/lower case
ZUSRCH FDB  SEARCH    ;Table search
ZUCOMP FDB  COMPAR    ;String compare
ZXDCHR FDB  XDCHAR    ;Draw character
ZUXINV FDB  INVCUR    ;Invert cursor
ZUSTAT FDB  STATUS    ;Keyboard Status (FLEX compatible)
;
;
; command table in main control loop (HKS)
;
HKSTAB EQU  *
       FCC  "D"        ;German char.set
       FDB  GERMAN
       FCC  "I"        ;ASCII char.set
       FDB  ASCII
       FCC  "G"        ;Start Userpgm
       FDB  GO
       FCC  "K"        ;Book Diskette
       FDB  BOOT
       FCC  "M"        ;Memory
       FDB  MEMORY
       FCC  "P"        ;Put value
       FDB  PUT
       FCC  "T"        ;Table
       FDB  TABLE
       FCC  "V"        ;Video-Terminal
       FDB  VIDEO
       FCC  "X"        ;Flex Warmstart
       FDB  CFLEX
       FCC  "Z"        ;Exit emulator
       FDB  EMUEXIT
;
MEMTAB FCC  "/"        ;Slash
       FDB  SLASH
       FCC  "."        ;Point
       FDB  POINT
       FCC  "^"        ;Uparrow
       FDB  UPAROW
       FCB  $0A        ;line-feed
       FDB  LINFED
       FCB  $0D        ;carriage-return
       FDB  HKS
ENDHT  EQU  *         ;End of HKS-Table
;
; outputtexts
;
       ORG ANFANG+$7C
INITSP FCB  0		; initial value for SERPAR
MESEUR EQU  *
THALLO EQU  *
       FCC  "EUROCOM MONITOR "
LVN    EQU  *
;
;
;       OPT  NOL
       IF   LVN>ANFANG+$8D
       ERR  "Version number too long!"
       ENDIF
;       OPT  LIS
;
       FCC  "V5."
       FCB  VERS+'0'
       FCC  " spec."
       FCB  $0D,$0A
       FCB  $04
;
TFROM  FCC  "FROM: "
       FCB  4
TTO    FCC  " TO: "
       FCB  4
WERTMS FCC  " Value: "
       FCB  4
HKSTXT FCC  ">"
       FCB  4
ZF0A5  FCC  "BRP"
       FCB  4
TREGIS FCC  "CC="
       FCB  4
       FCC  "A="
       FCB  4
       FCC  "B="
       FCB  4
       FCC  "DP="
       FCB  4
       FCC  "X="
       FCB  4
       FCC  "Y="
       FCB  4
       FCC  "U="
       FCB  4
       FCC  "PC="
       FCB  4
       FCC  "S="
       FCB  4
;
DEFCOM FCC  "HARDCOPY"
       FCB  $0D
;
;
; NMI-Einsprung
;
NMI1   STS  SAVUST    ;save stack
       LDS  #STACK    ;use own stack
       JSR  INCHA     ; get a character
       JSR  LOWUP     ; convert to uppercase
       CMPA #'H'      ; is it HARDCOPY ?
       BNE  NORMAL

       LDX  #LINBUF
       STX  BUFPNT
       LDY  #COMMAN
DOLOOP LDA  ,Y+
       STA  ,X+
       CMPA #$0D
       BNE  DOLOOP
       JSR  DOCMND
       LDS  SAVUST
       ANDCC #%10101111		; CLI
       JMP  WARMS

NORMAL CMPA #'B'        ;user wants to break?
       BEQ  NORM1

       LDS  SAVUST     ;get userstack back

       RTI             ;finish interrupt

NORM1  JSR  INITI

       LDA  INITSP
       STA  SERPAR
       CLR  LODFLG
       JSR  PCRLF
       LBRA IRQEN1     ;output all registers
;
; RESTART entry point
;
BEGINN LDS  #STACK
       LDU  #MMU+16
       LDD  #$0303
       PSHU B         ;EPROM at $F000
       PSHU A         ;EURO-RAM   at $E000
       LDD  #$0303
       PSHU D         ;EURO-RAM   at $C000-$DFFF
       CLRA
       CLRB
       TFR  D,X
       LDU  #$F000
CLRSC1 PSHU X,D       ;from $F000
       CMPU #$C000    ;to $C000 clear memory
       BNE  CLRSC1
       LDU  #MMU+16
       LDX  #CLRTAB
       BSR  CLRSCR
       BSR  CLRSCR
       PSHS U,X
       JSR  INITI
       LDA  INITSP
       STA  SERPAR
       JSR  INTSCR    ;from this point RAM extension can be used
       PULS U,X
       BSR  CLRSCR
       BSR  CLRSCR
       LDD  #SAVE
       STD  SAVE
       LDU  #MMU+16
       LDY  #MMURAM+16
       LDX  #$0303
       LDD  #$0303    ;EUROC. $C000-$EFFF, and then ROM
       BSR  PUSHU1
       LDX  #$0707    ;EUROC.-Ram from 8000-BFFF
       BSR  PUSHU
       LDX  #$0B0B    ;EUROC.-Ram from 4000-8000
       BSR  PUSHU
       LDX  #$0F0F    ;EUROC.-Ram from 0000-4000
       BSR  PUSHU
;
; now total RAM extension initialized
;
       LDX  #USERST-12
       STX  SAVUST-DIRPAG
       LDX  #NMI1
       STX  NMIVEC-DIRPAG
       LDX  #THALLO
       JSR  PDATA1
       LDX  #IRQENT
       STX  IRQVEC-DIRPAG
       LDX  #SWIENT
       STX  SWIVEC-DIRPAG
       LDX  #COMMAN
       LDY  #DEFCOM
COLOOP LDA  ,Y+       ;Default command
       STA  ,X+       ;copy to direct page
       CMPA #$0D      ;must with CR
       BNE  COLOOP
       LBRA BOOT      ;now try to boot
;
; Clear 3 16K-Blocks from CLRTAB
;
CLRSCR LDB   #3       ;loop over 3 pages
CLRP1  LDA  ,X+
       BSR   CLRPAG
       DECB
       BNE   CLRP1
       RTS
;
CLRPAG PSHS  B,X,U
       TFR   A,B
       LEAU  -4,U
       TFR   D,X
       PSHU  D,X
       PSHU  D,X
       PSHU  D,X
       CLRA
       LDX   #0
       LDU   #$C000
       JSR   CLEARS     ;Clear 1 page from $0-$BFFF
       PULS  B,X,U,PC
       FCB   $FF,$FF    ;Stuffing bytes

PUSHU  TFR  X,D
PUSHU1 PSHU X,D
       STX  ,--Y
       STD  ,--Y
       RTS

INITI  LDA  #DRCTPG
       TFR  A,DP       ;set direct page
       LDX  #PIA1AD    ;---
       CLR  $01,X
       CLR  $03,X
       CLR  $0B,X
       CLR  ,X         ;PIA-Initialization
       LDA  #$5E
       STA  $A,X
       LDA  #$3E
       STA  $01,X
       STA  $02,X
       STA  $0B,X
       STA  $03,X      ;---
       LDA  #$03
       STA  $0A,X      ;In PIA2, no Bell, MMU
       STA  $04,X      ;Master Reset ACIA
       LDA  #$15
       STA  $04,X      ;Initialization
       RTS
;
;
;
CLRTAB FCB  0,1,2,4,5,6,8,9,$A,$C,$D,$E
;
;
ERRHKS LDA  #'?'
       JSR  OUTA       ;output question mark
       LDA  #$07
       JSR  OUTA       ;output bell
HKS    LDS  #STACK     ;load stackpointer
       LDA  #DRCTPG
       TFR  A,DP
       CLR  LODFLG-DIRPAG   ;clear loadflag
       LDX  #HKSTXT
       JSR  PDATA      ; output input prompt ">"
       JSR  INCH       ; get character from keyboard
       JSR  LOWUP      ; convert to uppercase
       LDU  #HKSTAB    ; pointer to input table
       JSR  GETTAS     ; is character in input table ?
       BCS  ERRHKS     ; no, error
       JSR  ZSPACE     ; yes output space
;
HKS1   JMP  [$01,U]    ; execute command
;
;
; MEMORY: output and change contents of memory 
;
MEMORY JSR  IN4HX      ;Input 4-digit hex-address
       STX  MEMADR-DIRPAG    ;Save it. Is it correct?
       BCS  ERRHKS     ;no, input-error
       JSR  PSPACE     ;output a space
;
MEMOR1 JSR  OUT2HX     ;Output content of address
       LEAX -$01,X     ;Restore address
;
MEMOR2 JSR  INCH       ;Get input character
       JSR  LOWUP      ;To upper case
       LDU  #MEMTAB    ;Start of memory command table
       JSR  GETTAS     ;Find character in table?
       BCC  HKS1       ;yes, execute command
       JSR  INHEX1     ;no, input 2-hex digits
       JSR  BYTE0      ;get byte value, is it valid?
ERRO6  BCS  ERRHKS     ;no, input error
       STA  ,X         ;yes, store byte value
       CMPA ,X         ;byte value written correctly?
       BNE  ERRHKS     ;no, erro
       BRA  MEMOR2     ;loop to get next input
;
; SLASH: Display value of next address
;
SLASH  BSR  INCADR     ;increment address
       BRA  MEMOR1     ;loop to output addr. content
;
; LINE-FEED: Display next address and value in new line
;
LINFED BSR  INCADR     ;increment address
       LDA  #$0D       ;output new line
       JSR  OUTA
;
LINFE0 LDX  #MEMADR
       JSR  OUT4HX     ;output new address
       LDX  MEMADR-DIRPAG ;load new address in X
       BRA  MEMOR1     ;loop to output addr. content
;
; POINT: Display value of same address
;
POINT  LDX  MEMADR-DIRPAG ;load same address in X
       BRA  MEMOR1     ;loop to output addr. content
;
; UPARROW: Display previous address and value in new line
;
UPAROW LDX  MEMADR-DIRPAG    ;load address in X
       LEAX -$01,X     ;get previous address in X
       STX  MEMADR-DIRPAG    ;save address
       JSR  PCRLF      ;output new line
       BRA  LINFE0     ;output address and value
;
; Function to increment the memory address pointer
;
INCADR LDX  MEMADR-DIRPAG    ;load memory address into X
       LEAX $01,X      ;increment address
       STX  MEMADR-DIRPAG    ;store new address
       RTS
;
; TABLE: Output a memory dump in hex and ASCII
;
TABLE  JSR  FROMTO     ;Get address range
       BCS  ERRO6
       STD  TEMPX-DIRPAG     ;store start address
       LEAX $01,X      ;increment end address
       STX  TEMPY-DIRPAG     ;store it
TABLE0 JSR  PCRLF      ;new line
       LDX  #TEMPX     ;get start address
       JSR  OUT4HX     ;output it
       LDB  #16        ;16 values per line
       LDX  TEMPX      ;start address in X
       PSHS X          ;push it on stack
TABLE1 CMPX TEMPY      ;reached end address?
       BEQ  END_TA     ;yes, jump to output ASCII dump
       PSHS B          ;push value counter
       JSR  OUT2HX     ;output value of address
       PULS B          ;restore value counter
       STX  TEMPX-DIRPAG     ;store current address
       BSR  HALTAN     ;maybe break output
       DECB            ;have 16 values been dumped?
       BNE  TABLE1     ;no, continue in loop
ASDUMP PULS X          ;Restore address
       LDB  #17        ;16+1
       JSR  PSPACE     ;output space
NEXTT  CMPX TEMPY      ;Reached end address?
       LBEQ HKS        ;yes, jump back to hks
       BSR  HALTAN     ;maybe break output
       DECB            ;all values printed?
       BEQ  TABLE0     ;yes, next line
       LDA  ,X+        ;get value from memory
       ANDA #$7F       ;mask for 1-bit ASCII
       CMPA #$20
       BHS  ISAS
       LDA  #$5F       ;replace non-printable char. by _
ISAS   JSR  OUTCH      ;Print value
       BRA  NEXTT      ;loop for next ASCII value
;
END_TA LDA  #3
       MUL
ENDT1  JSR  PSPACE     ;Output 3 spaces
       DECB            ;for each not printed value
       BNE  ENDT1
       BRA  ASDUMP     ;jump to output ASCII
;
HALTAN PSHS A,B,X,Y,U,DP,CC
       JSR  TSTIN      ;keyboard input?
       BCC  NOHALT     ;no, jump back
       JSR  INCHA      ;Wait for input character
       CMPA #$0D       ;Is it a carrige return?
       LBEQ HKS        ;yes abort, go back to hks
NOHALT PULS D,X,Y,U,DP,CC,PC
;
; PUT: Fill memory range with value
;
PUT    JSR  FROMTO     ;Get address range
       LBCS ERRHKS
       STD  TEMPX      ;store start address
       LEAX 1,X        ;increment end address
       STX  TEMPY      ;store it
       LDX  #WERTMS
       JSR  PDATA1
       JSR  BYTE       ;get value
       LDX  TEMPX      ;get start address in X
       STA  ,X         ;store value into address
PUTLUP CMPX TEMPY      ;reaced end address?
       LBEQ HKS        ;yes, go back to hks
       STA  ,X+        ;increment address
       BRA  PUTLUP     ;loop to store value to next addr.
;
; VIDEO: Enter video terminal mode
;        input from serial port to video console.
;        input from keyboard to serial port.
;
VIDEO0 JSR  OUTCHS   ;Switch ouput to serial port
;
VIDEO  JSR  TSTINP   ;keyboard input?
       BCS  VIDEO0   ;yes, output to serial port
       JSR  TSTINS   ;input from serial port?
       BCC  VIDEO    ;no, loop
       JSR  XDTEXT   ;yes, output to video console
       BRA  VIDEO    ;loop
;
; Function to find an input character in an input
; table
; Parameter:
;      A: input character
; Return:
;      character found if carry clear
;      U: Points to jump address
;
GETTAS ANDA #$7F     ;convert to 7-bit ASCII
       CMPA ,U       ;found character in table?
       BEQ  GETTS9   ;yes, go back with carry cleared
       LEAU $03,U    ;no, increment address to next char.
       CMPU #ENDHT   ;reached end?
       BNE  GETTAS   ;no, loop
       ORCC #%00000001 ;go back with carry set
       RTS
;
GETTS9 ANDCC #%11111110 ;go back with carry cleared
       RTS
;
; GO: Continue at specified address
;
GO     JSR  IN4HX    ;Get jump address, ok?
ERRO3  LBCS ERRHKS   ;no, jump to hks
       TFR  X,Y      ;Store address in Y
       JSR  PCRLF    ;new line
       LDS  #USERST-12 ;Stack > User-Bereich
       ANDCC #%10101111; Enable IRQ,FIRQ interrupt
GO1    CLRA
       TFR  A,DP     ;clear direct page for 6800 compatibility
       TFR  Y,PC     ;continue at specified address
;
; X: Jump back to FLEX
;
CFLEX  LDY  #WARMS   ;get warmstart adress into Y
       BRA  GO1      ;proceed as with GO command
;
; Function to input a start and end address
; from keyboard.
; return:
;     X: start address
;     D: end address
;
FROMTO LDX  #TFROM
       BSR  PDATA1   ;output text "FROM"
       JSR  IN4HX    ;get start address, ok?
       BCS  ERRO8    ;no, jump to HKS
       PSHS X        ;push address
       LDX  #TTO
       BSR  PDATA1   ;output text "TO"
       JSR  IN4HX    ;get end address, ok?
       BCS  ERRO4    ;no, jump to HKS
ERRO4  PULS D        ;restore start address in D
ERRO8  RTS
;
; convert upper nibble into ASCII 0-9, A-F
;
CHEXL  LSRA          ;shift upper nibble
       LSRA          ;into the lower one
       LSRA
       LSRA
;
; convert lower nibble into ASCII 0-9, A-F
;
CHEXR  ANDA #$0F     ;mask upper nibble
       ADDA #$90     ;convert binary value
       DAA           ;into ASCII 0-9, A-F
       ADCA #$40
       DAA
PDATA3 RTS
;
; Output hexadecimal 16-bit value located at ,X
;
OUT4H  BSR  OUT2H      ;4 Hex-Zeichen ausgeben
;
; Output hexadecimal 8-bit value located at ,X
;
OUT2H  LDA  ,X
       BSR  CHEXL    ;convert upper nibble
       BSR  OUTA     ;output character
       LDA  ,X+
       BSR  CHEXR    ;convert lower nibble
       BRA  OUTA     ;output character

; Output hexadecimal 16-bit value located at ,X
; followed by a space.
;
OUT4HX BSR  OUT2H    ;output two hex digits
;
; Output hexadecimal 8-bit value located at ,X
; followed by a space.
;
OUT2HX BSR  OUT2H    ;output two hex digits
PSPACE LDA  #$20
       BRA  OUTA     ;output space
;
; OUT-DATA functions
;
PDATA  BSR  PCRLF    ;output new line
;
PDATA1 LDA  ,X+      ;get ASCII value
       CMPA #$04     ;is it end of text (EOT)?
       BEQ  PDATA3   ;yes, return
       BSR  OUTCH    ;no, output character
       BRA  PDATA1   ;loop for next character
;
PCRLF  LDA  #$0D
       BSR  OUTCH    ;output CR
;
PLF    LDA  #$0A
OUT1   BRA  OUTCH    ;output LF
;
CLRHKS CLR  LODFLG-DIRPAG
       BRA  TOHKS0

INCH   BSR  INCHA    ;Input character with echo
       CMPA #$18     ;Has Ctrl-Y been input?
TOHKS0 LBEQ HKS      ;yes, abort to hks
;
;
; Output character to video console.
; If Ctrl-X is entered from keyboard or serial port return to HKS
;
OUTA   PSHS A
       JSR  TSTINP   ;Input from video console?
       BCC  OUTCH0   ;no, continue
       CMPA #$18     ;Is it Ctrl-X?
       BEQ  CLRHKS   ;yes, abort to hks
;
OUTCH0 BSR  TSTINS   ;Input from serial port?
       BCC  OUTCH1   ;no, continue
       CMPA #$18     ;Is it Ctrl-X?
       BEQ  CLRHKS   ;yes, abort to hks
;
OUTCH1 PULS A
OUTCH  TST  LODFLG
       LBNE XDTEXT
       TST  SERPAR   ;Is Serial or parallel Output?
       LBEQ XDTEXT   ;out parallel
;
;
; Output Character to serial port
;
OUTCHS PSHS B
OUTCS1 LDB  ACIACO
       BITB #2       ;Transmit not ready
       BEQ  OUTCS1
       STA  ACIADA   ;output character
       PULS PC,B
;
; Get status of serial port
;
SERSTA PSHS A
       LDA  ACIACO
       ANDA #%00000001
       PULS A,PC

LOAD0  BSR  SERSTA
       BNE  LOAD0    ;loop until got input from
                     ;serial port
;
; Input one Character from ACIA
;
TSTINS BSR  SERSTA   ;get serial status
       BEQ  NOSERI   ;nothing there
       LDA  ACIADA
       ORCC #%00000001 ;SEC, Carry setzen
       RTS
NOSERI ANDCC #%11111110 ;CLC
       RTS
;
; Input one Character
;
INCHA  TST  CURSOR   ;is cursor visible?
       BNE  INOCUR   ;yes, jump
       LDA  INVFLG
       PSHS A
       BSR  ESCOUT
       CLR  INVFLG
       BSR  ESCOUT
       BSR  INOCUR
       BSR  ESCOUT
       PSHS A
       LDA  1,S
       STA  INVFLG   ;restore old value of INVFLG
       LDA  ,S++     ;repeat input, make stack ok
ESCOUT PSHS A
       LDA  #$1B
       BSR  OUTCH
       PULS A,PC
;
; Input without changing cursor
;
INOCUR BSR  TSTIN
       BCC  INOCUR
       RTS

STATUS TST  SERPAR
       BNE  SERSTA   ;check serial status
PSTATU PSHS A
       TST  ISNEWC   ;look for char that has not been fetched
       BNE  NOTEMP   ;found one
       LDA  PIA1AC
       BPL  ISEMPT   ;empty
       LDA  PIA1AD   ;get char from PIA
       STA  NEWCHR   ;and save it
       COM  ISNEWC   ;indicate new char available
ISEMPT ORCC #%00000100  ;set ZERO Flag
NOTEMP PULS A,PC
;
; Input one Character from PIA
;
TSTIN  LDA  SERPAR   ;input from serial port?
       BNE  TSTINS   ;yes, check ACIA
;
TSTINP BSR  PSTATU   ;check parallel only
       BNE  INPPAR   ;found something
       ANDCC #%11111110 ;CLC
       RTS           ;else return
;
INPPAR LDA  NEWCHR   ;get character
       CLR  ISNEWC   ;and clear FLAG
;
CNVERT CMPA #$19     ;Ctrl-Y
       BEQ  TRANS1   ;Toggle upper/lower case
       TST  UL
       BEQ  TRANS2   ;Uppercase only?
LOWUP  CMPA #'a'     ;Yes
       BCS  TRANS2   ;lower than a
       CMPA #'}'     ;
       BCC  TRANS2   ;higher than }
       TST  DEUTSC
       BNE  DO_CNV   ;if german convert now
       CMPA #'{'
       BCC  TRANS2   ;if ASCII {|} not
DO_CNV ANDA #%11011111
TRANS2 ORCC #%00000001 ;SEC, set carry
       RTS
;
TRANS1 COM  UL       ;Toggle
       BRA  TRANS2
;
;
; Input 4 HEX-Character
;
IN4HX0 PSHS X          ;for Motorola comp.
       CLRB
       BSR  IN4HX      ;Get 4 HEX-Char.
       BCS  NONHEX
       TFR  X,D
       PULS X
       STD  ,X
       LDB  #$04
       JSR  INCH
       RTS
;
NONHEX PULS PC,X
;
IN4HX  BSR  BYTE       ;input upper byte
;
IN4HX1 BCS  RAUS       ;input error
       PSHS B          ;push B on stack
       TFR  A,B        ;Move A to B
       BSR  BYTE       ;input lower byte
       BCS  RAUS1      ;input error
       EXG  A,B        ;upper byte in A,
       TFR  D,X        ;lower byte in B => X
;
RAUS1  PULS B          ;Restore B
RAUS   RTS
;
BYTE   BSR  INHEX      ;input hex digit
;
BYTE0  BCS  RAUS       ;input error
BYTE1  ASLA            ;shift hex digit to
       ASLA            ;upper nibble
       ASLA
       ASLA
       STA  TEMPA      ;store into TEMPA
       BSR  INHEX      ;input second hex digit
       BCS  RAUS       ;input error
       ADDA TEMPA      ;join 1st and 2nd digit
       ANDCC #%11111110 ;CLC
       RTS
;
INHEX  JSR  INCH        ;get character
;;
INHEX1 CMPA #'0'        ;---
       BMI  TRANS2      ;between 0 - 9 ?
       CMPA #'9'        ;---
       BLE  IN1HG
       ANDA #%11011111  ;lower -> upper case
       CMPA #'A'        ;---
       BMI  TRANS2      ;between A - F ?
       CMPA #'F'        ;---
       BGT  TRANS2
       SUBA #$07
IN1HG  ANDA #$0F
       ANDCC #%11111110 ;CLC, no input error
       RTS
;
; Entry point after a breakpoint
; (or Single-Step-Interrupt)
;
SWIENT TST  11,S       ;Programm-Counter
       BNE  SWIEN0     ;due to SWI
       DEC  10,S       ;1 Byte
;
SWIEN0 DEC  11,S       ;back again
       LDX  10,S       ;Breakpoint address
       BRA  IRQEN0     ;yes, output register
;
IRQENT LDX  #0
;
IRQEN0 LDA  #%00111110
       STA  PIA1BC
       LDA  PIA1BD
       LDA  #DRCTPG
       TFR  A,DP
       STS  SAVUST-DIRPAG
       LDS  #STACK
IRQEN1 BSR  OUTREG
       LBRA HKS
;
; Function to output register values
;
OUTREG LDX  #TREGIS    ;load text pointer
       LDU  SAVUST-DIRPAG
       LDA  #2         ;2 one byte registers
       BSR  FOUREG
       CLRA
       BSR  FOUREG
       JSR  PDATA1
       LDX  #SAVUST
       LBRA OUT4HX
;
FOUREG LDB  #$04
FOURE0 PSHS B,A
       LDB  #$02
       BSR PRIREG
       PULS B,A
       DECB
       BNE  FOURE0
       RTS
;
; Function to output one register value
;
PRIREG LDY  #PDATA     ;Pointer to "PDATA"
       JSR  B,Y        ;to PDATA or PDATA1
       EXG  X,U        ;register pointer to X
       LDY  #OUT4HX    ;Pointer to "OUT4HX"
       LDA  2,S        ;Jump offset in A
       JSR  A,Y        ;to OUT4HX or OUT2HX
       EXG  X,U        ;text pointer to X
       RTS
;
;
; Interrupt entry points
;
SWI3   JMP  [SW3VEC]
SWI2   JMP  [SW2VEC]
FIRQ   JMP  [FIRQVC]
IRQ    JMP  [IRQVEC]
SWI    JMP  [SWIVEC]
NMI    JMP  [NMIVEC]


;**********************************
; General Table Search            *
;                                 *
; Entry: X - Points to Table      *
;        y - Points to Command    *
;        First Byte of table must *
;        contain item length      *
;        last Byte must be $FF    *
; Exit:  C - Z set if found,      *
;            Clear if not found   *
;        X - Points to adress of  *
;            Routine for match    *
;        A,B -  changed           *
;**********************************

SEARCH LDB  ,X+         ;Get item length
SERCH1 BSR  COMPAR      ;Compare current item
       ABX              ;advance to next item
       BEQ  SERCHX      ;exit if match
       LEAX $02,X       ;step over address
       TST  ,X          ;end of table?
       BPL  SERCH1      ;no, again
SERCHX RTS


;**********************************
; General String Compare          *
;                                 *
; Entry: X - Adress of String 1   *
;        Y - Adress of String 2   *
;        B - Length of Strings    *
;                                 *
; Exit:  C - Set per Compare 1:2  *
;        B,X,Y - unchanged        *
;        A - changed              *
;**********************************

COMPAR PSHS Y,X,B       ;Save Registers
COMP1  LDA  ,X+         ;get next character
       CMPA ,Y+         ;compare it
       BNE  COMPX       ;exit if no match
       DECB             ;decrement loop count
       BNE  COMP1
COMPX  PULS PC,Y,X,B    ;Restore registers and return


DEFTA1  FCB  0
       FCB  10         ;Lines per character
       FCB  1          ;empty lines 1
       FCB  2          ;empty lines 2
       FCB  3          ;empty lines 3
       FCB  25         ;lines of char. per line
       FCB  84         ;Nr. of char. per line
       FDB  250        ;Lines per screen used
       FCB  7          ;lines per char. in char.set
       FCB  0          ;Dummy
       FCB  6          ;Character dots horizontal
       FCB  0          ;Set Page to init
       FDB  CTLTAB     ;Controlle table
       FDB  CHRTBL     ;Character table
       FDB  SCRN1      ;RAM-Bank for screen
       FDB  SCRN2      ;dto bei double res.
       FDB  ESCVEC     ; Escape Vector
       FDB  ESCTAB     ; Escape Table
       FDB  $FD7F      ;Soft Scroll value ($027F)
       FDB  BOTOM1     ;Bottom pointer
       FDB  -$4000     ;Bump Vector
       FCB  $00
;
;
; Switch Memory Management Unit
; This subroutine switches the RAM in 16K Blocks
; (4K Blocks are possible if only one Byte differs
; from old values)
; Input: X = New  Value of RAM (low)
;        Y = New  Value of RAM (high)
;        B = Low Byte of MMU-Address
;
; Output: X,Y,B changed
;         U,A,CC unchanged
;
SWITCH PSHS U,A,CC
       ORCC #%01010000  ; SEI
       LDA  #$FF
       PSHS B,A
       PSHS Y,X
       LDU  SAVE
       LDA  #DRCTPG
       TFR  D,X
       LDD  ,X
       LDY  2,X
       PSHU Y,X,D
       STU  SAVE
       PULS Y,D
       STD  ,X
       STY  2,X
       PULS X
       STD  ,X
       STY  2,X
       PULS PC,U,A,CC
;
; Switch back Memory Management Unit
; This is a subroutine to switch back to
; the original contents of the MMU
; revers of switch subroutine
;
; Input: No
; Output: X,Y,B as the input of the subr. switch
;
SWTCHB PSHS U,A,CC
       ORCC #$50
       LDU  SAVE
       PULU Y,X,D
       STU  SAVE
       STD  ,X
       STY  2,X
       EXG  D,X
       LDA  #$FF
       EXG  D,X
       STD  ,X
       STY  2,X
       EXG  D,X
       PULS PC,U,A,CC
;
; INTSCR initializes the screen on single
; resolution.
; It clears the screen and sets up the
; base registers.
;
INTSCR LDX  #DEFTA1
INTSC1  LDU  #NRLINS-1
       LDY  #LPCNT
SETUP  LDD  ,X++
       STD  ,U++
       LEAY -$01,Y
       BNE  SETUP
       LDA  INTPAG-DIRPAG
       CLRB             ;reset scrolling
       STD  VIDPAG
       LSLA
       LSLA
       LSLA
       LSLA
       ADDA #$40
       STD  DBASE-DIRPAG
       LDD  #PIXLIN
       STD  EOLNX-DIRPAG
       CLR  INVFLG-DIRPAG
       DEC  INVFLG-DIRPAG   ;wir wollen keinen Cursor
       CLR  CURSOR-DIRPAG    ; und haben auch noch keinen
       CLR  SCROFS-DIRPAG
       LDD  XBOTOM-DIRPAG


       SUBD SCRLIN-DIRPAG
       ADDD NRLINS-1
       STD  BOTTOM-DIRPAG
       LDD  XBOTOM-DIRPAG
       STD  YADDR-DIRPAG
       CLRA
       CLRB
       STD  XADDR-DIRPAG
       STD  INSRTF-DIRPAG
       LDA  #$FF
       STA  COLTAB-DIRPAG
       RTS
;
;
; DTEXT functions as an intelligent terminal
; on entry register A must contain a valid
; ASCII character.
; It will be drawn on the screen with complete
; updating of the cursor after drawing.
; it writes the characters in 25 lines of
; 84 characters.
; Scrolling is provided if the bottom character
; line is reached.
; All registers are saved.
;
XDTEXT PSHS U,Y,X,DP,B,A,CC  ;Save Registers
       ORCC #%01010000 ;SEI
       LDB  #DRCTPG
       TFR  B,DP
       TST  NEWST2-DIRPAG    ; Already switched the stack
       BNE  DTEXT4     ; yes, so go on
       STS  SAVEST-DIRPAG    ; save user stack
       LDS  #STACK2    ;get new internal stack
       INC  NEWST2-DIRPAG    ; set flag
       BRA  DTEXT5
DTEXT4 INC  NEWST1-DIRPAG    ; set flag
DTEXT5 STA  ,S         ;save it
       LDX  SCREN1-DIRPAG
       TFR  X,Y
       LDB  #$E0
       JSR  SWITCH
       LDX  SCREN2-DIRPAG   ; select second screen
       TFR  X,Y
       LDB  #$E4
       JSR  SWITCH
       LDA  ,S
       TST  ESCFLG-DIRPAG    ; Escape sequence?
       LBNE ESCSEQ     ;yes, do sequence
       ANDA #$7F       ;only 7 bit
       STA  ,S         ;store it
       JSR  INVCUR     ;clear cursor
       CMPA #$20       ;control code?
       LBCS DTEXTC     ; yes, to control codes
ZF850  LDA  ,S         ;get value back
       JSR  XDCHAR     ;draw character
COLOR2 LDD  XADDR-DIRPAG    ; update cursor adress
       ADDD CHROFS-1
       STD  XADDR-DIRPAG
       CMPD EOLNX-DIRPAG     ;End of line?
       BCS  DTEXT8     ;yes, store it
       CLRA            ;Reset X-coordinate
       CLRB;
       STD  XADDR-DIRPAG
       LBRA LF
DTEXT8 JSR  INVCUR     ;display cursor again
DTEXT3 JSR  SWTCHB
       JSR  SWTCHB
       TST  NEWST1-DIRPAG
       BNE  DTEXT6     ;internal stack still using
       CLR  NEWST2-DIRPAG    ;reset flag
       LDS  SAVEST-DIRPAG    ;user stack back
DTEXT7 PULS PC,U,Y,X,DP,B,A,CC  ;restore registers
;
DTEXT6 CLR  NEWST1-DIRPAG   ; reset flag
       BRA  DTEXT7      ;exit

CURSON CLR  INVFLG-DIRPAG
       TST  CURSOR-DIRPAG
       BEQ  DTEXT8      ;Cursor is off
       BRA  DTEXT3
CURSOF TST  CURSOR-DIRPAG
       BEQ  ISOFF       ;Cursor is already off
       CLR  INVFLG-DIRPAG
       JSR  INVCUR      ;otherwise delete it
ISOFF  LDA  #1
       STA  INVFLG-DIRPAG
       BRA  DTEXT3

INVON  LDA  #$FF
SETINV STA  HIGHLI
       BRA  DTEXT8
INVOFF CLRA
       BRA  SETINV

DEUON  LDA  #$FF
DEU    STA  DEUTSC
       BRA  DTEXT8
DEUOFF CLRA
       BRA  DEU

SCRLUP BSR  SCROLL    ;scroll up
       BRA  DTEXT8

SCRUD  LDX  DBASE-DIRPAG
       LEAX D,X
       LDA  SCROFS-DIRPAG
       PSHS A
       LDB  #$80      ;compute row offset
       MUL
       LEAX D,X       ;address of row
       LEAU $0080,X   ;end pointer (end of line)
       CLRA
       JSR  CLEARS    ;and clear
       TST  SOFT-DIRPAG
       BMI  SCRUD2
       LDX  SOFT-DIRPAG      ; SCROLL delay
SCRUD1 MUL
       LEAX -$01,X
       BNE  SCRUD1
SCRUD2 PULS A,PC
;
; Scroll does the processing to scroll the
; screen.
;
SCROLL LDY  NRLINS-1   ;Number of lines to scroll
SCROL1 PSHS Y
       LDD  BUMPV      ;other side of the screen
       BSR  SCRUD
       PULS Y
       INCA            ;increm offset
       ANDA #$7F
       STA  SCROFS-DIRPAG
SCROL7 ASLA
SCROL4 STA  BILANF    ;actual scroll
       LEAY -$02,Y     ;all lines done ?
       BNE  SCROL1     ;no
       LDD  YADDR-DIRPAG     ;else update variables
       SUBD NRLINS-1
       BGE  SCROL8
       LDA  SCRLIN-DIRPAG
SCROL8 STD  YADDR-DIRPAG
       LDD  BOTTOM-DIRPAG
       SUBD NRLINS-1
       BGE  SCROL9
       LDA  SCRLIN-DIRPAG
SCROL9 STD  BOTTOM-DIRPAG
       RTS

SCROLD LDY  NRLINS-1   ;Number of lines to scroll
LF882  PSHS Y
       LDA  SCROFS
       SUBA #4         ;erst mal 4 abziehen, um richtig zu loeschen
       ANDA #$7F
       STA  SCROFS-DIRPAG
       LDD  BUMPV      ;other side of the screen
       BSR  SCRUD
       ADDA #3         ;nun wieder 3 dazu, bleibt -1
       STA  SCROFS
       PULS Y
LF8B5  ASLA
LF8B6  STA  BILANF     ;actual scroll
       LEAY -$02,Y     ;all lines done ?
       BNE  LF882      ;no
       LDD  YADDR-DIRPAG     ;else update variables
       ADDD NRLINS-1
       CLRA
LF8C5  STD  YADDR-DIRPAG
       LDD  BOTTOM-DIRPAG
       ADDD NRLINS-1
       CMPD SCRLIN
       CLRA
LF8CF  STD  BOTTOM-DIRPAG
       RTS

DTEXTC LDX  CTLTA-DIRPAG
       LDA  ,S
       ASLA
       JMP  [A,X]
ESCSEQ DEC  ESCFLG-DIRPAG
       BNE  ESCSE1
       LDX  ESCTBL-DIRPAG
       LEAY ,S
       JSR  SEARCH
       LBNE DTEXT8
       JMP  [,X]
ESCSE1 LDB  ESCFLG-DIRPAG
       DECB
       LDX  ESCVC-DIRPAG
       JMP  [B,X]

CTABHO FDB  DTEXT8    ;0
       FDB  DTEXT8    ;1
       FDB  DTEXT8    ;2
       FDB  DTEXT8    ;3
       FDB  DTEXT8    ;4
       FDB  CURSOF    ;5
       FDB  DTEXT8    ;6
       FDB  BELL      ;Bell
       FDB  DTEXT8    ;8
       FDB  DTEXT8    ;9
       FDB  LF        ;LF
       FDB  DC2       ;CURSOR Down
       FDB  FF        ;CLS
       FDB  CR        ;CR
       FDB  SCRLUP    ;scroll up
       FDB  SCRDWN    ;scroll down

       FDB  DTEXT8    ;^P
       FDB  DC1       ;^Q
       FDB  DC2       ;^R
       FDB  DC3       ;^S
       FDB  DC4       ;^T
       FDB  CURSON    ;^U
       FDB  DTEXT8    ;^V
       FDB  DTEXT8    ;^W
       FDB  DC3       ;^X CURSOR RIGHT
       FDB  DC4       ;^Y CURSOR LEFT
       FDB  DC1       ;^Z CURSOR UP
       FDB  ESCAPE
       FDB  SOFTSC    ; SOFT/HARD=(HOME DOWN)
       FDB  HOME      ;CURSOR HOME
       FDB  ERAEOL    ;1E
       FDB  ERAEOS    ;1F

CTLTAB FDB  DTEXT8    ;0
       FDB  DTEXT8    ;1
       FDB  DTEXT8    ;2
       FDB  DTEXT8    ;3
       FDB  DTEXT8    ;4
       FDB  CURSOF    ;5
       FDB  DTEXT8    ;6
       FDB  BELL      ;Bell
       FDB  BS        ;BS
       FDB  DTEXT8    ;9
       FDB  LF        ;LF
       FDB  DTEXT8    ;B
       FDB  FF        ;CLS
       FDB  CR        ;CR
       FDB  SCRLUP    ;scroll up
       FDB  SCRDWN    ;scroll down

       FDB  DTEXT8    ;10
       FDB  DC1
       FDB  DC2
       FDB  DC3
       FDB  DC4
       FDB  CURSON    ;15
       FDB  DTEXT8    ;16
       FDB  DTEXT8    ;17
       FDB  DTEXT8    ;18
       FDB  DTEXT8    ;19
       FDB  DTEXT8    ;1A
       FDB  ESCAPE
       FDB  HOME      ;CURSOR HOME
       FDB  SOFTSC
       FDB  DTEXT8    ;1E
       FDB  DTEXT8    ;1F
;
ESCVEC FDB  LCURS1
       FDB  LCURS2
       FDB  PDOTX1
       FDB  PDOTX2
       FDB  PDOTY1
       FDB  PDOTY2
       FDB  SETCUX    ;alternative Cursor-Positioning
       FDB  SETCUY
;
; Escape table
;
ESCTAB FCB  1         ; Nr. of characters to check
       FCC  "A"       ; Plot dot even
       FDB  PEVEN
       FCC  "B"       ; delete line
       FDB  DELLIN
       FCC  "C"       ; insert line 
       FDB  INSLIN
       FCC  "D"
       FDB  DEUON     ; german characterset
       FCC  "E"
       FDB  SPCURS    ; Cursorpositioning YX
       FCC  "F"
       FDB  INSCUR    ; Use an insert Cursor
       FCC  "G"
       FDB  OVRCUR    ; Use an overwrite Cursor (default)
       FCC  "I"
       FDB  DEUOFF    ; International characterset
       FCC  "="
       FDB  CURPS2    ; Cursorpositioning XY
       FCC  "P"
       FDB  INVON     ; INVerse ON
       FCC  "Q"
       FDB  INVOFF    ; INVerse OFF
       FCC  "K"
       FDB  ERAEOL    ; ERAse to End Of Line
       FCC  "Z"
       FDB  ERAEOS    ; ERAse to End Of Screen
       FCC  "p"
       FDB  INVON     ; INVerse ON
       FCC  "q"
       FDB  INVOFF    ; INVerse OFF
       FCC  "K"
       FDB  ERAEOL    ; ERAse to End Of Line
       FCC  "L"
       FDB  ERAEOL
       FCC  "k"
       FDB  ERAEOS    ; ERAse to End Of Screen
       FCC  "S"
       FDB  ERAEOS
       FCB  $FF

SCRDWN JSR  SCROLD
       BRA  LF2       ;display Cursor usw.

LF     LDD  YADDR-DIRPAG     ;update Y-coordinate
       CMPD BOTTOM-DIRPAG    ;bottom of screen?
       BNE  LF3        ;No, store new Y-coordinate
       JSR  SCROLL     ;else scroll the page
       BRA  LF2        ;and finish
LF3    SUBD NRLINS-1
       BGE  LF1
       LDA  SCRLIN-DIRPAG
LF1    STD  YADDR-DIRPAG     ;store Y-coordinate
LF2    LBRA DTEXT8     ;main exit

HOME   LDA  #$20
       JSR  LCUR11     ;Y-Koordinate to 0

CR     CLRA            ;reset X-coordinate
       CLRB
       STD  XADDR-DIRPAG
       BRA  LF2        ;and draw new cursor

BS     LDD  XADDR-DIRPAG
       BEQ  LF2        ;skip if already zero
       SUBD CHROFS-1   ;do backspace
       STD  XADDR-DIRPAG
       LDA  #$20       ;erase char. by a space
       JSR  XDCHAR
       BRA  LF2

FF     LDU  DBASE-DIRPAG     ;get display base
       LDD  BUMPV-DIRPAG
       LEAX D,U        ;compute end address
       CLRA            ;set nulls
       BSR  CLEARS     ;now clear screen
       STD  XADDR-DIRPAG     ;reset X-coordinate
       STB  BILANF    ;clear offset latch
       CLR  SCROFS-DIRPAG
       LDD  XBOTOM-DIRPAG
       SUBD SCRLIN-DIRPAG
       ADDD NRLINS-1
       STD  BOTTOM-DIRPAG
       LDD  XBOTOM-DIRPAG
       STD  YADDR-DIRPAG     ;and setup new Y-coorinate
       BRA  LF2        ;go invert and return

DC1    LDD  BOTTOM-DIRPAG    ;check for top of screen
       ADDD SCRLIN-DIRPAG    ;B now contains top
       ANDA SCRLIN-DIRPAG
       PSHS D          ;save it
       LDD  YADDR-DIRPAG     ;adjust Y-coordinate
       ADDD NRLINS-1
       ANDA SCRLIN-DIRPAG
       CMPD ,S++       ;compare to actual
       BEQ  LF2
       BRA  LF1

DC2    LDD  YADDR-DIRPAG     ;adjust Y-coordinate
       CMPD BOTTOM-DIRPAG    ;Bottom reached?
       BEQ  LF2
       BRA  LF3

DC3    LDD  XADDR-DIRPAG     ;adjust X-coord.
       ADDD CHROFS-1   ;position on new character
       CMPD EOLNX-DIRPAG     ;end of line?
       BCC  LF2        ;yes, do nothing
       STD  XADDR-DIRPAG     ;else store it
       BRA  LF2        ;and draw new cursor

DC4    LDD  XADDR-DIRPAG     ;adjust X-coordinate
       BEQ  LF2        ;exit if already on the left
       SUBD CHROFS-1
       STD  XADDR-DIRPAG
LF21   BRA  LF2        ;and draw new position
;
; Clear of fill subroutine modulo 16 byte
; Input: A= fill character
;        X= Low address
;        U= High adress + 1
; Output: B,Y,U changed
;
CLEARS PSHS X
       TFR  A,B
       TFR  D,X
       TFR  D,Y
CLEA1  PSHU Y,X,B,A   ;---
       PSHU Y,X,B,A   ;Clear 16 Byte
       PSHU X,B,A     ;---
       CMPU ,S        ;Finish ?
       BNE  CLEA1     ;no, again
       PULS PC,X

; Copy one rasterline (64 Byte)
; Input: X = Source-Addr
;        U = Dest-Addr
; Output: X,U changed
;
COPYL  PSHS A,Y
       LDA  #32
COPYL1 LDY  ,X++
       STY  ,U++
       DECA
       BNE  COPYL1
       PULS A,Y,PC
        
SOFTSC COM  SOFT-DIRPAG      ;compl. flag
       BRA  BELL2      ;and back

BELL   LDB  PIA2BD   ;get data register
       PSHS B         ;save value
       ORB  #%1000000 ;set Bell bit
       STB  PIA2BD   ;Store it
       LDX  #6000     ;Preset Counter
BELL1  MUL            ;Delay
       LEAX -1,X      ;Decrement
       BNE  BELL1
       PULS B         ;Get old value
       STB  PIA2BD   ;no bell now
BELL2  BRA  LF21

LCUR11 SUBA #$20      ;calculate Y coordinate
       CMPA LINES-DIRPAG
       BCC  ZFA0F
       LDB  NRLINS-DIRPAG
       MUL
       SUBD SCRLIN-DIRPAG
       NEGB
       ADCA #$00
       NEGA
       ADDD BOTTOM-DIRPAG
       SUBD NRLINS-1
       ANDA SCRLIN-DIRPAG
       STD  YADDR-DIRPAG
ZFA0F  RTS
;
; LCURS1 and LCURS2 process the escape
; sequence to load the cursor at an
; absolute line and character position
;
LCURS1 BSR  LCUR11
       LDA  #$04
MORCUP STA  ESCFLG-DIRPAG
       LBRA DTEXT3
SETCUY BSR  LCUR11
       BRA  ENDCUP    ;Cursorposition finished
SETCUX BSR  LCUR21
       LDA  #16
       BRA  MORCUP    ;loop, sequence not yet finished

LCUR21 SUBA #$20      ;calculate X coordinate
       CMPA NRCHR-DIRPAG
       BHI  ZFA21
       LDB  CHROFS-DIRPAG
       MUL
       STD  XADDR-DIRPAG
ZFA21  RTS

LCURS2 BSR  LCUR21
ENDCUP CLR  ESCFLG-DIRPAG
       LBRA DTEXT8

PEVEN  LDA  #6
       BRA  SPCUR1
;
PDOTX1 STA  GXADDR-DIRPAG
       LDA  #8
       BRA  SPCUR1
;
PDOTX2 STA  GXADDR+1
       LDA  #10
       BRA  SPCUR1
;
PDOTY1 STA  GYADDR-DIRPAG
       LDA  #12
       BRA  SPCUR1
;
PDOTY2 EXG  A,B
       LDA  GYADDR-DIRPAG
       TFR  D,Y
       LDX  GXADDR-DIRPAG    ;get X address
       JSR  PIXADR
       EORB ,X
       STB  ,X
       BRA  ENDCUP
;
SPCURS LDA  #2
SPCUR1 STA  ESCFLG-DIRPAG
       LBRA DTEXT3
CURPS2 LDA  #14
       BRA  SPCUR1

INSLIN LDD  YADDR-DIRPAG
       CMPD BOTTOM-DIRPAG
       BEQ  LTD8      ; if already last line do nothing
       SUBD NRLINS-1-DIRPAG ;the line is already deleted
       CLRA
       INCB           ;start one rasterline above
       INCB
       PSHS D
       LDD BOTTOM-DIRPAG
       INCB
REPTI  SUBB #1
       PSHS D
       LDX  #0
       LBSR OFFSET
       TFR X,U	     ; Dest Pointer in U
       LDD  ,S
       SUBD NRLINS-1-DIRPAG
       LDX  #0
       LBSR OFFSET   ; Source Pointer in X
       LBSR COPYL    ; Copy one Rasterline
       PULS D
       CMPD ,S
       BNE  REPTI
       PULS D
       ADDD NRLINS-1-DIRPAG
       TFR  D,Y
       LDX  #0
       LBSR ERASL     ;clear last line
LTD8   LBRA TD8

DELLIN LDD  YADDR-DIRPAG
       CMPD BOTTOM-DIRPAG
       BEQ  LTD8      ; if already last line do nothing
       SUBD NRLINS-1-DIRPAG ; Line is alread deleted
       CLRA
       INCB           ; Start one raster line above
       INCB
REPTD  SUBB #1
       PSHS D
       LDX  #0
       LBSR OFFSET
       TFR X,U	     ; Source Pointer in U
       LDD  ,S
       ADDD NRLINS-1-DIRPAG
       LDX  #0
       LBSR OFFSET   ; Dest Pointer in X
       EXG  X,U
       LBSR COPYL    ; Copy one Rasterline
       PULS D
       CMPD BOTTOM-DIRPAG
       BNE  REPTD
       ADDD NRLINS-1-DIRPAG
       TFR  D,Y
       LDX  #0
       LBSR ERASL     ;clear last line
       BRA  LTD8

EMUEXIT LDX  #TEXIT
LPEXIT  LDA  ,X+
        STA  EMUCMD
        BNE  LPEXIT
        JMP  HKS
TEXIT   FCC  "EXIT"
        FCB  $00

ORIG   SET  *
       IF   ORIG>(ANFANG+$A1F)
       ERR  "Program overlaps character table!"
       ENDIF

       ORG  $FA1F
;
; CHRTBL contains a complete ASCII
; upper and lower case character set
; in a 5 x 7 matrix with lower case
; descenders, making it effectively
; 5 x 9.
;
; German umlaut as upper case
       FCB    $88,$00,$70,$88,$F8,$88,$88       ;AE
       FCB    $88,$70,$88,$88,$88,$88,$70       ;OE
       FCB    $88,$00,$88,$88,$88,$88,$70       ;UE
; German umlaut as lower case
       FCB    $50,$00,$70,$08,$78,$88,$78       ;ae
       FCB    $50,$00,$70,$88,$88,$88,$70       ;oe
       FCB    $50,$00,$88,$88,$88,$98,$68       ;ue
       FCB    $70,$88,$B0,$88,$88,$A8,$90       ;SZ
; Character for DEL
       FCB    $01,$01,$09,$11,$A1,$C1,$E1       ;DEL
;
CHRTBL FCB    $00,$00,$00,$00,$00,$00,$00      ; SPACE
       FCB    $20,$20,$20,$20,$20,$00,$20      ; !
       FCB    $50,$50,$50,$00,$00,$00,$00      ; "
       FCB    $50,$50,$F8,$50,$F8,$50,$50      ; #
       FCB    $20,$78,$A0,$70,$28,$F0,$20      ; $
       FCB    $C0,$C8,$10,$20,$40,$98,$18      ; %
       FCB    $20,$50,$50,$60,$A8,$90,$68      ; &
       FCB    $10,$20,$40,$00,$00,$00,$00      ; '
       FCB    $10,$20,$40,$40,$40,$20,$10      ; (
       FCB    $40,$20,$10,$10,$10,$20,$40      ; )
       FCB    $00,$20,$A8,$70,$A8,$20,$00      ; *
       FCB    $00,$20,$20,$F8,$20,$20,$00      ; +
       FCB    $01,$01,$01,$01,$21,$21,$41      ; ,
       FCB    $00,$00,$00,$F8,$00,$00,$00      ; -
       FCB    $00,$00,$00,$00,$00,$20,$20      ; .
       FCB    $00,$08,$10,$20,$40,$80,$00      ; /
       FCB    $70,$88,$98,$A8,$C8,$88,$70      ; 0
       FCB    $20,$60,$20,$20,$20,$20,$70      ; 1
       FCB    $70,$88,$08,$10,$20,$40,$F8      ; 2
       FCB    $F8,$10,$20,$10,$08,$88,$70      ; 3
       FCB    $10,$30,$50,$90,$F8,$10,$10      ; 4
       FCB    $F8,$80,$F0,$08,$08,$88,$70      ; 5
       FCB    $30,$40,$80,$F0,$88,$88,$70      ; 6
       FCB    $F8,$08,$10,$20,$40,$40,$40      ; 7
       FCB    $70,$88,$88,$70,$88,$88,$70      ; 8
       FCB    $70,$88,$88,$78,$08,$10,$60      ; 9
       FCB    $00,$00,$30,$30,$00,$30,$30      ; :
       FCB    $01,$31,$31,$01,$31,$31,$61      ; ;
       FCB    $10,$20,$40,$80,$40,$20,$10      ; <
       FCB    $00,$00,$F8,$00,$F8,$00,$00      ; =
       FCB    $40,$20,$10,$08,$10,$20,$40      ; >
       FCB    $70,$88,$08,$10,$20,$00,$20      ; ?
       FCB    $30,$48,$B8,$A8,$B8,$40,$38      ; @
       FCB    $70,$88,$88,$F8,$88,$88,$88      ; A
       FCB    $F0,$88,$88,$F0,$88,$88,$F0      ; B
       FCB    $70,$88,$80,$80,$80,$88,$70      ; C
       FCB    $E0,$90,$88,$88,$88,$90,$E0      ; D
       FCB    $F8,$80,$80,$F0,$80,$80,$F8      ; E
       FCB    $F8,$80,$80,$F8,$80,$80,$80      ; F
       FCB    $78,$80,$80,$98,$88,$88,$78      ; G
       FCB    $88,$88,$88,$F8,$88,$88,$88      ; H
       FCB    $70,$20,$20,$20,$20,$20,$70      ; I
       FCB    $38,$10,$10,$10,$10,$90,$60      ; J
       FCB    $88,$90,$A0,$C0,$A0,$90,$88      ; K
       FCB    $80,$80,$80,$80,$80,$80,$F8      ; L
       FCB    $88,$D8,$A8,$A8,$88,$88,$88      ; M
       FCB    $88,$88,$C8,$A8,$98,$88,$88      ; N
       FCB    $70,$88,$88,$88,$88,$88,$70      ; O
       FCB    $F0,$88,$88,$F0,$80,$80,$80      ; P
       FCB    $70,$88,$88,$88,$A8,$90,$68      ; Q
       FCB    $F0,$88,$88,$F0,$A0,$90,$88      ; R
       FCB    $78,$80,$80,$70,$08,$08,$F0      ; S
       FCB    $F8,$20,$20,$20,$20,$20,$20      ; T
       FCB    $88,$88,$88,$88,$88,$88,$70      ; U
       FCB    $88,$88,$88,$88,$88,$50,$20      ; V
       FCB    $88,$88,$88,$A8,$A8,$A8,$50      ; W
       FCB    $88,$88,$50,$20,$50,$88,$88      ; X
       FCB    $88,$88,$88,$50,$20,$20,$20      ; Y
       FCB    $F8,$08,$10,$20,$40,$80,$F8      ; Z
       FCB    $30,$20,$20,$20,$20,$20,$30      ; [
       FCB    $00,$80,$40,$20,$10,$08,$00      ; \
       FCB    $30,$10,$10,$10,$10,$10,$30      ; ]
       FCB    $20,$70,$A8,$20,$20,$20,$20      ; ^
       FCB    $01,$01,$01,$01,$01,$01,$F9      ; ULIN
       FCB    $40,$20,$10,$00,$00,$00,$00      ; BACK
       FCB    $00,$00,$70,$08,$78,$88,$78      ; a
       FCB    $80,$80,$F0,$88,$88,$88,$F0      ; b
       FCB    $00,$00,$70,$88,$80,$80,$70      ; c
       FCB    $08,$08,$78,$88,$88,$88,$78      ; d
       FCB    $00,$00,$78,$88,$F8,$80,$70      ; e
       FCB    $30,$48,$E0,$40,$40,$40,$40      ; f
       FCB    $69,$99,$89,$99,$69,$09,$F1      ; g
       FCB    $80,$80,$F0,$88,$88,$88,$88      ; h
       FCB    $20,$00,$20,$20,$20,$20,$20      ; i
       FCB    $09,$09,$09,$09,$09,$91,$61      ; j
       FCB    $80,$80,$90,$A0,$D0,$88,$88      ; k
       FCB    $60,$20,$20,$20,$20,$20,$70      ; l
       FCB    $00,$00,$D0,$A8,$A8,$A8,$A8      ; m
       FCB    $00,$00,$F0,$88,$88,$88,$88      ; n
       FCB    $00,$00,$70,$88,$88,$88,$70      ; o
       FCB    $B1,$C9,$89,$C9,$B1,$81,$81      ; p
       FCB    $69,$99,$89,$99,$69,$09,$09      ; q
       FCB    $00,$00,$B0,$C8,$80,$80,$80      ; r
       FCB    $00,$00,$78,$80,$70,$08,$F0      ; s
       FCB    $40,$40,$E0,$40,$40,$48,$30      ; t
       FCB    $00,$00,$88,$88,$88,$98,$68      ; u
       FCB    $00,$00,$88,$88,$88,$50,$20      ; v
       FCB    $00,$00,$88,$88,$88,$A8,$50      ; w
       FCB    $00,$00,$88,$50,$20,$50,$88      ; x
       FCB    $89,$89,$89,$99,$69,$09,$F1      ; y
       FCB    $00,$00,$F8,$10,$20,$40,$F8      ; z
       FCB    $30,$40,$40,$80,$40,$40,$30      ; {
       FCB    $20,$20,$20,$20,$20,$20,$20      ; |
       FCB    $60,$10,$10,$08,$10,$10,$60      ; }
       FCB    $40,$A8,$10,$00,$00,$00,$00      ; ~

	;OPT  NOL
ORIG   SET  *
       IF   ORIG>(ANFANG+$CF0)
       ERR  "Character table too long!"
       ENDIF
       ;OPT  LIS

       ORG  ANFANG+$D00  ;$FD00-$FD0F patch with 0 if no RTC present

       FCB   $00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00

       ORG  ANFANG+$D40  ;$FCF0-$FD3F free for I/O
;
;
; Boot routine for the FLEX Operating System
; It loads the Bootblock at address $C100
; then it starts the routine
;
BOOT   JSR  CURSAN     ;Cursor on
       LDA  #FLCPAG / 256
       TFR  A,DP
       LDD  #$C10D     ;Drive 0, Restore 12 ms
       STA  FLDRIV-FLCPAG ;and start timer
       STB  FLCOMM-FLCPAG ;Cmd: restore 12 ms
       BSR  RET1       ;Delay
BOOT1  LDB  FLCOMM-FLCPAG ;check for ready
       BPL  BOOT3      ;jump if disk ready
BOOT9  CLR  FLDRIV-FLCPAG ;stop timer
       JSR  CURAUS     ;Cursor off
       JMP  HKS        ;boot failed, jump HKS
;
BOOT3  BITB #$01
       BNE  BOOT1      ;jump if still busy
;
; Read boot sector
       LDD  #$0180     ;Sektor 1, 256 Bytes
       STA  FLSEKT-FLCPAG
       LDX  #$C100+$80 ;Store Boots. at $C100
       LDA  #$84
       BSR  RET1
       STA  FLCOMM-FLCPAG ;Cmd: read one Sektor
BOOT2  LDA  FLCOMM-FLCPAG ;read status
       BITA #$02
       BNE  BOOT4     ;jump if data request
       BITA #$01      ;still busy?
       BNE  BOOT2     ;if busy jump back
; Check for errors: not ready, record not found,
; crc error, lost data
       BITA #$9C      ;are errors?
       BNE  BOOT9     ;yes, jmp to HKS
;
BOOT4  LDA  FLDATA-FLCPAG ;get byte
       STA  B,X        ;save it
       INCB
       BVC  BOOT2      ;jump to read next byte
       CLRA            ;set Direct Page to 0
       TFR  A,DP
       JMP  $C100      ;jump into boot sec. read
;
RET1   BSR  RTN1
RTN1   BSR  RTN
RTN    RTS

;BUMPX  MACRO
;       LEAX $40,X
;       CMPX DBASE-DIRPAG
;       BLO  *+6
;       LEAX -$4000,X
;       ENDM
;
; subroutine to delete the background on which
; a character has to be drawn.
;
LOESCH PSHS D,X
       LEAY <LOETBL,PC
       LDY  A,Y
       LDA  NRLINS
       PSHS A,Y
;
RCL    LDD  1,S
       ANDA ,X
       ANDB 1,X
       STD  ,X
       LEAX $40,X
       CMPX DBASE
       BLO  *+6
       LEAX -$4000,X
       DEC  ,S
       BNE  RCL
       LEAS 3,S
       PULS D,X,PC
;
LOETBL FDB  %0000000111111111,%1100000001111111
       FDB  %1111000000011111,%1111110000000111

;
;
; DCHAR does the processing to draw one
; character on the current cursor position.
;
XDCHAR PSHS U,Y,X,D    ;save all registers
       CMPA #BLANK     ;skip control chars
       LBLO NIXIS
       LDU  CHATAB     ;Point to Char.tab.
       CMPA #$7F       ;Is ist DEL?
       BNE  NODEL
       LDA  MULINS
       NEGA            ;Subtract length of
       LEAU A,U        ;one character
       BRA  DCHR
NODEL  TST  DEUTSC
       BEQ  NODEU
       CMPA #'^'       ;PFEIL
       BEQ  NODEU
       CMPA #$5F       ;ULIN
       BEQ  NODEU
       PSHS A
       ANDA #%11011111 ;to upper case
       CMPA #'Z'
       PULS A
       BLS  NODEU      ;no german umlaut
       BITA #%00100000
       BEQ  GROSS      ;jump to upper case
       SUBA #($7B-$5E) ;$7B -> $5E, ...
GROSS  SUBA #($5B+8)   ;$5B -> -8, ...
       NEGA            ;-8 -> 8, ...
       LDB  MULINS
       MUL
       COMA
       COMB
       ADDD #1         ;subtract from U
       BRA  DEU1

NODEU  LDB  MULINS     ;Nr. of lines per char
       SUBA #BLANK
       MUL             ;compute entry addr in table
DEU1   LEAU D,U        ;Point to this address
;
; Drawing the character
;
DCHR   LDX  XADDR      ;X-coordinate
       LDY  YADDR      ;and Y-coordinate
       LEAY 1,Y        ;start one line higher
       LBSR OFFSET     ;compute pixel address
       LBSR LOESCH
       LDY  #DCHAR1    ;computed goto
       LDB  ,U         ;check if shifted char
       RORB
       BCC  DCHAR0     ;no, do unshifted jump
       LEAX $80,X
DCHAR0 LEAX $40,X
       CMPX DBASE
       BLO  JUMP
       LEAX -$4000,X
JUMP   JMP  A,Y
;
DCHAR1 BRA  CASE1
       BRA  CASE2
       BRA  CASE3
CASE4  LDD  #$0704
       BRA  CASE40
;
CASE3  LDD  #$0710
CASE40 PSHS D
CASE30 LDA  ,U+
       LSRA
       LDB  1,S
       MUL
       ADDD ,X
       STD  ,X
       LEAX $40,X
       CMPX DBASE
       BLO  *+6
       LEAX -$4000,X
       DEC  ,S
       BNE  CASE30
       LEAS 2,S
HIGH1  TST  HIGHLI
       BEQ  NIXIS
       LBSR XINV       ; inverse display character 
NIXIS  PULS D,X,Y,U,PC
;
CASE2  LDB  MULINS
CASE20 LDA  ,U+
       LSRA
       LSRA
       LSRA
       ADDA ,X
       STA  ,X
       LEAX $40,X
       CMPX DBASE
       BLO  *+6
       LEAX -$4000,X
       DECB
       BNE  CASE20
       BRA  HIGH1

CASE1  LDB  MULINS
CASE10 LDA  ,U+
       LSRA
       ADDA ,X
       STA  ,X
       LEAX $40,X
       CMPX DBASE
       BLO  *+6
       LEAX -$4000,X
       DECB
       BNE  CASE10
       BRA  HIGH1
;
; Subr. to bump the display pointer
;
BUMP   LEAX 64,X
BUMP0  CMPX DBASE
       BLO  BUMP1
       LEAX -$4000,X
BUMP1  RTS
;
; Cursor left, near the left character
;
INVTB2 FDB  $FE00,$3F80,$0FE0,$03F8
;
; Curosr right, small
;
INVTBL FDB  $7E00,$1F80,$07E0,$01F8
;
; XINV does the processing to invert
; the cursor
; ATTENTION: DP register must be set before
;
XINV   PSHS A
       LDA  INSRTF-DIRPAG
       CLR  INSRTF-DIRPAG
       BSR  INVCU1
       STA  INSRTF-DIRPAG
       PULS A,PC
;
; INVCUR does the processing to invert one
; character on the current cursor position
;
INVCUR TST  INVFLG
       BNE  INV12
       COM  CURSOR   ;Cursor complement
INVCU1 PSHS D,X,Y
       LDB  #DRCTPG  ;set direct page
       TFR  B,DP
       LDX  XADDR-DIRPAG
       LDY  YADDR-DIRPAG
       LEAY 1,Y
       TST  INSRTF-DIRPAG
       BEQ  XINV0
       LDB  NRLINS-DIRPAG
       SUBB #2
       NEGB
       LEAY B,Y
XINV0  BSR  OFFSET
       LDB  NRLINS-DIRPAG
       TST  INSRTF-DIRPAG
       BEQ  XINV1
       LDB  #2
XINV1  LDY  #INVTBL
       TST  HIGHLI-DIRPAG
       BEQ  INV11
       LDY  #INVTB2
INV11  LEAY A,Y
       PSHS B
REPINV LDD  ,Y
       EORA ,X
       EORB 1,X
       STD  ,X
       LEAX $40,X
       CMPX DBASE-DIRPAG
       BLO  *+6
       LEAX -$4000,X
       DEC  ,S
       BNE  REPINV
       LEAS 1,S
       PULS D,X,Y,PC
INV12  RTS
;
; PIXADR computes the byte offset from display
; X and Y Registers contain the coordinates
; on exit X contains the memory address,
; A the Bit-Offset
;
PIXADR BSR  OFFSET     ;compute offset
       PSHS A          ;save accu
       LDA  SCROFS     ;get screen offset
       LDB  #128       ;compute offset
       MUL
       LEAX D,X        ;adjust pointer
       LBSR  BUMP0     ;check for end of display
       LDA  ,S         ;restore accu
       CLRB            ;reset accu
       INCA  
       ORCC #%00000001 ; SEC, Prepare carry for shift
PIXA1  RORB
       DECA
       BNE  PIXA1      ;get the bit position
       PULS A,PC
;
; OFFSET computes byte offset from display
; base address.
; Bit offset is in A (Range 0 - 7)
; On entry X-register and Y-register contain
; X and Y coordinates.
;
OFFSET TFR  X,D        ;column pointer in X
       LDX  DBASE      ;get display base address
       PSHS D          ;save complete address
       ANDB #%00000110 ;mask byte-address
       STB  ,S         ;save bit-address
       LDB  1,S        ;get byte-address
       LSRA            ;divide by 8
       RORB
       LSRB
       LSRB
       SUBB #$40       ;other end of the line
       LEAX B,X        ;get byte pointer
       TFR  Y,D        ;get row pointer in B
       LDA  #$40
       MUL
       NEGB
       ADCA #0
       NEGA
       LEAX D,X        ;X now contains right Byte adr
       PULS D,PC       ;Restore Bit-Adress in Accu A

ESCAPE INC  ESCFLG     ;set escape flag
       LBRA DTEXT3     ;and back

OVRCUR CLRA            ;set an overwrite cursor
       BRA  INSCU1

INSCUR LDA  #1        ;set an insert cursor
INSCU1 TST  CURSOR-DIRPAG
       BEQ  INSCU2
       JSR  INVCUR    ; clear cursor
INSCU2 STA  INSRTF-DIRPAG
       BRA  TD8	; draw cursor again

ERAEOL BSR  ERALIN
TD8    LBRA DTEXT8

ERALIN LDX  XADDR     ;clear from cursor
       LDY  YADDR
ERASL  LEAY 1,Y
ERALU  PSHS X,Y       ;Y -> X
       BSR  OFFSET
       LBSR LOESCH    ;clear char. on cursorpos.
       PULS D,Y       ;X -> D, Y -> Y
       ADDD CHROFS-1
       CMPD EOLNX
       BCC  ENDEL     ;at end, show cursor
       TFR  D,X
       BRA  ERALU     ;clear next position
ENDEL  RTS

ERAEOS BSR  ERALIN
       LDD  YADDR
       CMPD BOTTOM
       BEQ  TD8       ;maybe it was the last one
       SUBD NRLINS-1  ;the line already has been deleted
       CLRA
       INCB           ;start one raster line above
       INCB
REPT   SUBB #1
       PSHS D,U
       TFR  D,Y
       LDX  #0
       BSR  OFFSET
       LEAU 64,X
       CLRA
       LBSR CLEARS    ;clear one raster line
       PULS D,U
       CMPD BOTTOM
       BNE  REPT      ;do loop
LASTLI TFR  D,Y
       LDX  #0
       BSR  ERASL     ;clear last line
       BRA  TD8


GERMAN LDA  #$FF
TOG    STA  DEUTSC
       JMP  HKS
ASCII  CLRA
       BRA  TOG

CURSAN LDA  #$15
MAKE   JMP  XDTEXT
CURAUS LDA  #5
       BRA  MAKE

;       OPT  NOL
ORI    SET  *
       IF   ORI>ANFANG+$FE0
       ERR  "No space left for MMU I/O device!"
       ENDIF
;       OPT  LIS

       ORG  ANFANG+$FF0

ZFFF0  FDB  0

       FDB  SWI3
       FDB  SWI2
       FDB  FIRQ
       FDB  IRQ
       FDB  SWI
       FDB  NMI
       FDB  ZRESTA

       END
