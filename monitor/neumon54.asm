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

; SYM 6
; OPT -G,P,M,E,-C,

; commandoline for assembler as6809:
;    as6809,neumon54.asm -l neumon54.lis -o neumon54.hex
; an existing hex-file will be deleted.
;


; PAG
VERS   EQU  4      ;for ELTEC compatibility

ANFANG EQU  $F000
DIRPAG EQU  $EF00

DRCTPG EQU  DIRPAG/$100
;       SETDP  DRCTPG

; Zero-Page Locations
       ORG  DIRPAG+1

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
       ERR  "Dirpag zu lang vor $58"
       ENDI
;       OPT  LIS

       ORG  DIRPAG+$58

HIGHLI RMB  1         ;highlight flag
DEUTSC RMB  1         ;FlAG for characterset
NEWCHR RMB  1         ;last input from parallel keyboard
CURSOR RMB  1         ;Cursor visible / unvisible
UNUSED RMB  1
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
FLCOMM EQU  $FD30      ;commandregister
FLSEKT EQU  $FD32      ;sectorregister
FLDATA EQU  $FD33      ;dataregister
FLDRIV EQU  $FD38      ;driveselectregister
;
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
;
;       OPT  NOL
       IF   LVN>ANFANG+$8D
       ERR  "too much code for versionnumber"
       ENDI
;       OPT  LIS
;
; outputtexts
;
       ORG ANFANG+$8D-17
INITSP FCB  0		; initial value for SERPAR
			; (can be set by by flexemu after load)
MESEUR EQU  *
THALLO EQU  *
       FCC  "EUROCOM MONITOR "
LVN    FCC  "V5."
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
DEFCOM FCC  'HARDCOPY'
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
; RESTART Einsprung
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
       CLR  LODFLG-DIRPAG   ;clear loagflag
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
MEMORY JSR  IN4HX      ;Adresse von Tastatur
       STX  MEMADR-DIRPAG    ;konservieren. Richtig?
       BCS  ERRHKS     ;nein, Eingabe-Fehler
       JSR  PSPACE     ;Space ausgeben
;
MEMOR1 JSR  OUT2HX     ;Adress-Inhalt ausgeben
       LEAX -$01,X     ;Pointer wieder zurueck
;
MEMOR2 JSR  INCH       ;Eingabe holen
       JSR  LOWUP      ;Upper case
       LDU  #MEMTAB    ;Anfang Eingabe-Tabelle
       JSR  GETTAS     ;Eingabe in Tabelle
       BCC  HKS1       ;ja, bearbeiten
       JSR  INHEX1     ;nein, restl. Byte
       JSR  BYTE0      ;holen, richtig?
ERRO6  BCS  ERRHKS     ;nein, Eingabe-Fehler
       STA  ,X         ;ja, Byte abspeichern
       CMPA ,X         ;Adresse schreibfaehig?
       BNE  ERRHKS     ;nein, Fehler
       BRA  MEMOR2     ;naechste Eingabe holen
;
; SLASH: Naechste Speicher-Zelle oeffnen
;
SLASH  BSR  INCADR     ;Auf naechste Adresse
       BRA  MEMOR1     ;Inhalt ausgeben
;
; LINE-FEED: Naechste Speicher-Zelle oeffnen
;
LINFED BSR  INCADR     ;Auf naechste Adresse
       LDA  #$0D       ;CR
       JSR  OUTA       ;ausgeben
;
LINFE0 LDX  #MEMADR    ;Memory-Adresse
       JSR  OUT4HX     ;ausgeben
       LDX  MEMADR-DIRPAG    ;Adress-Inhalt
       BRA  MEMOR1     ;ausgeben
;
; POINT: Speicherzelle erneut oeffnen
;
POINT  LDX  MEMADR-DIRPAG    ;Inhalt der Adresse
       BRA  MEMOR1     ;nochmal ausgeben
;
; UPARROW: Vorige Speicherzelle oeffnen
;
UPAROW LDX  MEMADR-DIRPAG    ;Pointer auf vorige
       LEAX -$01,X     ;Adresse und wieder
       STX  MEMADR-DIRPAG    ;konservieren
       JSR  PCRLF      ;Zeilen-Vorschub,
       BRA  LINFE0     ;Adr.+Inhalt ausgeben
