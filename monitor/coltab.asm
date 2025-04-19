; TTL "COLOR TABLE"
;
; create a color table on the screen
;
; History
;========
;
; 22.09.2000 W. Schwotzer Created
; 04.12.2020 W. Schwotzer Reduced all over cycle time
; 04.12.2020 W. Schwotzer Added two more color tables
; 04.12.2020 W. Schwotzer Leave emulator with ESC key
; 19.04.2025 W. Schwotzer Add boot ROM identifier string

ESC	EQU	$1B	; ESC Character to exit emulator
MMU	EQU	$FFE0   ; I/O base address of memory management unit (MMU)
EMUCMD	EQU	$FD3C   ; flexemu emulator command register
PIA1AD	EQU	$FCF0   ; Pia A data- and
PIA1AC	EQU	$FCF1   ; control register
DIRPAG	EQU	$EF00   ; Direct page base address
SCRNBAS EQU	$0000	; Screen base address
SIZEX	EQU	64	; Screen raster line size in byte
SIZEY	EQU	256	; Number of raster lines on screen
SCRNSIZ	EQU	SIZEX*SIZEY ; Screen memory size in byte
BLOCKH	EQU	SIZEY/8 ; Height of one block (COLTAB2)
STACK	EQU	$F000	; Base addr of stack / program start address

        ORG     DIRPAG
COUNT   RMB     1       ; Base counter to change color bit
COUNT1  RMB     1       ; Counter to change color bit
LCOUNT  RMB     1       ; Inner loop counter (COLTAB2)
BCOUNT  RMB     1       ; Color bar/block counter, 0..63

	ORG	STACK

START	BRA	START1
*
VERS	EQU	4 ; Version number
*
; Color plane codes to switch MMU
PLANES1	FCB	$0C ; Green low
	FCB	$0D ; Blue low
	FCB	$0E ; Red low
	FCB	$04 ; Green high
	FCB	$05 ; Blue high
	FCB	$06 ; Red high
	FCB	$00 ; End marker
*
PLANES2	FCB	$0C ; Green low
	FCB	$04 ; Green high
	FCB	$0D ; Blue low
	FCB	$05 ; Blue high
	FCB	$0E ; Red low
	FCB	$06 ; Red high
	FCB	$00 ; End marker

; Initialize PIA1 for keyboard input
INITPIA	PSHS    A
	CLR	PIA1AC
	CLR	PIA1AD
	LDA	#$3E
	STA	PIA1AC
	PULS	A, PC
*
; Wait for keyboard input character from PIA1
INCH	LDA	PIA1AC
	BPL	INCH
	LDA	PIA1AD
	RTS
*
; Exit emulator
EMUEXIT	LDX	#TEXIT
LPEXIT	LDA	,X+
	STA	EMUCMD
	BNE	LPEXIT
LPEX9   BRA     LPEX9
*
TEXIT   FCC     "EXIT"
	FCB	$00
*
RESET	EQU	*
START1	LDS	#STACK
        LDA     #DIRPAG/$100
        TFR     A,DP
        BSR     INITPIA

; Top quater of screen: Horizontal color bar
	LDX	#SCRNBAS
	LDU	#PLANES1
	LDA	#64
	BSR	COLTAB1
; Next quater of screen: Horizontal color bar with changed color planes
	LDX	#SCRNBAS+(SCRNSIZ/4)
	LDU	#PLANES2
	LDA	#64
	BSR	COLTAB1
; Bottom half of screen: Color blocks, 16 horizontally, 4 vertically
	LDX	#SCRNBAS+(SCRNSIZ/2)
	LDU	#PLANES2
	LDA	#64
	LBSR	COLTAB2
*
END8	BSR	INCH
	CMPA	#ESC
	BNE	END8
	BSR	EMUEXIT

*****************************************
* Color table 1 algorithm               *
*****************************************
LOOP3	BSR	SWTCH
	CLRA
LOOP1	LDB	<COUNT
	STB	<COUNT1
LOOP2	LDB	#(SIZEY / 8 / 4)
LOOP4	STA	0,X
	STA	SIZEX,X
	STA	2*SIZEX,X
	STA	3*SIZEX,X
	STA	4*SIZEX,X
	STA	5*SIZEX,X
	STA	6*SIZEX,X
	STA	7*SIZEX,X
	LEAX	8*SIZEX,X
	DECB
	BNE	LOOP4
	DEC	<BCOUNT
	LEAX	1-(SCRNSIZ / 4),X
	DEC	<COUNT1
	BNE 	LOOP2
	COMA
	TST	<BCOUNT
	BNE	LOOP1
*
CT10	LSR	<COUNT
	TFR	Y,X
	LDB	#64
	STB	<BCOUNT
	LDB	,U+
	BNE	LOOP3
	BRA	SWTCHB
*
COLTAB1 TFR	X,Y
	STA	<COUNT
	BRA	CT10

; Switch MMU
SWTCH	PSHS	B
	TFR	B,A
	STD	MMU
	STD	MMU+2
	PULS	B, PC
*
; Switch back MMU
SWTCHB	PSHS	B
	LDB	#$0F
	TFR	A, B
	STD	MMU
	STD	MMU+2
	PULS	B, PC

*****************************************
* Color table 2 algorithm               *
*****************************************
LOOP7	BSR	SWTCH
	LDX	#SCRNBAS+(SCRNSIZ / 2)
	LDB	#64
	STB	<BCOUNT
	CLRA
LOOP5	LDB	<COUNT
	STB	<COUNT1
LOOP8	LDB     #BLOCKH / 4
	STB	<LCOUNT
	TFR	A,B
LOOP6	STD	0,X
	STD	2,X
	STD	SIZEX+0,X
	STD	SIZEX+2,X
	STD	2*SIZEX+0,X
	STD	2*SIZEX+2,X
	STD	3*SIZEX+0,X
	STD	3*SIZEX+2,X
	LEAX	4*SIZEX,X
	DEC	<LCOUNT
	BNE	LOOP6
	LEAX	4-(BLOCKH*SIZEX),X
	DEC	<BCOUNT
	LDB	<BCOUNT
	BITB	#15
	BNE	NOXKOR
; After 16 Blocks begin a next row of 16 blocks
	LEAX	(BLOCKH-1)*SIZEX,X
NOXKOR	DEC	COUNT1
	BNE 	LOOP8
	COMA
	TST	<BCOUNT
	BNE	LOOP5
*
CT20	LSR	<COUNT
	LDB	,U+
	BNE	LOOP7
	BRA	SWTCHB
*
COLTAB2 STA	<COUNT
	BRA	CT20
*
ROMID	FCC	"EUROCOM COLOR TABLE V1.0"
	FCB	$04

SWI	RTI
IRQ     LBRA	EMUEXIT
*
ORIG   SET  *
       IF   ORIG>=($FFF0)
       ERR  "Program overlaps interrupt vectors!"
       ENDIF

; Interrupt vectors
	ORG	$FFF0

	FDB	0   ; reserved
	FDB	SWI ; SWI3
	FDB	SWI ; SWI2
	FDB	IRQ ; FIRQ
	FDB	IRQ ; IRQ
	FDB	SWI ; SWI
	FDB	IRQ ; NMI
	FDB	RESET ; RESET

	END START