;
; Unterprogramm zum erhoehen des Memory-
; Adress-Zeigers um 1
;
INCADR LDX  MEMADR-DIRPAG    ;Pointer auf naechste
       LEAX $01,X      ;Adresse und wieder
       STX  MEMADR-DIRPAG    ;konservieren
       RTS
;
; TABLE: Ausgabe von Speicher-Inhalten
;        in HEX- und ASCII-Form
;
TABLE  JSR  FROMTO     ;Bereich einholen
       BCS  ERRO6
       STD  TEMPX-DIRPAG     ;Von-Adresse und
       LEAX $01,X      ;Bis-Adresse + 1
       STX  TEMPY-DIRPAG     ;konservieren
TABLE0 JSR  PCRLF      ;Zeilenvorschub
       LDX  #TEMPX     ;naechste Adresse
       JSR  OUT4HX     ;ausgeben
       LDB  #16        ;16 Inhalte/Zeilen
       LDX  TEMPX      ;Pointer auf Adresse
       PSHS X          ;Pointer retten
TABLE1 CMPX TEMPY      ;Bis-Adresse erreicht?
       BEQ  END_TA     ;bei Ende, ASCII-Dump
       PSHS B          ;Zaehler konservieren
       JSR  OUT2HX     ;Adress-Inhalt ausgeben
       PULS B          ;Zaehler zurueck
       STX  TEMPX-DIRPAG     ;Adresse konservieren
       BSR  HALTAN     ;Ausgabe unterbrechen?
       DECB            ;16 Inhalte ausgegeben?
       BNE  TABLE1     ;nein, weiter
ASDUMP PULS X          ;Pointer zurueck
       LDB  #17        ;16+1
       JSR  PSPACE     ;Zwischenraum ausgeben
NEXTT  CMPX TEMPY      ;Bis-Adresse erreicht?
       LBEQ HKS        ;Schluss
       BSR  HALTAN     ;Ausgabe unterbrechen?
       DECB            ;Alle Inhalte ausg.?
       BEQ  TABLE0     ;ja, naechste Zeile
       LDA  ,X+        ;Inhalt holen
       ANDA #$7F       ;oberstes Bit weg
       CMPA #$20
       BHS  ISAS
       LDA  #$5F       ;underline
ISAS   JSR  OUTCH      ;Zeichen ausgeben
       BRA  NEXTT
;
END_TA LDA  #3
       MUL
ENDT1  JSR  PSPACE     ;fuer jedes nicht
       DECB            ;ausgegebene Byte 3 Spaces
       BNE  ENDT1
       BRA  ASDUMP     ;nun den Rest als ASCII
;
HALTAN PSHS A,B,X,Y,U,DP,CC
       JSR  TSTIN      ;Liegt Eingabe vor?
       BCC  NOHALT     ;nein, zurueck
       JSR  INCHA      ;Auf Eingabe warten
       CMPA #$0D       ;Bei CR
       LBEQ HKS        ;abbrechen
NOHALT PULS D,X,Y,U,DP,CC,PC
;
; PUT: Fuellen eines Speicherbereiches
; mit einem bestimmten Wert
;
PUT    JSR  FROMTO       ;Anfang und Ende holen
       LBCS ERRHKS       ;Fehler bei der Eingabe
       STD  TEMPX
       LEAX 1,X
       STX  TEMPY
       LDX  #WERTMS
       JSR  PDATA1
       JSR  BYTE         ;den Wert holen
       LDX  TEMPX
       STA  ,X           ;gleich in die Anfangsadresse schreiben
PUTLUP CMPX TEMPY
       LBEQ HKS          ;alles gefuellt
       STA  ,X+
       BRA  PUTLUP
;
; VIDEO: Reaktion des Mikrocomputers
;        wie ein Video-Terminal
;
VIDEO0 JSR  OUTCHS     ;Ausgabe auf ACIA
;
VIDEO  JSR  TSTINP     ;PIA-Eingabe erfolgt?
       BCS  VIDEO0     ;ja, auf ACIA ausgeben
       JSR  TSTINS     ;ACIA-Eingabe erfolgt?
       BCC  VIDEO      ;nein, erneut testen
       JSR  XDTEXT     ;ja, auf Bildsch. schr.
       BRA  VIDEO      ;erneut ueberpruefen
;
; Unterprogramm zum Auffinden einer
; Tasten-Eingabe in der Eingabe-Tabelle
;
GETTAS ANDA #$7F       ;Maskiere A
       CMPA ,U         ;Eingabe in Tabelle?
       BEQ  ZF2E9      ;ja, carry loeschen
       LEAU $03,U      ;nein
       CMPU #ENDHT     ;alles ueberprueft?
       BNE  GETTAS     ;nein, weiter pruefen
       ORCC #%00000001 ; SEC, Carry setzen
       RTS
;
ZF2E9  ANDCC #%11111110 ;CLC, Eingabe gueltig, carry
       RTS             ;loeschen, zurueck
;
; GO: Benutzer-Programme starten
;
GO     JSR  IN4HX      ;Startadresse holen
ERRO3  LBCS ERRHKS     ;Eingabefehler, HKS
       TFR  X,Y        ;Adresse konservieren
       JSR  PCRLF      ;Zeilen-Vorschub
       LDS  #USERST-12 ;Stack > User-Bereich
       ANDCC #%10101111; CLI, Enable Interrupt
GO1    CLRA            ;Set Direct Page
       TFR  A,DP       ;for 6800 Compatibility
       TFR  Y,PC       ;Zur Start-Adresse
;
; X: Einsprung in FLEX
;
CFLEX  LDY  #WARMS     ;Warmstartadresse holen
       BRA  GO1        ;sonst wie GO
;
; Unterprogramm zum Einlesen von Anfangs-
; und End-Adresse von der Tastatur
;
FROMTO LDX  #TFROM     ;Text "FROM"
       BSR  PDATA1     ;ausgeben
       JSR  IN4HX      ;Von-Adresse holen
       BCS  ERRO8      ;Eingabe-Fehler, HKS
       PSHS X          ;Adresse konservieren
       LDX  #TTO       ;Text "TO"
       BSR  PDATA1     ;ausgeben
       JSR  IN4HX      ;Bis-Adresse holen
       BCS  ERRO4      ;Eingabe-Fehler, HKS
ERRO4  PULS D          ;Von-Adresse zurueck
ERRO8  RTS             ;Ruecksprung
;
;
CHEXL  LSRA            ;Das obere
       LSRA            ;Halbbyte in das
       LSRA            ;untere
       LSRA            ;rotieren
;
CHEXR  ANDA #$0F       ;Oberes Halbbyte weg
       ADDA #$90       ;Binaer-Wert in
       DAA             ;in ASCII-
       ADCA #$40       ;Wert
       DAA             ;umwandeln
PDATA3 RTS
;
; OUTHEX-Routinen
;
OUT4H  BSR  OUT2H      ;4 Hex-Zeichen ausgeben
;
OUT2H  LDA  ,X
       BSR  CHEXL      ;oberes Halbbyte wandeln
       BSR  OUTA       ;und ausgeben
       LDA  ,X+
       BSR  CHEXR      ;unteres Halbbyte wandeln
       BRA  OUTA       ;und ausgeben
;
OUT4HX BSR  OUT2H      ;4 Hexzeichen + Space
;
OUT2HX BSR  OUT2H      ;2 Hexzeichen +
PSPACE LDA  #$20       ;Space
       BRA  OUTA       ;ausgeben
;
; OUT-DATA-Routinen
;
PDATA  BSR  PCRLF      ;Zeilen-Vorschub
;
PDATA1 LDA  ,X+        ;ASCII-Wert holen
       CMPA #$04       ;End of Text (EOT)?
       BEQ  PDATA3     ;ja, Ruecksprung
       BSR  OUTCH      ;nein, Zeichen ausgeben
       BRA  PDATA1     ;naechstes Zeichen
;
PCRLF  LDA  #$0D       ;CR
       BSR  OUTCH      ;Ausgeben
;
PLF    LDA  #$0A       ;LF
OUT1   BRA  OUTCH      ;ausgeben
;
CLRHKS CLR  LODFLG-DIRPAG
       BRA  TOHKS0

INCH   BSR  INCHA    ;In Char. mit Echo
       CMPA #$18     ;^X eingegeben?
TOHKS0 LBEQ HKS
;
;
; Character ausgeben
;
OUTA   PSHS A          ;Accu konservieren
       JSR  TSTINP     ;Eingabe ueber PIA?
       BCC  OUTCH0     ;nein, weiter
       CMPA #$18       ;^X eingegeben?
       BEQ  CLRHKS     ;ja, Abbruch zur HKS
;
OUTCH0 BSR  TSTINS     ;Eingabe von ACIA?
       BCC  OUTCH1     ;nein, weiter
       CMPA #$18       ;^X eingegeben?
       BEQ  CLRHKS     ;ja, Abbruch zur HKS
;
OUTCH1 PULS A
OUTCH  TST  LODFLG
       LBNE XDTEXT
       TST  SERPAR    ;Ser./par. Output?
       LBEQ XDTEXT     ;out parallel
;
;
; Output Character to ACIA
;
OUTCHS PSHS B
OUTCS1 LDB  ACIACO
       BITB #2         ;Transmit not ready
       BEQ  OUTCS1
       STA  ACIADA    ;output character
       PULS PC,B
;
; Statusabfrage der ACIA
;
SERSTA PSHS A
       LDA  ACIACO
       ANDA #%00000001
       PULS A,PC

LOAD0  BSR  SERSTA
       BNE  LOAD0    ;noch keine Eingabe bis jetzt
;
; Input one Character from ACIA
;
TSTINS BSR  SERSTA    ;get serial status
       BEQ  NOSERI    ;nothing there
       LDA  ACIADA
       ORCC #%00000001 ; SEC, Carry setzen
       RTS
NOSERI ANDCC #%11111110 ;CLC
       RTS
;
; Input one Character
;
INCHA  TST  CURSOR
       BNE  INOCUR    ;der Cursor ist schon zu sehen
       LDA  INVFLG
       PSHS A
       BSR  ESCOUT
       CLR  INVFLG
       BSR  ESCOUT
       BSR  INOCUR
       BSR  ESCOUT
       PSHS A
       LDA  1,S       ;den alten Inhalt von INVFLG
       STA  INVFLG   ;Cursor wieder auf alten Stand
       LDA  ,S++      ;Eingabe wiederh. und Stack ok
ESCOUT PSHS A
       LDA  #$1B
       JSR  OUTCH
       PULS A,PC
;
; Eingabe, ohne dass Cursor beeinflusst wird
;
INOCUR BSR  TSTIN
       BCC  INOCUR
       RTS

STATUS TST  SERPAR
       BNE  SERSTA    ;check serial status
PSTATU PSHS A
       TST  NEWCHR   ;look for char that has not been fetched
       BNE  NOTEMP    ;found one
       LDA  PIA1AC
       BPL  ISEMPT    ;empty
       LDA  PIA1AD   ;get char from PIA
       STA  NEWCHR   ;and save it
ISEMPT ORCC #%00000100  ;set ZERO Flag
NOTEMP PULS A,PC
;
; Input one Character from PIA
;
TSTIN  LDA  SERPAR    ;Serielle Eingabe?
       BNE  TSTINS     ;ja, ACIA ueberpruefen
;
TSTINP BSR  PSTATU     ;check parallel only
       BNE  INPPAR     ;found something
       ANDCC #%11111110 ;CLC
       RTS             ;else return
;
INPPAR LDA  NEWCHR    ;get character
       CLR  NEWCHR    ;and clear status
;
CNVERT CMPA #$19       ;^Y
       BEQ  TRANS1     ;Toggle upper/lower case
       TST  UL
       BEQ  TRANS2     ;Uppercase only?
LOWUP  CMPA #'a'        ;Yes
       BCS  TRANS2     ;lower than a
       CMPA #'}'        ;
       BCC  TRANS2     ;higher than }
       TST  DEUTSC
       BNE  DO_CNV     ;bei deutsch jetzt konvert.
       CMPA #'{'
       BCC  TRANS2     ;bei internat. {|} nicht
DO_CNV ANDA #%11011111
TRANS2 ORCC #%00000001 ; SEC, Carry setzen
       RTS
;
TRANS1 COM  UL        ;Toggle
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
IN4HX  BSR  BYTE       ;High-Byte einlesen
;
IN4HX1 BCS  RAUS       ;Eingabe-Fehler
       PSHS B          ;Accu B retten
       TFR  A,B        ;Akku A nach B retten
       BSR  BYTE       ;Low-Byte einlesen
       BCS  RAUS1      ;Eingabe-Fehler
       EXG  A,B        ;High-Byte in A, Low-Byte
       TFR  D,X        ;in B, Beide in D
;
RAUS1  PULS B          ;Accu B restaurieren
RAUS   RTS
;
BYTE   BSR  INHEX      ;HEX-Char. einlesen
;
BYTE0  BCS  RAUS       ;Eingabe-Fehler
BYTE1  ASLA            ;HEX-Zeichen
       ASLA            ;linksbuendig
       ASLA            ;in Accu A
       ASLA            ;rotieren und
       STA  TEMPA     ;zwischenspeichern
       BSR  INHEX      ;HEX-Zeichen einholen
       BCS  RAUS       ;Eingabe-Fehler
       ADDA TEMPA     ;Zum ersten Zeichen dazu
       ANDCC #%11111110 ;CLC
       RTS
;
INHEX  JSR  INCH       ;Zeichen einholen
;;
INHEX1 CMPA #'0'        ;---
       BMI  TRANS2     ;zwischen 0 - 9 ?
       CMPA #'9'        ;---
       BLE  IN1HG
       ANDA #%11011111 ;lower -> upper case
       CMPA #'A'        ;---
       BMI  TRANS2     ;zwischen A - F ?
       CMPA #'F'        ;---
       BGT  TRANS2
       SUBA #$07
IN1HG  ANDA #$0F
       ANDCC #%11111110 ;CLC, kein Eingabe-Fehler
       RTS
;
; Einsprung-Stelle nach Breakpoint
; (oder Single-Step-Interrupt)
;
SWIENT TST  11,S       ;Programm-Counter
       BNE  SWIEN0     ;wegen SWI wieder
       DEC  10,S       ;um 1 Byte
;
SWIEN0 DEC  11,S       ;zurueck
       LDX  10,S       ;Breakpoint-Adresse
       BRA  IRQEN0     ;ja, Register ausgeben
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
; Unterprogramm zur Registerausgabe
;
OUTREG LDX  #TREGIS    ;Text-Pointer laden
       LDU  SAVUST-DIRPAG    ; Register-Pointer laden
       LDA  #2         ;2 1-Byte-Register
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
; Unterprogramm zur Ausgabe
; eines Registers
;
PRIREG LDY  #PDATA     ;Pointer auf "PDATA"
       JSR  B,Y        ;nach PDATA oder PDATA1
       EXG  X,U        ;Register-Pointer nach X
       LDY  #OUT4HX    ;Pointer auf "OUT4HX"
       LDA  2,S        ;Sprung-Offset in A
       JSR  A,Y        ;nach OUT4HX oder OUT2HX
       EXG  X,U        ;Text-Pointer nach X
       RTS
;
;
; Interrupt Einsprung Adressen
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
       LDD  0,X
       LDY  2,X
       PSHU Y,X,D
       STU  SAVE
       PULS Y,D
       STD  0,X
       STY  2,X
       PULS X
       STD  0,X
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
       STD  0,X
       STY  2,X
       EXG  D,X
       LDA  #$FF
       EXG  D,X
       STD  0,X
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

LCUR11 SUBA #$20      ;Y-Koordinate berechnen
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
       BRA  ENDCUP    ;Cursorposi zu ende
SETCUX BSR  LCUR21
       LDA  #16
       BRA  MORCUP    ;Sequenz noch nicht zu ende

LCUR21 SUBA #$20      ;X-Koordinate berechnen
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

INSLIN LDD  YADDR
       CMPD BOTTOM
       BEQ  LTD8       ; if already last line do nothing
       SUBD NRLINS-1  ;die Zeile ist ja schon geloescht
       CLRA
       INCB           ;eine Rasterzeile hoeher anfangen
       INCB
       PSHS D
       LDD BOTTOM
       INCB
REPTI  SUBB #1
       PSHS D
       LDX  #0
       LBSR OFFSET
       TFR X,U	     ; Dest Pointer in U
       LDD  ,S
       SUBD NRLINS-1
       LDX  #0
       LBSR OFFSET   ; Source Pointer in X
       LBSR COPYL    ; Copy one Rasterline
       PULS D
       CMPD ,S
       BNE  REPTI
       PULS D
       ADDD NRLINS-1
       TFR  D,Y
       LDX  #0
       LBSR ERASL     ;clear last line
LTD8   LBRA TD8

DELLIN LDD  YADDR
       CMPD BOTTOM
       BEQ  LTD8      ; if already last line do nothing
       SUBD NRLINS-1  ;die Zeile ist ja schon geloescht
       CLRA
       INCB           ;eine Rasterzeile hoeher anfangen
       INCB
REPTD  SUBB #1
       PSHS D
       LDX  #0
       LBSR OFFSET
       TFR X,U	     ; Source Pointer in U
       LDD  ,S
       ADDD NRLINS-1
       LDX  #0
       LBSR OFFSET   ; Dest Pointer in X
       EXG  X,U
       LBSR COPYL    ; Copy one Rasterline
       PULS D
       CMPD BOTTOM 
       BNE  REPTD
       ADDD NRLINS-1
       TFR  D,Y
       LDX  #0
       LBSR ERASL     ;clear last line
       BRA  LTD8

ORIG   SET  *
       IF   ORIG>(ANFANG+$A1F)
       ERR  "PROGRAM OVERLAPS CHAR.TAB."
       ENDI

       ORG  $FA1F
;
; CHRTBL contains a complete ASCII
; upper and lower case character set
; in a 5 x 7 matrix with lower case
; descenders, making it effectively
; 5 x 9.
;
; hier die deutschen Grossbuchstaben
       FCB    $88,$00,$70,$88,$F8,$88,$88       ;AE
       FCB    $88,$70,$88,$88,$88,$88,$70       ;OE
       FCB    $88,$00,$88,$88,$88,$88,$70       ;UE
; und die deutschen Kleinbuchstaben
       FCB    $50,$00,$70,$08,$78,$88,$78       ;ae
       FCB    $50,$00,$70,$88,$88,$88,$70       ;oe
       FCB    $50,$00,$88,$88,$88,$98,$68       ;ue
       FCB    $70,$88,$B0,$88,$88,$A8,$90       ;SZ
; Zeichen fuer DEL
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
       ERR  "charactertable too long"
       ENDI
       ;OPT  LIS

       ORG  ANFANG+$D40  ;$FCF0-$FD3F free for I/O
;
;
; Boot routine for the FLEX Operating System
; It loads the Bootblock at address $C100
; then it starts the routine
;
BOOT   JSR  CURSAN     ;Cursor on
       LDD  #$C10D     ;Drive 0, Restore 12 ms
       STA  FLDRIV    ;and start timer
       STB  FLCOMM    ;restore
       BSR  RET1       ;Delay
BOOT1  LDB  FLCOMM    ;check for ready
       BPL  BOOT3      ;ready
       CLR  FLDRIV    ;stop timer
       JSR  CURAUS     ;Cursor off
       JMP  HKS
;
BOOT3  BITB #$01
       BNE  BOOT1      ;still busy
       LDD  #$0180     ;Sektor 1, 256 Bytes
       STA  FLSEKT
       LDX  #$C100+$80 ;Store Boots. at $C100
       LDA  #$84
       BSR  RET1
       STA  FLCOMM    ;read one Sektor
BOOT2  LDA  FLDRIV    ;Data request
       BPL  BOOT2      ;no
       LDA  FLDATA    ;get byte
       STA  B,X        ;save it
       INCB
       BVC  BOOT2      ;next byte
       CLRA            ;set Direct Page to 0
       TFR  A,DP
       JMP  $C100      ;read rest of Flex
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
       LDY  #LOETBL
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
       ANDA #%11011111 ;in Grossbu wandeln
       CMPA #'Z'
       PULS A
       BLS  NODEU      ;keins der Sonderzeichen
       BITA #%00100000
       BEQ  GROSS      ;Grossbuchstabe
       SUBA #($7B-$5E) ;$7B -> $5E, ...
GROSS  SUBA #($5B+8)   ;$5B -> -8, ...
       NEGA            ;-8 -> 8, ...
       LDB  MULINS
       MUL
       COMA
       COMB
       ADDD #1         ;von U abziehen
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
; Cursor ganz links, nahe am linken Buchstaben
;
INVTB2 FDB  $FE00,$3F80,$0FE0,$03F8
;
;Cursor rechts, klein
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
       BEQ  TD8       ;vielleicht war's die letzte
       SUBD NRLINS-1  ;die Zeile ist ja schon geloescht
       CLRA
       INCB           ;eine Rasterzeile hoeher anfangen
       INCB
REPT   SUBB #1
       PSHS D,U
       TFR  D,Y
       LDX  #0
       LBSR OFFSET
       LEAU 64,X
       CLRA
       LBSR CLEARS    ;1 Rasterzeile loeschen
       PULS D,U
       CMPD BOTTOM
       BNE  REPT      ;weiter gehts
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
       ERR  "kein Platz fuer MMU"
       ENDI
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
