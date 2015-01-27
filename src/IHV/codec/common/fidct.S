!
! SCCS: @(#)fidct.S 1.2 93/07/02
!
#define HALF 32768

#define T0		%g2
#define T1		%g3

#define RESULT		%i0
#define COEFFLIST	%i1
#define I 		%i2
#define DC		%i3
#define COEFF 		%i4
#define D3 		%i5

#define A0 		%l0
#define A2 		%l1
#define B2 		%l2
#define B0 		%l3
#define C0 		%l4
#define C2 		%l5
#define D2 		%l6
#define D0 		%l7

#define A1 		%o0
#define A3 		%o1
#define B3 		%o2
#define B1 		%o3
#define C1 		%o4
#define C3 		%o5
#define JTBASE 		%o7
#define D1 		%g1

#include <sys/asm_linkage.h>
	    .seg	"text"
	    .proc	4
	    .align	4
	    .global	idct
idct:
	    save	%sp,-SA(MINFRAME),%sp
	!
	! .... first we check if there is a DC coefficient ....
	!
	    ld		[COEFFLIST],COEFF
	    andcc	COEFF,63,T0
	    bne		Lhasac
	    sethi	%hi(HALF),DC


	!
	! .... next, if there was a DC coefficient, we check
	! if it is the only coefficient ....
	!
	    ld		[COEFF],A0
	    ld		[COEFFLIST+4],COEFF
	    add		DC,A0,DC
	    andcc	COEFF,63,T0
	    bne		Lhasac
	    add		COEFFLIST,4,COEFFLIST

	!
	! .... for a DC only block we exit early ....
	!
	    sethi	%hi(0xffff0000),I
	    srl		DC,16,A0
	    and		DC,I,DC
	    add		A0,DC,A0
	    mov		A0,A2
	    std		A0,[RESULT]
	    std		A0,[RESULT+8]
	    std		A0,[RESULT+16]
	    std		A0,[RESULT+24]
	    std		A0,[RESULT+32]
	    std		A0,[RESULT+40]
	    std		A0,[RESULT+48]
	    std		A0,[RESULT+56]
	    std		A0,[RESULT+64]
	    std		A0,[RESULT+72]
	    std		A0,[RESULT+80]
	    std		A0,[RESULT+88]
	    std		A0,[RESULT+96]
	    std		A0,[RESULT+104]
	    std		A0,[RESULT+112]
	    std		A0,[RESULT+120]
	    ret
	    restore	%sp,SA(MINFRAME),%sp
	    
Lhasac:
	    call	Lskip		! squirrel away PC in JTBASE
	    sll		T0,2,T0
Lskip:
	    add		JTBASE,(Ljtabr0 - Lhasac),JTBASE
	    mov		DC,A0
	    mov		DC,A1
	    mov		DC,A2
	    mov		DC,A3
	    mov		0,B0
	    mov		0,B1
	    mov		0,B2
	    mov		0,B3
	    mov		0,C0
	    mov		0,C1
	    mov		0,C2
	    mov		0,C3
	    mov		0,D0
	    mov		0,D1
	    mov		0,D2
	    mov		0,D3
	    ld		[JTBASE+T0],T0
	    andn	COEFF,63,COEFF
	    jmp		JTBASE+T0
	    mov		4,I
Ljtabr0:
	    .word	(Lbfly - Ljtabr0)
	    .word	(Lcase1r0 - Ljtabr0)
	    .word	(Lcase2r0 - Ljtabr0)
	    .word	(Lcase3r0 - Ljtabr0)
	    .word	(Lcase4r0 - Ljtabr0)
	    .word	(Lcase5r0 - Ljtabr0)
	    .word	(Lcase6r0 - Ljtabr0)
	    .word	(Lcase7r0 - Ljtabr0)
	    .word	(Lcase8r0 - Ljtabr0)
	    .word	(Lcase9r0 - Ljtabr0)
	    .word	(Lcase10r0 - Ljtabr0)
	    .word	(Lcase11r0 - Ljtabr0)
	    .word	(Lcase12r0 - Ljtabr0)
	    .word	(Lcase13r0 - Ljtabr0)
	    .word	(Lcase14r0 - Ljtabr0)
	    .word	(Lcase15r0 - Ljtabr0)
	    .word	(Lcase16r0 - Ljtabr0)
	    .word	(Lcase17r0 - Ljtabr0)
	    .word	(Lcase18r0 - Ljtabr0)
	    .word	(Lcase19r0 - Ljtabr0)
	    .word	(Lcase20r0 - Ljtabr0)
	    .word	(Lcase21r0 - Ljtabr0)
	    .word	(Lcase22r0 - Ljtabr0)
	    .word	(Lcase23r0 - Ljtabr0)
	    .word	(Lcase24r0 - Ljtabr0)
	    .word	(Lcase25r0 - Ljtabr0)
	    .word	(Lcase26r0 - Ljtabr0)
	    .word	(Lcase27r0 - Ljtabr0)
	    .word	(Lcase28r0 - Ljtabr0)
	    .word	(Lcase29r0 - Ljtabr0)
	    .word	(Lcase30r0 - Ljtabr0)
	    .word	(Lcase31r0 - Ljtabr0)
	    .word	(Lcase32r0 - Ljtabr0)
	    .word	(Lcase33r0 - Ljtabr0)
	    .word	(Lcase34r0 - Ljtabr0)
	    .word	(Lcase35r0 - Ljtabr0)
	    .word	(Lcase36r0 - Ljtabr0)
	    .word	(Lcase37r0 - Ljtabr0)
	    .word	(Lcase38r0 - Ljtabr0)
	    .word	(Lcase39r0 - Ljtabr0)
	    .word	(Lcase40r0 - Ljtabr0)
	    .word	(Lcase41r0 - Ljtabr0)
	    .word	(Lcase42r0 - Ljtabr0)
	    .word	(Lcase43r0 - Ljtabr0)
	    .word	(Lcase44r0 - Ljtabr0)
	    .word	(Lcase45r0 - Ljtabr0)
	    .word	(Lcase46r0 - Ljtabr0)
	    .word	(Lcase47r0 - Ljtabr0)
	    .word	(Lcase48r0 - Ljtabr0)
	    .word	(Lcase49r0 - Ljtabr0)
	    .word	(Lcase50r0 - Ljtabr0)
	    .word	(Lcase51r0 - Ljtabr0)
	    .word	(Lcase52r0 - Ljtabr0)
	    .word	(Lcase53r0 - Ljtabr0)
	    .word	(Lcase54r0 - Ljtabr0)
	    .word	(Lcase55r0 - Ljtabr0)
	    .word	(Lcase56r0 - Ljtabr0)
	    .word	(Lcase57r0 - Ljtabr0)
	    .word	(Lcase58r0 - Ljtabr0)
	    .word	(Lcase59r0 - Ljtabr0)
	    .word	(Lcase60r0 - Ljtabr0)
	    .word	(Lcase61r0 - Ljtabr0)
	    .word	(Lcase62r0 - Ljtabr0)
	    .word	(Lcase63r0 - Ljtabr0)
	    .word	(Lstorer0 - Ljtabr0)

Lstorer0:
    	    std		A0,[RESULT + 0]
	    add		JTBASE,(Ljtabr1 - Ljtabr0),JTBASE
	    mov		DC,A0
    	    ld		[COEFFLIST],COEFF
	    mov		DC,A1
	    mov		DC,A2
	    mov		DC,A3
    	    std		B2,[RESULT + 8]
	    mov		0,B0
	    mov		0,B1
	    mov		0,B2
	    mov		0,B3
    	    std		C0,[RESULT + 112]
	    mov		0,C0
	    mov		0,C1
	    mov		0,C2
	    mov		0,C3
    	    std		D2,[RESULT + 120]
	    and		COEFF,63,T0
	    mov		0,D0
	    sll		T0,2,T0
	    mov		0,D1
	    mov		0,D2
	    mov		0,D3
	    ld		[JTBASE+T0],T0
	    andn	COEFF,63,COEFF
	    jmp		JTBASE+T0
	    mov		4,I
Ljtabr1:
	    .word	(Lbfly - Ljtabr1)
	    .word	(Lcase1r1 - Ljtabr1)
	    .word	(Lcase2r1 - Ljtabr1)
	    .word	(Lcase3r1 - Ljtabr1)
	    .word	(Lcase4r1 - Ljtabr1)
	    .word	(Lcase5r1 - Ljtabr1)
	    .word	(Lcase6r1 - Ljtabr1)
	    .word	(Lcase7r1 - Ljtabr1)
	    .word	(Lcase8r1 - Ljtabr1)
	    .word	(Lcase9r1 - Ljtabr1)
	    .word	(Lcase10r1 - Ljtabr1)
	    .word	(Lcase11r1 - Ljtabr1)
	    .word	(Lcase12r1 - Ljtabr1)
	    .word	(Lcase13r1 - Ljtabr1)
	    .word	(Lcase14r1 - Ljtabr1)
	    .word	(Lcase15r1 - Ljtabr1)
	    .word	(Lcase16r1 - Ljtabr1)
	    .word	(Lcase17r1 - Ljtabr1)
	    .word	(Lcase18r1 - Ljtabr1)
	    .word	(Lcase19r1 - Ljtabr1)
	    .word	(Lcase20r1 - Ljtabr1)
	    .word	(Lcase21r1 - Ljtabr1)
	    .word	(Lcase22r1 - Ljtabr1)
	    .word	(Lcase23r1 - Ljtabr1)
	    .word	(Lcase24r1 - Ljtabr1)
	    .word	(Lcase25r1 - Ljtabr1)
	    .word	(Lcase26r1 - Ljtabr1)
	    .word	(Lcase27r1 - Ljtabr1)
	    .word	(Lcase28r1 - Ljtabr1)
	    .word	(Lcase29r1 - Ljtabr1)
	    .word	(Lcase30r1 - Ljtabr1)
	    .word	(Lcase31r1 - Ljtabr1)
	    .word	(Lcase32r1 - Ljtabr1)
	    .word	(Lcase33r1 - Ljtabr1)
	    .word	(Lcase34r1 - Ljtabr1)
	    .word	(Lcase35r1 - Ljtabr1)
	    .word	(Lcase36r1 - Ljtabr1)
	    .word	(Lcase37r1 - Ljtabr1)
	    .word	(Lcase38r1 - Ljtabr1)
	    .word	(Lcase39r1 - Ljtabr1)
	    .word	(Lcase40r1 - Ljtabr1)
	    .word	(Lcase41r1 - Ljtabr1)
	    .word	(Lcase42r1 - Ljtabr1)
	    .word	(Lcase43r1 - Ljtabr1)
	    .word	(Lcase44r1 - Ljtabr1)
	    .word	(Lcase45r1 - Ljtabr1)
	    .word	(Lcase46r1 - Ljtabr1)
	    .word	(Lcase47r1 - Ljtabr1)
	    .word	(Lcase48r1 - Ljtabr1)
	    .word	(Lcase49r1 - Ljtabr1)
	    .word	(Lcase50r1 - Ljtabr1)
	    .word	(Lcase51r1 - Ljtabr1)
	    .word	(Lcase52r1 - Ljtabr1)
	    .word	(Lcase53r1 - Ljtabr1)
	    .word	(Lcase54r1 - Ljtabr1)
	    .word	(Lcase55r1 - Ljtabr1)
	    .word	(Lcase56r1 - Ljtabr1)
	    .word	(Lcase57r1 - Ljtabr1)
	    .word	(Lcase58r1 - Ljtabr1)
	    .word	(Lcase59r1 - Ljtabr1)
	    .word	(Lcase60r1 - Ljtabr1)
	    .word	(Lcase61r1 - Ljtabr1)
	    .word	(Lcase62r1 - Ljtabr1)
	    .word	(Lcase63r1 - Ljtabr1)
	    .word	(Lstorer1 - Ljtabr1)

Lstorer1:
    	    std		A0,[RESULT + 16]
	    add		JTBASE,(Ljtabr2 - Ljtabr1),JTBASE
	    mov		DC,A0
	    ld		[COEFFLIST],COEFF
	    mov		DC,A1
	    mov		DC,A2
	    mov		DC,A3
    	    std		B2,[RESULT + 24]
	    mov		0,B0
	    mov		0,B1
	    mov		0,B2
	    mov		0,B3
    	    std		C0,[RESULT + 96]
	    mov		0,C0
	    mov		0,C1
	    mov		0,C2
	    mov		0,C3
    	    std		D2,[RESULT + 104]
	    and		COEFF,63,T0
	    mov		0,D0
	    sll		T0,2,T0
	    mov		0,D1
	    mov		0,D2
	    mov		0,D3
	    ld		[JTBASE+T0],T0
	    andn	COEFF,63,COEFF
	    jmp		JTBASE+T0
	    mov		4,I
Ljtabr2:
	    .word	(Lbfly - Ljtabr2)
	    .word	(Lcase1r2 - Ljtabr2)
	    .word	(Lcase2r2 - Ljtabr2)
	    .word	(Lcase3r2 - Ljtabr2)
	    .word	(Lcase4r2 - Ljtabr2)
	    .word	(Lcase5r2 - Ljtabr2)
	    .word	(Lcase6r2 - Ljtabr2)
	    .word	(Lcase7r2 - Ljtabr2)
	    .word	(Lcase8r2 - Ljtabr2)
	    .word	(Lcase9r2 - Ljtabr2)
	    .word	(Lcase10r2 - Ljtabr2)
	    .word	(Lcase11r2 - Ljtabr2)
	    .word	(Lcase12r2 - Ljtabr2)
	    .word	(Lcase13r2 - Ljtabr2)
	    .word	(Lcase14r2 - Ljtabr2)
	    .word	(Lcase15r2 - Ljtabr2)
	    .word	(Lcase16r2 - Ljtabr2)
	    .word	(Lcase17r2 - Ljtabr2)
	    .word	(Lcase18r2 - Ljtabr2)
	    .word	(Lcase19r2 - Ljtabr2)
	    .word	(Lcase20r2 - Ljtabr2)
	    .word	(Lcase21r2 - Ljtabr2)
	    .word	(Lcase22r2 - Ljtabr2)
	    .word	(Lcase23r2 - Ljtabr2)
	    .word	(Lcase24r2 - Ljtabr2)
	    .word	(Lcase25r2 - Ljtabr2)
	    .word	(Lcase26r2 - Ljtabr2)
	    .word	(Lcase27r2 - Ljtabr2)
	    .word	(Lcase28r2 - Ljtabr2)
	    .word	(Lcase29r2 - Ljtabr2)
	    .word	(Lcase30r2 - Ljtabr2)
	    .word	(Lcase31r2 - Ljtabr2)
	    .word	(Lcase32r2 - Ljtabr2)
	    .word	(Lcase33r2 - Ljtabr2)
	    .word	(Lcase34r2 - Ljtabr2)
	    .word	(Lcase35r2 - Ljtabr2)
	    .word	(Lcase36r2 - Ljtabr2)
	    .word	(Lcase37r2 - Ljtabr2)
	    .word	(Lcase38r2 - Ljtabr2)
	    .word	(Lcase39r2 - Ljtabr2)
	    .word	(Lcase40r2 - Ljtabr2)
	    .word	(Lcase41r2 - Ljtabr2)
	    .word	(Lcase42r2 - Ljtabr2)
	    .word	(Lcase43r2 - Ljtabr2)
	    .word	(Lcase44r2 - Ljtabr2)
	    .word	(Lcase45r2 - Ljtabr2)
	    .word	(Lcase46r2 - Ljtabr2)
	    .word	(Lcase47r2 - Ljtabr2)
	    .word	(Lcase48r2 - Ljtabr2)
	    .word	(Lcase49r2 - Ljtabr2)
	    .word	(Lcase50r2 - Ljtabr2)
	    .word	(Lcase51r2 - Ljtabr2)
	    .word	(Lcase52r2 - Ljtabr2)
	    .word	(Lcase53r2 - Ljtabr2)
	    .word	(Lcase54r2 - Ljtabr2)
	    .word	(Lcase55r2 - Ljtabr2)
	    .word	(Lcase56r2 - Ljtabr2)
	    .word	(Lcase57r2 - Ljtabr2)
	    .word	(Lcase58r2 - Ljtabr2)
	    .word	(Lcase59r2 - Ljtabr2)
	    .word	(Lcase60r2 - Ljtabr2)
	    .word	(Lcase61r2 - Ljtabr2)
	    .word	(Lcase62r2 - Ljtabr2)
	    .word	(Lcase63r2 - Ljtabr2)
	    .word	(Lstorer2 - Ljtabr2)

Lstorer2:
    	    std		A0,[RESULT + 32]
	    add		JTBASE,(Ljtabr3 - Ljtabr2),JTBASE
	    mov		DC,A0
	    ld		[COEFFLIST],COEFF
	    mov		DC,A1
	    mov		DC,A2
	    mov		DC,A3
    	    std		B2,[RESULT + 40]
	    mov		0,B0
	    mov		0,B1
	    mov		0,B2
	    mov		0,B3
    	    std		C0,[RESULT + 80]
	    mov		0,C0
	    mov		0,C1
	    mov		0,C2
	    mov		0,C3
    	    std		D2,[RESULT + 88]
	    and		COEFF,63,T0
	    mov		0,D0
	    sll		T0,2,T0
	    mov		0,D1
	    mov		0,D2
	    mov		0,D3
	    ld		[JTBASE+T0],T0
	    andn	COEFF,63,COEFF
	    jmp		JTBASE+T0
	    mov		4,I
Ljtabr3:
	    .word	(Lbfly - Ljtabr3)
	    .word	(Lcase1r3 - Ljtabr3)
	    .word	(Lcase2r3 - Ljtabr3)
	    .word	(Lcase3r3 - Ljtabr3)
	    .word	(Lcase4r3 - Ljtabr3)
	    .word	(Lcase5r3 - Ljtabr3)
	    .word	(Lcase6r3 - Ljtabr3)
	    .word	(Lcase7r3 - Ljtabr3)
	    .word	(Lcase8r3 - Ljtabr3)
	    .word	(Lcase9r3 - Ljtabr3)
	    .word	(Lcase10r3 - Ljtabr3)
	    .word	(Lcase11r3 - Ljtabr3)
	    .word	(Lcase12r3 - Ljtabr3)
	    .word	(Lcase13r3 - Ljtabr3)
	    .word	(Lcase14r3 - Ljtabr3)
	    .word	(Lcase15r3 - Ljtabr3)
	    .word	(Lcase16r3 - Ljtabr3)
	    .word	(Lcase17r3 - Ljtabr3)
	    .word	(Lcase18r3 - Ljtabr3)
	    .word	(Lcase19r3 - Ljtabr3)
	    .word	(Lcase20r3 - Ljtabr3)
	    .word	(Lcase21r3 - Ljtabr3)
	    .word	(Lcase22r3 - Ljtabr3)
	    .word	(Lcase23r3 - Ljtabr3)
	    .word	(Lcase24r3 - Ljtabr3)
	    .word	(Lcase25r3 - Ljtabr3)
	    .word	(Lcase26r3 - Ljtabr3)
	    .word	(Lcase27r3 - Ljtabr3)
	    .word	(Lcase28r3 - Ljtabr3)
	    .word	(Lcase29r3 - Ljtabr3)
	    .word	(Lcase30r3 - Ljtabr3)
	    .word	(Lcase31r3 - Ljtabr3)
	    .word	(Lcase32r3 - Ljtabr3)
	    .word	(Lcase33r3 - Ljtabr3)
	    .word	(Lcase34r3 - Ljtabr3)
	    .word	(Lcase35r3 - Ljtabr3)
	    .word	(Lcase36r3 - Ljtabr3)
	    .word	(Lcase37r3 - Ljtabr3)
	    .word	(Lcase38r3 - Ljtabr3)
	    .word	(Lcase39r3 - Ljtabr3)
	    .word	(Lcase40r3 - Ljtabr3)
	    .word	(Lcase41r3 - Ljtabr3)
	    .word	(Lcase42r3 - Ljtabr3)
	    .word	(Lcase43r3 - Ljtabr3)
	    .word	(Lcase44r3 - Ljtabr3)
	    .word	(Lcase45r3 - Ljtabr3)
	    .word	(Lcase46r3 - Ljtabr3)
	    .word	(Lcase47r3 - Ljtabr3)
	    .word	(Lcase48r3 - Ljtabr3)
	    .word	(Lcase49r3 - Ljtabr3)
	    .word	(Lcase50r3 - Ljtabr3)
	    .word	(Lcase51r3 - Ljtabr3)
	    .word	(Lcase52r3 - Ljtabr3)
	    .word	(Lcase53r3 - Ljtabr3)
	    .word	(Lcase54r3 - Ljtabr3)
	    .word	(Lcase55r3 - Ljtabr3)
	    .word	(Lcase56r3 - Ljtabr3)
	    .word	(Lcase57r3 - Ljtabr3)
	    .word	(Lcase58r3 - Ljtabr3)
	    .word	(Lcase59r3 - Ljtabr3)
	    .word	(Lcase60r3 - Ljtabr3)
	    .word	(Lcase61r3 - Ljtabr3)
	    .word	(Lcase62r3 - Ljtabr3)
	    .word	(Lcase63r3 - Ljtabr3)
	    .word	(Lstorer3 - Ljtabr3)

Lstorer3:
    	    std		A0,[RESULT + 48]
    	    std		B2,[RESULT + 56]
    	    std		C0,[RESULT + 64]
    	    std		D2,[RESULT + 72]
	    ret
	    restore	%sp,SA(MINFRAME),%sp

Lbfly:
	    sethi	%hi(0xffff0000),I
    	    add		A0,B0,T0
    	    sub		A0,B0,T1
    	    add		C0,D0,A0
    	    sub		C0,D0,B0
	    sub		T0,A0,C0
	    and		C0,I,C0
	    add		T0,A0,A0
	    and		A0,I,A0
	    sub		T1,B0,D0
    	    srl		D0,16,D0
	    add		T1,B0,B0
    	    srl		B0,16,B0

    	    add		A1,B1,T0
    	    sub		A1,B1,T1
    	    add		C1,D1,A1
    	    sub		C1,D1,B1
	    sub		T0,A1,C1
    	    srl		C1,16,C1
	    add		T0,A1,A1
    	    srl		A1,16,A1
	    sub		T1,B1,D1
	    and		D1,I,D1
	    add		T1,B1,B1
	    and		B1,I,B1

    	    add		A2,B2,T0
    	    sub		A2,B2,T1
    	    add		C2,D2,A2
    	    sub		C2,D2,B2
	    sub		T0,A2,C2
	    and		C2,I,C2
	    add		T0,A2,A2
	    and		A2,I,A2
	    sub		T1,B2,D2
    	    srl		D2,16,D2
	    add		T1,B2,B2
    	    srl		B2,16,B2

    	    add		A3,B3,T0
    	    sub		A3,B3,T1
    	    add		C3,D3,A3
    	    sub		C3,D3,B3
	    sub		T0,A3,C3
    	    srl		C3,16,C3
	    add		T0,A3,A3
    	    srl		A3,16,A3
	    sub		T1,B3,D3
	    and		D3,I,D3
	    add		T1,B3,B3
	    and		B3,I,B3

	    ld		[JTBASE+256],T0
	    add		A0,A1,A0
	    add		A2,A3,A2
	    add		B2,B3,B2
	    add		B0,B1,B0
	    add		C0,C1,C0
    	    add		C2,C3,C2
    	    add		D2,D3,D2
	    jmp		JTBASE+T0
	    add		D0,D1,D0

Lcase1r0:
Lcase1r1:
Lcase1r2:
Lcase1r3:
	    ldd		[COEFF + 4*4],T0
	    add		B0,T0,B0
	    add		B1,T1,B1
	    ldd		[COEFF + 4*6],T0
	    ld		[COEFFLIST+I],COEFF
	    add		B2,T0,B2
	    and		COEFF,63,T0
	    sll		T0,2,T0
	    add		B3,T1,B3
	    ld		[JTBASE+T0],T0
	    andn	COEFF,63,COEFF
	    jmp		JTBASE+T0
	    inc		4,I

Lcase2r0:
	    ld		[COEFF + 4*4],T0
	    ld		[COEFFLIST+I],COEFF
	    add		C0,T0,C0
	    and		COEFF,63,T1
	    add		C1,T0,C1
	    sll		T1,2,T1
	    add		C2,T0,C2
	    add		C3,T0,C3
	    ld		[JTBASE+T1],T1
	    andn	COEFF,63,COEFF
	    jmp		JTBASE+T1
	    inc		4,I

Lcase2r1:
	    ld		[COEFF + 4*5],T0
	    ld		[COEFFLIST+I],COEFF
	    add		C0,T0,C0
	    and		COEFF,63,T1
	    add		C1,T0,C1
	    sll		T1,2,T1
	    add		C2,T0,C2
	    add		C3,T0,C3
	    ld		[JTBASE+T1],T1
	    andn	COEFF,63,COEFF
	    jmp		JTBASE+T1
	    inc		4,I

Lcase2r2:
	    ld		[COEFF + 4*6],T0
	    ld		[COEFFLIST+I],COEFF
	    add		C0,T0,C0
	    and		COEFF,63,T1
	    add		C1,T0,C1
	    sll		T1,2,T1
	    add		C2,T0,C2
	    add		C3,T0,C3
	    ld		[JTBASE+T1],T1
	    andn	COEFF,63,COEFF
	    jmp		JTBASE+T1
	    inc		4,I

Lcase2r3:
	    ld		[COEFF + 4*7],T0
	    ld		[COEFFLIST+I],COEFF
	    add		C0,T0,C0
	    and		COEFF,63,T1
	    add		C1,T0,C1
	    sll		T1,2,T1
	    add		C2,T0,C2
	    add		C3,T0,C3
	    ld		[JTBASE+T1],T1
	    andn	COEFF,63,COEFF
	    jmp		JTBASE+T1
	    inc		4,I

Lcase3r0:
	    ld		[COEFF + 4*2],T0
	    ld		[COEFFLIST+I],COEFF
	    add		A0,T0,A0
	    and		COEFF,63,T1
	    add		A1,T0,A1
	    sll		T1,2,T1
	    add		A2,T0,A2
	    add		A3,T0,A3
	    ld		[JTBASE+T1],T1
	    andn	COEFF,63,COEFF
	    jmp		JTBASE+T1
	    inc		4,I

Lcase3r1:
	    ld		[COEFF + 4*3],T0
	    ld		[COEFFLIST+I],COEFF
	    add		A0,T0,A0
	    and		COEFF,63,T1
	    add		A1,T0,A1
	    sll		T1,2,T1
	    add		A2,T0,A2
	    add		A3,T0,A3
	    ld		[JTBASE+T1],T1
	    andn	COEFF,63,COEFF
	    jmp		JTBASE+T1
	    inc		4,I

Lcase3r2:
	    ld		[COEFF + 4*3],T0
	    ld		[COEFFLIST+I],COEFF
	    sub		A0,T0,A0
	    and		COEFF,63,T1
	    sub		A1,T0,A1
	    sll		T1,2,T1
	    sub		A2,T0,A2
	    sub		A3,T0,A3
	    ld		[JTBASE+T1],T1
	    andn	COEFF,63,COEFF
	    jmp		JTBASE+T1
	    inc		4,I

Lcase3r3:
	    ld		[COEFF + 4*2],T0
	    ld		[COEFFLIST+I],COEFF
	    sub		A0,T0,A0
	    and		COEFF,63,T1
	    sub		A1,T0,A1
	    sll		T1,2,T1
	    sub		A2,T0,A2
	    sub		A3,T0,A3
	    ld		[JTBASE+T1],T1
	    andn	COEFF,63,COEFF
	    jmp		JTBASE+T1
	    inc		4,I

Lcase4r0:
	    ldd		[COEFF + 4*20],T0
	    add		D0,T0,D0
	    add		D1,T1,D1
	    ldd		[COEFF + 4*22],T0
	    ld		[COEFFLIST+I],COEFF
	    add		D2,T0,D2
	    and		COEFF,63,T0
	    sll		T0,2,T0
	    add		D3,T1,D3
	    ld		[JTBASE+T0],T0
	    andn	COEFF,63,COEFF
	    jmp		JTBASE+T0
	    inc		4,I

Lcase4r1:
	    ld		[COEFF + 4*21],T0
	    add		D0,T0,D0
	    ldd		[COEFF + 4*24],T0
	    add		D1,T0,D1
	    add		D2,T1,D2
	    ld		[COEFF + 4*26],T0
	    ld		[COEFFLIST+I],COEFF
	    add		D3,T0,D3
	    and		COEFF,63,T0
	    sll		T0,2,T0
	    ld		[JTBASE+T0],T0
	    andn	COEFF,63,COEFF
	    jmp		JTBASE+T0
	    inc		4,I

Lcase4r2:
	    ld		[COEFF + 4*22],T0
	    ld		[COEFF + 4*25],T1
	    add		D0,T0,D0
	    add		D1,T1,D1
	    ld		[COEFF + 4*27],T0
	    ld		[COEFF + 4*28],T1
	    ld		[COEFFLIST+I],COEFF
	    add		D2,T0,D2
	    and		COEFF,63,T0
	    sll		T0,2,T0
	    add		D3,T1,D3
	    ld		[JTBASE+T0],T0
	    andn	COEFF,63,COEFF
	    jmp		JTBASE+T0
	    inc		4,I
Lcase4r3:
	    ld		[COEFF + 4*23],T0
	    ld		[COEFF + 4*26],T1
	    add		D0,T0,D0
	    add		D1,T1,D1
	    ldd		[COEFF + 4*28],T0
	    ld		[COEFFLIST+I],COEFF
	    add		D2,T0,D2
	    and		COEFF,63,T0
	    sll		T0,2,T0
	    add		D3,T1,D3
	    ld		[JTBASE+T0],T0
	    andn	COEFF,63,COEFF
	    jmp		JTBASE+T0
	    inc		4,I

Lcase5r0:
Lcase5r1:
Lcase5r2:
Lcase5r3:
	    ldd		[COEFF + 4*2],T0
	    ld		[COEFFLIST+I],COEFF
	    add		A0,T0,A0
	    sub		A3,T0,A3
	    add		A1,T1,A1
	    and		COEFF,63,T0
	    sll		T0,2,T0
	    sub		A2,T1,A2
	    ld		[JTBASE+T0],T0
	    andn	COEFF,63,COEFF
	    jmp		JTBASE+T0
	    inc		4,I

Lcase6r0:
Lcase6r1:
Lcase6r2:
Lcase6r3:
	    ldd		[COEFF + 4*4],T0
	    sub		B2,T0,B2
	    add		B0,T1,B0
	    ldd		[COEFF + 4*6],T0
	    ld		[COEFFLIST+I],COEFF
	    sub		B3,T0,B3
	    and		COEFF,63,T0
	    sll		T0,2,T0
	    sub		B1,T1,B1
	    ld		[JTBASE+T0],T0
	    andn	COEFF,63,COEFF
	    jmp		JTBASE+T0
	    inc		4,I

Lcase7r0:
	    ld		[COEFF + 4*8],T0
	    ld		[COEFF + 4*12],T1
	    add		C0,T0,C0
	    sub		C3,T0,C3
	    ld		[COEFFLIST+I],COEFF
	    add		C1,T1,C1
	    and		COEFF,63,T0
	    sll		T0,2,T0
	    sub		C2,T1,C2
	    ld		[JTBASE+T0],T0
	    andn	COEFF,63,COEFF
	    jmp		JTBASE+T0
	    inc		4,I

Lcase7r1:
	    ld		[COEFF + 4*9],T0
	    ld		[COEFF + 4*13],T1
	    add		C0,T0,C0
	    sub		C3,T0,C3
	    ld		[COEFFLIST+I],COEFF
	    add		C1,T1,C1
	    and		COEFF,63,T0
	    sll		T0,2,T0
	    sub		C2,T1,C2
	    ld		[JTBASE+T0],T0
	    andn	COEFF,63,COEFF
	    jmp		JTBASE+T0
	    inc		4,I

Lcase7r2:
	    ld		[COEFF + 4*10],T0
	    ld		[COEFF + 4*14],T1
	    add		C0,T0,C0
	    sub		C3,T0,C3
	    ld		[COEFFLIST+I],COEFF
	    add		C1,T1,C1
	    and		COEFF,63,T0
	    sll		T0,2,T0
	    sub		C2,T1,C2
	    ld		[JTBASE+T0],T0
	    andn	COEFF,63,COEFF
	    jmp		JTBASE+T0
	    inc		4,I

Lcase7r3:
	    ld		[COEFF + 4*11],T0
	    ld		[COEFF + 4*15],T1
	    add		C0,T0,C0
	    sub		C3,T0,C3
	    ld		[COEFFLIST+I],COEFF
	    add		C1,T1,C1
	    and		COEFF,63,T0
	    sll		T0,2,T0
	    sub		C2,T1,C2
	    ld		[JTBASE+T0],T0
	    andn	COEFF,63,COEFF
	    jmp		JTBASE+T0
	    inc		4,I

Lcase8r0:
	    ldd		[COEFF + 4*8],T0
	    add		B0,T0,B0
	    add		B1,T1,B1
	    ldd		[COEFF + 4*10],T0
	    ld		[COEFFLIST+I],COEFF
	    add		B2,T0,B2
	    and		COEFF,63,T0
	    sll		T0,2,T0
	    add		B3,T1,B3
	    ld		[JTBASE+T0],T0
	    andn	COEFF,63,COEFF
	    jmp		JTBASE+T0
	    inc		4,I

Lcase8r1:
	    ldd		[COEFF + 4*12],T0
	    add		B0,T0,B0
	    add		B1,T1,B1
	    ldd		[COEFF + 4*14],T0
	    ld		[COEFFLIST+I],COEFF
	    add		B2,T0,B2
	    and		COEFF,63,T0
	    sll		T0,2,T0
	    add		B3,T1,B3
	    ld		[JTBASE+T0],T0
	    andn	COEFF,63,COEFF
	    jmp		JTBASE+T0
	    inc		4,I

Lcase8r2:
	    ldd		[COEFF + 4*12],T0
	    sub		B0,T0,B0
	    sub		B1,T1,B1
	    ldd		[COEFF + 4*14],T0
	    ld		[COEFFLIST+I],COEFF
	    sub		B2,T0,B2
	    and		COEFF,63,T0
	    sll		T0,2,T0
	    sub		B3,T1,B3
	    ld		[JTBASE+T0],T0
	    andn	COEFF,63,COEFF
	    jmp		JTBASE+T0
	    inc		4,I

Lcase8r3:
	    ldd		[COEFF + 4*8],T0
	    sub		B0,T0,B0
	    sub		B1,T1,B1
	    ldd		[COEFF + 4*10],T0
	    ld		[COEFFLIST+I],COEFF
	    sub		B2,T0,B2
	    and		COEFF,63,T0
	    sll		T0,2,T0
	    sub		B3,T1,B3
	    ld		[JTBASE+T0],T0
	    andn	COEFF,63,COEFF
	    jmp		JTBASE+T0
	    inc		4,I

Lcase9r0:
	    ld		[COEFF + 4*5],T0
	    ld		[COEFFLIST+I],COEFF
	    add		C0,T0,C0
	    and		COEFF,63,T1
	    add		C1,T0,C1
	    sll		T1,2,T1
	    add		C2,T0,C2
	    add		C3,T0,C3
	    ld		[JTBASE+T1],T1
	    andn	COEFF,63,COEFF
	    jmp		JTBASE+T1
	    inc		4,I

Lcase9r1:
	    ld		[COEFF + 4*7],T0
	    ld		[COEFFLIST+I],COEFF
	    sub		C0,T0,C0
	    and		COEFF,63,T1
	    sub		C1,T0,C1
	    sll		T1,2,T1
	    sub		C2,T0,C2
	    sub		C3,T0,C3
	    ld		[JTBASE+T1],T1
	    andn	COEFF,63,COEFF
	    jmp		JTBASE+T1
	    inc		4,I

Lcase9r2:
	    ld		[COEFF + 4*4],T0
	    ld		[COEFFLIST+I],COEFF
	    sub		C0,T0,C0
	    and		COEFF,63,T1
	    sub		C1,T0,C1
	    sll		T1,2,T1
	    sub		C2,T0,C2
	    sub		C3,T0,C3
	    ld		[JTBASE+T1],T1
	    andn	COEFF,63,COEFF
	    jmp		JTBASE+T1
	    inc		4,I

Lcase9r3:
	    ld		[COEFF + 4*6],T0
	    ld		[COEFFLIST+I],COEFF
	    sub		C0,T0,C0
	    and		COEFF,63,T1
	    sub		C1,T0,C1
	    sll		T1,2,T1
	    sub		C2,T0,C2
	    sub		C3,T0,C3
	    ld		[JTBASE+T1],T1
	    andn	COEFF,63,COEFF
	    jmp		JTBASE+T1
	    inc		4,I

Lcase10r0:
Lcase10r3:
	    ld		[COEFF + 4*0],T0
	    ld		[COEFFLIST+I],COEFF
	    add		A0,T0,A0
	    and		COEFF,63,T1
	    add		A1,T0,A1
	    sll		T1,2,T1
	    add		A2,T0,A2
	    add		A3,T0,A3
	    ld		[JTBASE+T1],T1
	    andn	COEFF,63,COEFF
	    jmp		JTBASE+T1
	    inc		4,I

Lcase10r1:
Lcase10r2:
	    ld		[COEFF + 4*0],T0
	    ld		[COEFFLIST+I],COEFF
	    sub		A0,T0,A0
	    and		COEFF,63,T1
	    sub		A1,T0,A1
	    sll		T1,2,T1
	    sub		A2,T0,A2
	    sub		A3,T0,A3
	    ld		[JTBASE+T1],T1
	    andn	COEFF,63,COEFF
	    jmp		JTBASE+T1
	    inc		4,I

Lcase11r0:
	    ld		[COEFF + 4*21],T0
	    add		D0,T0,D0
	    ldd		[COEFF + 4*24],T0
	    add		D1,T0,D1
	    add		D2,T1,D2
	    ld		[COEFF + 4*26],T0
	    ld		[COEFFLIST+I],COEFF
	    add		D3,T0,D3
	    and		COEFF,63,T0
	    sll		T0,2,T0
	    ld		[JTBASE+T0],T0
	    andn	COEFF,63,COEFF
	    jmp		JTBASE+T0
	    inc		4,I

Lcase11r1:
	    ld		[COEFF + 4*23],T0
	    ld		[COEFF + 4*26],T1
	    sub		D0,T0,D0
	    sub		D1,T1,D1
	    ldd		[COEFF + 4*28],T0
	    ld		[COEFFLIST+I],COEFF
	    sub		D2,T0,D2
	    and		COEFF,63,T0
	    sll		T0,2,T0
	    sub		D3,T1,D3
	    ld		[JTBASE+T0],T0
	    andn	COEFF,63,COEFF
	    jmp		JTBASE+T0
	    inc		4,I

Lcase11r2:
	    ldd		[COEFF + 4*20],T0
	    sub		D0,T0,D0
	    sub		D1,T1,D1
	    ldd		[COEFF + 4*22],T0
	    ld		[COEFFLIST+I],COEFF
	    sub		D2,T0,D2
	    and		COEFF,63,T0
	    sll		T0,2,T0
	    sub		D3,T1,D3
	    ld		[JTBASE+T0],T0
	    andn	COEFF,63,COEFF
	    jmp		JTBASE+T0
	    inc		4,I

Lcase11r3:
	    ld		[COEFF + 4*22],T0
	    ld		[COEFF + 4*25],T1
	    sub		D0,T0,D0
	    sub		D1,T1,D1
	    ld		[COEFF + 4*27],T0
	    ld		[COEFF + 4*28],T1
	    sub		D2,T0,D2
	    ld		[COEFFLIST+I],COEFF
	    sub		D3,T1,D3
	    and		COEFF,63,T0
	    sll		T0,2,T0
	    ld		[JTBASE+T0],T0
	    andn	COEFF,63,COEFF
	    jmp		JTBASE+T0
	    inc		4,I

Lcase12r0:
	    ldd		[COEFF + 4*16],T0
	    ld		[COEFFLIST+I],COEFF
	    add		A0,T0,A0
	    sub		A3,T0,A3
	    add		A1,T1,A1
	    and		COEFF,63,T0
	    sll		T0,2,T0
	    sub		A2,T1,A2
	    ld		[JTBASE+T0],T0
	    andn	COEFF,63,COEFF
	    jmp		JTBASE+T0
	    inc		4,I

Lcase12r1:
	    ldd		[COEFF + 4*18],T0
	    ld		[COEFFLIST+I],COEFF
	    add		A0,T0,A0
	    sub		A3,T0,A3
	    add		A1,T1,A1
	    and		COEFF,63,T0
	    sll		T0,2,T0
	    sub		A2,T1,A2
	    ld		[JTBASE+T0],T0
	    andn	COEFF,63,COEFF
	    jmp		JTBASE+T0
	    inc		4,I

Lcase12r2:
	    ldd		[COEFF + 4*18],T0
	    ld		[COEFFLIST+I],COEFF
	    sub		A0,T0,A0
	    add		A3,T0,A3
	    sub		A1,T1,A1
	    and		COEFF,63,T0
	    sll		T0,2,T0
	    add		A2,T1,A2
	    ld		[JTBASE+T0],T0
	    andn	COEFF,63,COEFF
	    jmp		JTBASE+T0
	    inc		4,I

Lcase12r3:
	    ldd		[COEFF + 4*16],T0
	    ld		[COEFFLIST+I],COEFF
	    sub		A0,T0,A0
	    add		A3,T0,A3
	    sub		A1,T1,A1
	    and		COEFF,63,T0
	    sll		T0,2,T0
	    add		A2,T1,A2
	    ld		[JTBASE+T0],T0
	    andn	COEFF,63,COEFF
	    jmp		JTBASE+T0
	    inc		4,I

Lcase13r0:
	    ldd		[COEFF + 4*20],T0
	    sub		D2,T0,D2
	    add		D0,T1,D0
	    ldd		[COEFF + 4*22],T0
	    ld		[COEFFLIST+I],COEFF
	    sub		D3,T0,D3
	    and		COEFF,63,T0
	    sll		T0,2,T0
	    sub		D1,T1,D1
	    ld		[JTBASE+T0],T0
	    andn	COEFF,63,COEFF
	    jmp		JTBASE+T0
	    inc		4,I

Lcase13r1:
	    ld		[COEFF + 4*21],T0
	    sub		D2,T0,D2
	    ldd		[COEFF + 4*24],T0
	    add		D0,T0,D0
	    sub		D3,T1,D3
	    ld		[COEFF + 4*26],T0
	    ld		[COEFFLIST+I],COEFF
	    sub		D1,T0,D1
	    and		COEFF,63,T0
	    sll		T0,2,T0
	    ld		[JTBASE+T0],T0
	    andn	COEFF,63,COEFF
	    jmp		JTBASE+T0
	    inc		4,I

Lcase13r2:
	    ld		[COEFF + 4*22],T0
	    ld		[COEFF + 4*25],T1
	    sub		D2,T0,D2
	    add		D0,T1,D0
	    ld		[COEFF + 4*27],T0
	    ld		[COEFF + 4*28],T1
	    sub		D3,T0,D3
	    ld		[COEFFLIST+I],COEFF
	    sub		D1,T1,D1
	    and		COEFF,63,T0
	    sll		T0,2,T0
	    ld		[JTBASE+T0],T0
	    andn	COEFF,63,COEFF
	    jmp		JTBASE+T0
	    inc		4,I

Lcase13r3:
	    ld		[COEFF + 4*23],T0
	    ld		[COEFF + 4*26],T1
	    sub		D2,T0,D2
	    add		D0,T1,D0
	    ldd		[COEFF + 4*28],T0
	    ld		[COEFFLIST+I],COEFF
	    sub		D3,T0,D3
	    and		COEFF,63,T0
	    sll		T0,2,T0
	    sub		D1,T1,D1
	    ld		[JTBASE+T0],T0
	    andn	COEFF,63,COEFF
	    jmp		JTBASE+T0
	    inc		4,I

Lcase14r0:
Lcase14r1:
Lcase14r2:
Lcase14r3:
	    ld		[COEFF + 4*0],T0
	    ld		[COEFFLIST+I],COEFF
	    add		A0,T0,A0
	    and		COEFF,63,T1
	    sub		A1,T0,A1
	    sll		T1,2,T1
	    sub		A2,T0,A2
	    add		A3,T0,A3
	    ld		[JTBASE+T1],T1
	    andn	COEFF,63,COEFF
	    jmp		JTBASE+T1
	    inc		4,I

Lcase15r0:
Lcase15r1:
Lcase15r2:
Lcase15r3:
	    ldd		[COEFF + 4*4],T0
	    sub		B1,T0,B1
	    add		B3,T1,B3
	    ldd		[COEFF + 4*6],T0
	    ld		[COEFFLIST+I],COEFF
	    add		B0,T0,B0
	    and		COEFF,63,T0
	    sll		T0,2,T0
	    add		B2,T1,B2
	    ld		[JTBASE+T0],T0
	    andn	COEFF,63,COEFF
	    jmp		JTBASE+T0
	    inc		4,I

Lcase16r0:
	    ld		[COEFF + 4*4],T0
	    ld		[COEFFLIST+I],COEFF
	    add		C0,T0,C0
	    and		COEFF,63,T1
	    sub		C1,T0,C1
	    sll		T1,2,T1
	    sub		C2,T0,C2
	    add		C3,T0,C3
	    ld		[JTBASE+T1],T1
	    andn	COEFF,63,COEFF
	    jmp		JTBASE+T1
	    inc		4,I

Lcase16r1:
	    ld		[COEFF + 4*5],T0
	    ld		[COEFFLIST+I],COEFF
	    add		C0,T0,C0
	    and		COEFF,63,T1
	    sub		C1,T0,C1
	    sll		T1,2,T1
	    sub		C2,T0,C2
	    add		C3,T0,C3
	    ld		[JTBASE+T1],T1
	    andn	COEFF,63,COEFF
	    jmp		JTBASE+T1
	    inc		4,I

Lcase16r2:
	    ld		[COEFF + 4*6],T0
	    ld		[COEFFLIST+I],COEFF
	    add		C0,T0,C0
	    and		COEFF,63,T1
	    sub		C1,T0,C1
	    sll		T1,2,T1
	    sub		C2,T0,C2
	    add		C3,T0,C3
	    ld		[JTBASE+T1],T1
	    andn	COEFF,63,COEFF
	    jmp		JTBASE+T1
	    inc		4,I

Lcase16r3:
	    ld		[COEFF + 4*7],T0
	    ld		[COEFFLIST+I],COEFF
	    add		C0,T0,C0
	    and		COEFF,63,T1
	    sub		C1,T0,C1
	    sll		T1,2,T1
	    sub		C2,T0,C2
	    add		C3,T0,C3
	    ld		[JTBASE+T1],T1
	    andn	COEFF,63,COEFF
	    jmp		JTBASE+T1
	    inc		4,I

Lcase17r0:
	    ldd		[COEFF + 4*8],T0
	    sub		B2,T0,B2
	    add		B0,T1,B0
	    ldd		[COEFF + 4*10],T0
	    ld		[COEFFLIST+I],COEFF
	    sub		B3,T0,B3
	    and		COEFF,63,T0
	    sll		T0,2,T0
	    sub		B1,T1,B1
	    ld		[JTBASE+T0],T0
	    andn	COEFF,63,COEFF
	    jmp		JTBASE+T0
	    inc		4,I

Lcase17r1:
	    ldd		[COEFF + 4*12],T0
	    sub		B2,T0,B2
	    add		B0,T1,B0
	    ldd		[COEFF + 4*14],T0
	    ld		[COEFFLIST+I],COEFF
	    sub		B3,T0,B3
	    and		COEFF,63,T0
	    sll		T0,2,T0
	    sub		B1,T1,B1
	    ld		[JTBASE+T0],T0
	    andn	COEFF,63,COEFF
	    jmp		JTBASE+T0
	    inc		4,I

Lcase17r2:
	    ldd		[COEFF + 4*12],T0
	    add		B2,T0,B2
	    sub		B0,T1,B0
	    ldd		[COEFF + 4*14],T0
	    ld		[COEFFLIST+I],COEFF
	    add		B3,T0,B3
	    and		COEFF,63,T0
	    sll		T0,2,T0
	    add		B1,T1,B1
	    ld		[JTBASE+T0],T0
	    andn	COEFF,63,COEFF
	    jmp		JTBASE+T0
	    inc		4,I

Lcase17r3:
	    ldd		[COEFF + 4*8],T0
	    add		B2,T0,B2
	    sub		B0,T1,B0
	    ldd		[COEFF + 4*10],T0
	    ld		[COEFFLIST+I],COEFF
	    add		B3,T0,B3
	    and		COEFF,63,T0
	    sll		T0,2,T0
	    add		B1,T1,B1
	    ld		[JTBASE+T0],T0
	    andn	COEFF,63,COEFF
	    jmp		JTBASE+T0
	    inc		4,I

Lcase18r0:
	    ld		[COEFF + 4*9],T0
	    ld		[COEFF + 4*13],T1
	    add		C0,T0,C0
	    sub		C3,T0,C3
	    ld		[COEFFLIST+I],COEFF
	    add		C1,T1,C1
	    and		COEFF,63,T0
	    sll		T0,2,T0
	    sub		C2,T1,C2
	    ld		[JTBASE+T0],T0
	    andn	COEFF,63,COEFF
	    jmp		JTBASE+T0
	    inc		4,I

Lcase18r1:
	    ld		[COEFF + 4*11],T0
	    ld		[COEFF + 4*15],T1
	    sub		C0,T0,C0
	    add		C3,T0,C3
	    ld		[COEFFLIST+I],COEFF
	    sub		C1,T1,C1
	    and		COEFF,63,T0
	    sll		T0,2,T0
	    add		C2,T1,C2
	    ld		[JTBASE+T0],T0
	    andn	COEFF,63,COEFF
	    jmp		JTBASE+T0
	    inc		4,I

Lcase18r2:
	    ld		[COEFF + 4*8],T0
	    ld		[COEFF + 4*12],T1
	    sub		C0,T0,C0
	    add		C3,T0,C3
	    ld		[COEFFLIST+I],COEFF
	    sub		C1,T1,C1
	    and		COEFF,63,T0
	    sll		T0,2,T0
	    add		C2,T1,C2
	    ld		[JTBASE+T0],T0
	    andn	COEFF,63,COEFF
	    jmp		JTBASE+T0
	    inc		4,I

Lcase18r3:
	    ld		[COEFF + 4*10],T0
	    ld		[COEFF + 4*14],T1
	    sub		C0,T0,C0
	    add		C3,T0,C3
	    ld		[COEFFLIST+I],COEFF
	    sub		C1,T1,C1
	    and		COEFF,63,T0
	    sll		T0,2,T0
	    add		C2,T1,C2
	    ld		[JTBASE+T0],T0
	    andn	COEFF,63,COEFF
	    jmp		JTBASE+T0
	    inc		4,I

Lcase19r0:
Lcase19r3:
	    ldd		[COEFF + 4*4],T0
	    add		B0,T0,B0
	    add		B1,T1,B1
	    ldd		[COEFF + 4*6],T0
	    ld		[COEFFLIST+I],COEFF
	    add		B2,T0,B2
	    and		COEFF,63,T0
	    sll		T0,2,T0
	    add		B3,T1,B3
	    ld		[JTBASE+T0],T0
	    andn	COEFF,63,COEFF
	    jmp		JTBASE+T0
	    inc		4,I

Lcase19r1:
Lcase19r2:
	    ldd		[COEFF + 4*4],T0
	    sub		B0,T0,B0
	    sub		B1,T1,B1
	    ldd		[COEFF + 4*6],T0
	    ld		[COEFFLIST+I],COEFF
	    sub		B2,T0,B2
	    and		COEFF,63,T0
	    sll		T0,2,T0
	    sub		B3,T1,B3
	    ld		[JTBASE+T0],T0
	    andn	COEFF,63,COEFF
	    jmp		JTBASE+T0
	    inc		4,I

Lcase20r0:
	    ld		[COEFF + 4*6],T0
	    ld		[COEFFLIST+I],COEFF
	    add		C0,T0,C0
	    and		COEFF,63,T1
	    add		C1,T0,C1
	    sll		T1,2,T1
	    add		C2,T0,C2
	    add		C3,T0,C3
	    ld		[JTBASE+T1],T1
	    andn	COEFF,63,COEFF
	    jmp		JTBASE+T1
	    inc		4,I
	    
Lcase20r1:
	    ld		[COEFF + 4*4],T0
	    ld		[COEFFLIST+I],COEFF
	    sub		C0,T0,C0
	    and		COEFF,63,T1
	    sub		C1,T0,C1
	    sll		T1,2,T1
	    sub		C2,T0,C2
	    sub		C3,T0,C3
	    ld		[JTBASE+T1],T1
	    andn	COEFF,63,COEFF
	    jmp		JTBASE+T1
	    inc		4,I
	    
Lcase20r2:
	    ld		[COEFF + 4*7],T0
	    ld		[COEFFLIST+I],COEFF
	    add		C0,T0,C0
	    and		COEFF,63,T1
	    add		C1,T0,C1
	    sll		T1,2,T1
	    add		C2,T0,C2
	    add		C3,T0,C3
	    ld		[JTBASE+T1],T1
	    andn	COEFF,63,COEFF
	    jmp		JTBASE+T1
	    inc		4,I
	    
Lcase20r3:
	    ld		[COEFF + 4*5],T0
	    ld		[COEFFLIST+I],COEFF
	    add		C0,T0,C0
	    and		COEFF,63,T1
	    add		C1,T0,C1
	    sll		T1,2,T1
	    add		C2,T0,C2
	    add		C3,T0,C3
	    ld		[JTBASE+T1],T1
	    andn	COEFF,63,COEFF
	    jmp		JTBASE+T1
	    inc		4,I
	    
Lcase21r0:
	    ld		[COEFF + 4*3],T0
	    ld		[COEFFLIST+I],COEFF
	    add		A0,T0,A0
	    and		COEFF,63,T1
	    add		A1,T0,A1
	    sll		T1,2,T1
	    add		A2,T0,A2
	    add		A3,T0,A3
	    ld		[JTBASE+T1],T1
	    andn	COEFF,63,COEFF
	    jmp		JTBASE+T1
	    inc		4,I

Lcase21r1:
	    ld		[COEFF + 4*2],T0
	    ld		[COEFFLIST+I],COEFF
	    sub		A0,T0,A0
	    and		COEFF,63,T1
	    sub		A1,T0,A1
	    sll		T1,2,T1
	    sub		A2,T0,A2
	    sub		A3,T0,A3
	    ld		[JTBASE+T1],T1
	    andn	COEFF,63,COEFF
	    jmp		JTBASE+T1
	    inc		4,I

Lcase21r2:
	    ld		[COEFF + 4*2],T0
	    ld		[COEFFLIST+I],COEFF
	    add		A0,T0,A0
	    and		COEFF,63,T1
	    add		A1,T0,A1
	    sll		T1,2,T1
	    add		A2,T0,A2
	    add		A3,T0,A3
	    ld		[JTBASE+T1],T1
	    andn	COEFF,63,COEFF
	    jmp		JTBASE+T1
	    inc		4,I

Lcase21r3:
	    ld		[COEFF + 4*3],T0
	    ld		[COEFFLIST+I],COEFF
	    sub		A0,T0,A0
	    and		COEFF,63,T1
	    sub		A1,T0,A1
	    sll		T1,2,T1
	    sub		A2,T0,A2
	    sub		A3,T0,A3
	    ld		[JTBASE+T1],T1
	    andn	COEFF,63,COEFF
	    jmp		JTBASE+T1
	    inc		4,I

Lcase22r0:
	    ld		[COEFF + 4*22],T0
	    ld		[COEFF + 4*25],T1
	    add		D0,T0,D0
	    add		D1,T1,D1
	    ld		[COEFF + 4*27],T0
	    ld		[COEFF + 4*28],T1
	    ld		[COEFFLIST+I],COEFF
	    add		D2,T0,D2
	    and		COEFF,63,T0
	    sll		T0,2,T0
	    add		D3,T1,D3
	    ld		[JTBASE+T0],T0
	    andn	COEFF,63,COEFF
	    jmp		JTBASE+T0
	    inc		4,I

Lcase22r1:
	    ld		[COEFF + 4*20],T0
	    ld		[COEFF + 4*21],T1
	    sub		D0,T0,D0
	    sub		D1,T1,D1
	    ld		[COEFF + 4*22],T0
	    ld		[COEFF + 4*23],T1
	    ld		[COEFFLIST+I],COEFF
	    sub		D2,T0,D2
	    and		COEFF,63,T0
	    sll		T0,2,T0
	    sub		D3,T1,D3
	    ld		[JTBASE+T0],T0
	    andn	COEFF,63,COEFF
	    jmp		JTBASE+T0
	    inc		4,I

Lcase22r2:
	    ld		[COEFF + 4*23],T0
	    ld		[COEFF + 4*26],T1
	    add		D0,T0,D0
	    add		D1,T1,D1
	    ld		[COEFF + 4*28],T0
	    ld		[COEFF + 4*29],T1
	    ld		[COEFFLIST+I],COEFF
	    add		D2,T0,D2
	    and		COEFF,63,T0
	    sll		T0,2,T0
	    add		D3,T1,D3
	    ld		[JTBASE+T0],T0
	    andn	COEFF,63,COEFF
	    jmp		JTBASE+T0
	    inc		4,I

Lcase22r3:
	    ld		[COEFF + 4*21],T0
	    ld		[COEFF + 4*24],T1
	    add		D0,T0,D0
	    add		D1,T1,D1
	    ld		[COEFF + 4*25],T0
	    ld		[COEFF + 4*26],T1
	    ld		[COEFFLIST+I],COEFF
	    add		D2,T0,D2
	    and		COEFF,63,T0
	    sll		T0,2,T0
	    add		D3,T1,D3
	    ld		[JTBASE+T0],T0
	    andn	COEFF,63,COEFF
	    jmp		JTBASE+T0
	    inc		4,I

Lcase23r0:
Lcase23r3:
	    ldd		[COEFF + 4*2],T0
	    ld		[COEFFLIST+I],COEFF
	    add		A0,T0,A0
	    sub		A3,T0,A3
	    add		A1,T1,A1
	    and		COEFF,63,T0
	    sll		T0,2,T0
	    sub		A2,T1,A2
	    ld		[JTBASE+T0],T0
	    andn	COEFF,63,COEFF
	    jmp		JTBASE+T0
	    inc		4,I

Lcase23r1:
Lcase23r2:
	    ldd		[COEFF + 4*2],T0
	    ld		[COEFFLIST+I],COEFF
	    sub		A0,T0,A0
	    add		A3,T0,A3
	    sub		A1,T1,A1
	    and		COEFF,63,T0
	    sll		T0,2,T0
	    add		A2,T1,A2
	    ld		[JTBASE+T0],T0
	    andn	COEFF,63,COEFF
	    jmp		JTBASE+T0
	    inc		4,I

Lcase24r0:
	    ld		[COEFF + 4*21],T0
	    sub		D2,T0,D2
	    ldd		[COEFF + 4*24],T0
	    add		D0,T0,D0
	    sub		D3,T1,D3
	    ld		[COEFF + 4*26],T0
	    ld		[COEFFLIST+I],COEFF
	    sub		D1,T0,D1
	    and		COEFF,63,T0
	    sll		T0,2,T0
	    ld		[JTBASE+T0],T0
	    andn	COEFF,63,COEFF
	    jmp		JTBASE+T0
	    inc		4,I

Lcase24r1:
	    ld		[COEFF + 4*23],T0
	    ld		[COEFF + 4*26],T1
	    add		D2,T0,D2
	    sub		D0,T1,D0
	    ldd		[COEFF + 4*28],T0
	    ld		[COEFFLIST+I],COEFF
	    add		D3,T0,D3
	    and		COEFF,63,T0
	    sll		T0,2,T0
	    add		D1,T1,D1
	    ld		[JTBASE+T0],T0
	    andn	COEFF,63,COEFF
	    jmp		JTBASE+T0
	    inc		4,I

Lcase24r2:
	    ldd		[COEFF + 4*20],T0
	    add		D2,T0,D2
	    sub		D0,T1,D0
	    ldd		[COEFF + 4*22],T0
	    ld		[COEFFLIST+I],COEFF
	    add		D3,T0,D3
	    and		COEFF,63,T0
	    sll		T0,2,T0
	    add		D1,T1,D1
	    ld		[JTBASE+T0],T0
	    andn	COEFF,63,COEFF
	    jmp		JTBASE+T0
	    inc		4,I

Lcase24r3:
	    ld		[COEFF + 4*22],T0
	    ld		[COEFF + 4*25],T1
	    add		D2,T0,D2
	    sub		D0,T1,D0
	    ld		[COEFF + 4*27],T0
	    ld		[COEFF + 4*28],T1
	    ld		[COEFFLIST+I],COEFF
	    add		D3,T0,D3
	    and		COEFF,63,T0
	    sll		T0,2,T0
	    add		D1,T1,D1
	    ld		[JTBASE+T0],T0
	    andn	COEFF,63,COEFF
	    jmp		JTBASE+T0
	    inc		4,I

Lcase25r0:
	    ld		[COEFF + 4*2],T0
	    ld		[COEFFLIST+I],COEFF
	    add		A0,T0,A0
	    and		COEFF,63,T1
	    sub		A1,T0,A1
	    sll		T1,2,T1
	    sub		A2,T0,A2
	    add		A3,T0,A3
	    ld		[JTBASE+T1],T1
	    andn	COEFF,63,COEFF
	    jmp		JTBASE+T1
	    inc		4,I

Lcase25r1:
	    ld		[COEFF + 4*3],T0
	    ld		[COEFFLIST+I],COEFF
	    add		A0,T0,A0
	    and		COEFF,63,T1
	    sub		A1,T0,A1
	    sll		T1,2,T1
	    sub		A2,T0,A2
	    add		A3,T0,A3
	    ld		[JTBASE+T1],T1
	    andn	COEFF,63,COEFF
	    jmp		JTBASE+T1
	    inc		4,I

Lcase25r2:
	    ld		[COEFF + 4*3],T0
	    ld		[COEFFLIST+I],COEFF
	    sub		A0,T0,A0
	    and		COEFF,63,T1
	    add		A1,T0,A1
	    sll		T1,2,T1
	    add		A2,T0,A2
	    sub		A3,T0,A3
	    ld		[JTBASE+T1],T1
	    andn	COEFF,63,COEFF
	    jmp		JTBASE+T1
	    inc		4,I

Lcase25r3:
	    ld		[COEFF + 4*2],T0
	    ld		[COEFFLIST+I],COEFF
	    sub		A0,T0,A0
	    and		COEFF,63,T1
	    add		A1,T0,A1
	    sll		T1,2,T1
	    add		A2,T0,A2
	    sub		A3,T0,A3
	    ld		[JTBASE+T1],T1
	    andn	COEFF,63,COEFF
	    jmp		JTBASE+T1
	    inc		4,I

Lcase26r0:
	    ldd		[COEFF + 4*20],T0
	    sub		D1,T0,D1
	    add		D3,T1,D3
	    ldd		[COEFF + 4*22],T0
	    ld		[COEFFLIST+I],COEFF
	    add		D0,T0,D0
	    and		COEFF,63,T0
	    sll		T0,2,T0
	    add		D2,T1,D2
	    ld		[JTBASE+T0],T0
	    andn	COEFF,63,COEFF
	    jmp		JTBASE+T0
	    inc		4,I

Lcase26r1:
	    ld		[COEFF + 4*21],T0
	    sub		D1,T0,D1
	    ldd		[COEFF + 4*24],T0
	    add		D3,T0,D3
	    add		D0,T1,D0
	    ld		[COEFF + 4*26],T0
	    ld		[COEFFLIST+I],COEFF
	    add		D2,T0,D2
	    and		COEFF,63,T0
	    sll		T0,2,T0
	    ld		[JTBASE+T0],T0
	    andn	COEFF,63,COEFF
	    jmp		JTBASE+T0
	    inc		4,I

Lcase26r2:
	    ld		[COEFF + 4*22],T0
	    ld		[COEFF + 4*25],T1
	    sub		D1,T0,D1
	    add		D3,T1,D3
	    ld		[COEFF + 4*27],T0
	    ld		[COEFF + 4*28],T1
	    ld		[COEFFLIST+I],COEFF
	    add		D0,T0,D0
	    and		COEFF,63,T0
	    sll		T0,2,T0
	    add		D2,T1,D2
	    ld		[JTBASE+T0],T0
	    andn	COEFF,63,COEFF
	    jmp		JTBASE+T0
	    inc		4,I

Lcase26r3:
	    ld		[COEFF + 4*23],T0
	    sub		D1,T0,D1
	    ld		[COEFF + 4*26],T1
	    add		D3,T1,D3
	    ldd		[COEFF + 4*28],T0
	    ld		[COEFFLIST+I],COEFF
	    add		D0,T0,D0
	    and		COEFF,63,T0
	    sll		T0,2,T0
	    add		D2,T1,D2
	    ld		[JTBASE+T0],T0
	    andn	COEFF,63,COEFF
	    jmp		JTBASE+T0
	    inc		4,I

Lcase27r0:
Lcase27r1:
Lcase27r2:
Lcase27r3:
	    ldd		[COEFF + 4*2],T0
	    ld		[COEFFLIST+I],COEFF
	    sub		A1,T0,A1
	    add		A2,T0,A2
	    add		A0,T1,A0
	    and		COEFF,63,T0
	    sll		T0,2,T0
	    sub		A3,T1,A3
	    ld		[JTBASE+T0],T0
	    andn	COEFF,63,COEFF
	    jmp		JTBASE+T0
	    inc		4,I

Lcase28r0:
Lcase28r1:
Lcase28r2:
Lcase28r3:
	    ldd		[COEFF + 4*4],T0
	    sub		B3,T0,B3
	    add		B2,T1,B2
	    ldd		[COEFF + 4*6],T0
	    ld		[COEFFLIST+I],COEFF
	    sub		B1,T0,B1
	    and		COEFF,63,T0
	    sll		T0,2,T0
	    add		B0,T1,B0
	    ld		[JTBASE+T0],T0
	    andn	COEFF,63,COEFF
	    jmp		JTBASE+T0
	    inc		4,I

Lcase29r0:
	    ld		[COEFF + 4*8],T0
	    ld		[COEFF + 4*12],T1
	    sub		C1,T0,C1
	    add		C2,T0,C2
	    ld		[COEFFLIST+I],COEFF
	    add		C0,T1,C0
	    and		COEFF,63,T0
	    sll		T0,2,T0
	    sub		C3,T1,C3
	    ld		[JTBASE+T0],T0
	    andn	COEFF,63,COEFF
	    jmp		JTBASE+T0
	    inc		4,I

Lcase29r1:
	    ld		[COEFF + 4*9],T0
	    ld		[COEFF + 4*13],T1
	    sub		C1,T0,C1
	    add		C2,T0,C2
	    ld		[COEFFLIST+I],COEFF
	    add		C0,T1,C0
	    and		COEFF,63,T0
	    sll		T0,2,T0
	    sub		C3,T1,C3
	    ld		[JTBASE+T0],T0
	    andn	COEFF,63,COEFF
	    jmp		JTBASE+T0
	    inc		4,I

Lcase29r2:
	    ld		[COEFF + 4*10],T0
	    ld		[COEFF + 4*14],T1
	    sub		C1,T0,C1
	    add		C2,T0,C2
	    ld		[COEFFLIST+I],COEFF
	    add		C0,T1,C0
	    and		COEFF,63,T0
	    sll		T0,2,T0
	    sub		C3,T1,C3
	    ld		[JTBASE+T0],T0
	    andn	COEFF,63,COEFF
	    jmp		JTBASE+T0
	    inc		4,I

Lcase29r3:
	    ld		[COEFF + 4*11],T0
	    ld		[COEFF + 4*15],T1
	    sub		C1,T0,C1
	    add		C2,T0,C2
	    ld		[COEFFLIST+I],COEFF
	    add		C0,T1,C0
	    and		COEFF,63,T0
	    sll		T0,2,T0
	    sub		C3,T1,C3
	    ld		[JTBASE+T0],T0
	    andn	COEFF,63,COEFF
	    jmp		JTBASE+T0
	    inc		4,I

Lcase30r0:
	    ldd		[COEFF + 4*8],T0
	    sub		B1,T0,B1
	    add		B3,T1,B3
	    ldd		[COEFF + 4*10],T0
	    ld		[COEFFLIST+I],COEFF
	    add		B0,T0,B0
	    and		COEFF,63,T0
	    sll		T0,2,T0
	    add		B2,T1,B2
	    ld		[JTBASE+T0],T0
	    andn	COEFF,63,COEFF
	    jmp		JTBASE+T0
	    inc		4,I

Lcase30r1:
	    ldd		[COEFF + 4*12],T0
	    sub		B1,T0,B1
	    add		B3,T1,B3
	    ldd		[COEFF + 4*14],T0
	    ld		[COEFFLIST+I],COEFF
	    add		B0,T0,B0
	    and		COEFF,63,T0
	    sll		T0,2,T0
	    add		B2,T1,B2
	    ld		[JTBASE+T0],T0
	    andn	COEFF,63,COEFF
	    jmp		JTBASE+T0
	    inc		4,I

Lcase30r2:
	    ldd		[COEFF + 4*12],T0
	    add		B1,T0,B1
	    sub		B3,T1,B3
	    ldd		[COEFF + 4*14],T0
	    ld		[COEFFLIST+I],COEFF
	    sub		B0,T0,B0
	    and		COEFF,63,T0
	    sll		T0,2,T0
	    sub		B2,T1,B2
	    ld		[JTBASE+T0],T0
	    andn	COEFF,63,COEFF
	    jmp		JTBASE+T0
	    inc		4,I

Lcase30r3:
	    ldd		[COEFF + 4*8],T0
	    add		B1,T0,B1
	    sub		B3,T1,B3
	    ldd		[COEFF + 4*10],T0
	    ld		[COEFFLIST+I],COEFF
	    sub		B0,T0,B0
	    and		COEFF,63,T0
	    sll		T0,2,T0
	    sub		B2,T1,B2
	    ld		[JTBASE+T0],T0
	    andn	COEFF,63,COEFF
	    jmp		JTBASE+T0
	    inc		4,I

Lcase31r0:
	    ld		[COEFF + 4*5],T0
	    ld		[COEFFLIST+I],COEFF
	    add		C0,T0,C0
	    and		COEFF,63,T1
	    sub		C1,T0,C1
	    sll		T1,2,T1
	    sub		C2,T0,C2
	    add		C3,T0,C3
	    ld		[JTBASE+T1],T1
	    andn	COEFF,63,COEFF
	    jmp		JTBASE+T1
	    inc		4,I

Lcase31r1:
	    ld		[COEFF + 4*7],T0
	    ld		[COEFFLIST+I],COEFF
	    sub		C0,T0,C0
	    and		COEFF,63,T1
	    add		C1,T0,C1
	    sll		T1,2,T1
	    add		C2,T0,C2
	    sub		C3,T0,C3
	    ld		[JTBASE+T1],T1
	    andn	COEFF,63,COEFF
	    jmp		JTBASE+T1
	    inc		4,I

Lcase31r2:
	    ld		[COEFF + 4*4],T0
	    ld		[COEFFLIST+I],COEFF
	    sub		C0,T0,C0
	    and		COEFF,63,T1
	    add		C1,T0,C1
	    sll		T1,2,T1
	    add		C2,T0,C2
	    sub		C3,T0,C3
	    ld		[JTBASE+T1],T1
	    andn	COEFF,63,COEFF
	    jmp		JTBASE+T1
	    inc		4,I

Lcase31r3:
	    ld		[COEFF + 4*6],T0
	    ld		[COEFFLIST+I],COEFF
	    sub		C0,T0,C0
	    and		COEFF,63,T1
	    add		C1,T0,C1
	    sll		T1,2,T1
	    add		C2,T0,C2
	    sub		C3,T0,C3
	    ld		[JTBASE+T1],T1
	    andn	COEFF,63,COEFF
	    jmp		JTBASE+T1
	    inc		4,I

Lcase32r0:
Lcase32r3:
	    ldd		[COEFF + 4*4],T0
	    sub		B2,T0,B2
	    add		B0,T1,B0
	    ldd		[COEFF + 4*6],T0
	    ld		[COEFFLIST+I],COEFF
	    sub		B3,T0,B3
	    and		COEFF,63,T0
	    sll		T0,2,T0
	    sub		B1,T1,B1
	    ld		[JTBASE+T0],T0
	    andn	COEFF,63,COEFF
	    jmp		JTBASE+T0
	    inc		4,I

Lcase32r1:
Lcase32r2:
	    ldd		[COEFF + 4*4],T0
	    add		B2,T0,B2
	    sub		B0,T1,B0
	    ldd		[COEFF + 4*6],T0
	    ld		[COEFFLIST+I],COEFF
	    add		B3,T0,B3
	    and		COEFF,63,T0
	    sll		T0,2,T0
	    add		B1,T1,B1
	    ld		[JTBASE+T0],T0
	    andn	COEFF,63,COEFF
	    jmp		JTBASE+T0
	    inc		4,I

Lcase33r0:
	    ld		[COEFF + 4*10],T0
	    ld		[COEFF + 4*14],T1
	    add		C0,T0,C0
	    sub		C3,T0,C3
	    ld		[COEFFLIST+I],COEFF
	    add		C1,T1,C1
	    and		COEFF,63,T0
	    sll		T0,2,T0
	    sub		C2,T1,C2
	    ld		[JTBASE+T0],T0
	    andn	COEFF,63,COEFF
	    jmp		JTBASE+T0
	    inc		4,I

Lcase33r1:
	    ld		[COEFF + 4*8],T0
	    ld		[COEFF + 4*12],T1
	    sub		C0,T0,C0
	    add		C3,T0,C3
	    ld		[COEFFLIST+I],COEFF
	    sub		C1,T1,C1
	    and		COEFF,63,T0
	    sll		T0,2,T0
	    add		C2,T1,C2
	    ld		[JTBASE+T0],T0
	    andn	COEFF,63,COEFF
	    jmp		JTBASE+T0
	    inc		4,I

Lcase33r2:
	    ld		[COEFF + 4*11],T0
	    ld		[COEFF + 4*15],T1
	    add		C0,T0,C0
	    sub		C3,T0,C3
	    ld		[COEFFLIST+I],COEFF
	    add		C1,T1,C1
	    and		COEFF,63,T0
	    sll		T0,2,T0
	    sub		C2,T1,C2
	    ld		[JTBASE+T0],T0
	    andn	COEFF,63,COEFF
	    jmp		JTBASE+T0
	    inc		4,I

Lcase33r3:
	    ld		[COEFF + 4*9],T0
	    ld		[COEFF + 4*13],T1
	    add		C0,T0,C0
	    sub		C3,T0,C3
	    ld		[COEFFLIST+I],COEFF
	    add		C1,T1,C1
	    and		COEFF,63,T0
	    sll		T0,2,T0
	    sub		C2,T1,C2
	    ld		[JTBASE+T0],T0
	    andn	COEFF,63,COEFF
	    jmp		JTBASE+T0
	    inc		4,I

Lcase34r0:
	    ldd		[COEFF + 4*12],T0
	    add		B0,T0,B0
	    add		B1,T1,B1
	    ldd		[COEFF + 4*14],T0
	    ld		[COEFFLIST+I],COEFF
	    add		B2,T0,B2
	    and		COEFF,63,T0
	    sll		T0,2,T0
	    add		B3,T1,B3
	    ld		[JTBASE+T0],T0
	    andn	COEFF,63,COEFF
	    jmp		JTBASE+T0
	    inc		4,I

Lcase34r1:
	    ldd		[COEFF + 4*8],T0
	    sub		B0,T0,B0
	    sub		B1,T1,B1
	    ldd		[COEFF + 4*10],T0
	    ld		[COEFFLIST+I],COEFF
	    sub		B2,T0,B2
	    and		COEFF,63,T0
	    sll		T0,2,T0
	    sub		B3,T1,B3
	    ld		[JTBASE+T0],T0
	    andn	COEFF,63,COEFF
	    jmp		JTBASE+T0
	    inc		4,I

Lcase34r2:
	    ldd		[COEFF + 4*8],T0
	    add		B0,T0,B0
	    add		B1,T1,B1
	    ldd		[COEFF + 4*10],T0
	    ld		[COEFFLIST+I],COEFF
	    add		B2,T0,B2
	    and		COEFF,63,T0
	    sll		T0,2,T0
	    add		B3,T1,B3
	    ld		[JTBASE+T0],T0
	    andn	COEFF,63,COEFF
	    jmp		JTBASE+T0
	    inc		4,I

Lcase34r3:
	    ldd		[COEFF + 4*12],T0
	    sub		B0,T0,B0
	    sub		B1,T1,B1
	    ldd		[COEFF + 4*14],T0
	    ld		[COEFFLIST+I],COEFF
	    sub		B2,T0,B2
	    and		COEFF,63,T0
	    sll		T0,2,T0
	    sub		B3,T1,B3
	    ld		[JTBASE+T0],T0
	    andn	COEFF,63,COEFF
	    jmp		JTBASE+T0
	    inc		4,I

Lcase35r0:
	    ld		[COEFF + 4*7],T0
	    ld		[COEFFLIST+I],COEFF
	    add		C0,T0,C0
	    and		COEFF,63,T1
	    add		C1,T0,C1
	    sll		T1,2,T1
	    add		C2,T0,C2
	    add		C3,T0,C3
	    ld		[JTBASE+T1],T1
	    andn	COEFF,63,COEFF
	    jmp		JTBASE+T1
	    inc		4,I

Lcase35r1:
	    ld		[COEFF + 4*6],T0
	    ld		[COEFFLIST+I],COEFF
	    sub		C0,T0,C0
	    and		COEFF,63,T1
	    sub		C1,T0,C1
	    sll		T1,2,T1
	    sub		C2,T0,C2
	    sub		C3,T0,C3
	    ld		[JTBASE+T1],T1
	    andn	COEFF,63,COEFF
	    jmp		JTBASE+T1
	    inc		4,I

Lcase35r2:
	    ld		[COEFF + 4*5],T0
	    ld		[COEFFLIST+I],COEFF
	    add		C0,T0,C0
	    and		COEFF,63,T1
	    add		C1,T0,C1
	    sll		T1,2,T1
	    add		C2,T0,C2
	    add		C3,T0,C3
	    ld		[JTBASE+T1],T1
	    andn	COEFF,63,COEFF
	    jmp		JTBASE+T1
	    inc		4,I

Lcase35r3:
	    ld		[COEFF + 4*4],T0
	    ld		[COEFFLIST+I],COEFF
	    sub		C0,T0,C0
	    and		COEFF,63,T1
	    sub		C1,T0,C1
	    sll		T1,2,T1
	    sub		C2,T0,C2
	    sub		C3,T0,C3
	    ld		[JTBASE+T1],T1
	    andn	COEFF,63,COEFF
	    jmp		JTBASE+T1
	    inc		4,I

Lcase36r0:
	    ld		[COEFF + 4*23],T0
	    ld		[COEFF + 4*26],T1
	    add		D0,T0,D0
	    add		D1,T1,D1
	    ldd		[COEFF + 4*28],T0
	    ld		[COEFFLIST+I],COEFF
	    add		D2,T0,D2
	    and		COEFF,63,T0
	    sll		T0,2,T0
	    add		D3,T1,D3
	    ld		[JTBASE+T0],T0
	    andn	COEFF,63,COEFF
	    jmp		JTBASE+T0
	    inc		4,I

Lcase36r1:
	    ld		[COEFF + 4*22],T0
	    ld		[COEFF + 4*25],T1
	    sub		D0,T0,D0
	    sub		D1,T1,D1
	    ld		[COEFF + 4*27],T0
	    ld		[COEFF + 4*28],T1
	    ld		[COEFFLIST+I],COEFF
	    sub		D2,T0,D2
	    and		COEFF,63,T0
	    sll		T0,2,T0
	    sub		D3,T1,D3
	    ld		[JTBASE+T0],T0
	    andn	COEFF,63,COEFF
	    jmp		JTBASE+T0
	    inc		4,I

Lcase36r2:
	    ld		[COEFF + 4*21],T0
	    add		D0,T0,D0
	    ldd		[COEFF + 4*24],T0
	    add		D1,T0,D1
	    add		D2,T1,D2
	    ld		[COEFF + 4*26],T0
	    ld		[COEFFLIST+I],COEFF
	    add		D3,T0,D3
	    and		COEFF,63,T0
	    sll		T0,2,T0
	    ld		[JTBASE+T0],T0
	    andn	COEFF,63,COEFF
	    jmp		JTBASE+T0
	    inc		4,I

Lcase36r3:
	    ldd		[COEFF + 4*20],T0
	    sub		D0,T0,D0
	    sub		D1,T1,D1
	    ldd		[COEFF + 4*22],T0
	    ld		[COEFFLIST+I],COEFF
	    sub		D2,T0,D2
	    and		COEFF,63,T0
	    sll		T0,2,T0
	    sub		D3,T1,D3
	    ld		[JTBASE+T0],T0
	    andn	COEFF,63,COEFF
	    jmp		JTBASE+T0
	    inc		4,I

Lcase37r0:
	    ld		[COEFF + 4*18],T0
	    ld		[COEFF + 4*19],T1
	    add		A0,T0,A0
	    sub		A3,T0,A3
	    ld		[COEFFLIST+I],COEFF
	    add		A1,T1,A1
	    and		COEFF,63,T0
	    sll		T0,2,T0
	    sub		A2,T1,A2
	    ld		[JTBASE+T0],T0
	    andn	COEFF,63,COEFF
	    jmp		JTBASE+T0
	    inc		4,I

Lcase37r1:
	    ld		[COEFF + 4*16],T0
	    ld		[COEFF + 4*17],T1
	    sub		A0,T0,A0
	    add		A3,T0,A3
	    ld		[COEFFLIST+I],COEFF
	    sub		A1,T1,A1
	    and		COEFF,63,T0
	    sll		T0,2,T0
	    add		A2,T1,A2
	    ld		[JTBASE+T0],T0
	    andn	COEFF,63,COEFF
	    jmp		JTBASE+T0
	    inc		4,I

Lcase37r2:
	    ld		[COEFF + 4*16],T0
	    ld		[COEFF + 4*17],T1
	    add		A0,T0,A0
	    sub		A3,T0,A3
	    ld		[COEFFLIST+I],COEFF
	    add		A1,T1,A1
	    and		COEFF,63,T0
	    sll		T0,2,T0
	    sub		A2,T1,A2
	    ld		[JTBASE+T0],T0
	    andn	COEFF,63,COEFF
	    jmp		JTBASE+T0
	    inc		4,I

Lcase37r3:
	    ld		[COEFF + 4*18],T0
	    ld		[COEFF + 4*19],T1
	    sub		A0,T0,A0
	    add		A3,T0,A3
	    ld		[COEFFLIST+I],COEFF
	    sub		A1,T1,A1
	    and		COEFF,63,T0
	    sll		T0,2,T0
	    add		A2,T1,A2
	    ld		[JTBASE+T0],T0
	    andn	COEFF,63,COEFF
	    jmp		JTBASE+T0
	    inc		4,I

Lcase38r0:
	    ld		[COEFF + 4*22],T0
	    ld		[COEFF + 4*25],T1
	    sub		D2,T0,D2
	    add		D0,T1,D0
	    ld		[COEFF + 4*27],T0
	    ld		[COEFF + 4*28],T1
	    ld		[COEFFLIST+I],COEFF
	    sub		D3,T0,D3
	    and		COEFF,63,T0
	    sll		T0,2,T0
	    sub		D1,T1,D1
	    ld		[JTBASE+T0],T0
	    andn	COEFF,63,COEFF
	    jmp		JTBASE+T0
	    inc		4,I

Lcase38r1:
	    ldd		[COEFF + 4*20],T0
	    add		D2,T0,D2
	    sub		D0,T1,D0
	    ldd		[COEFF + 4*22],T0
	    ld		[COEFFLIST+I],COEFF
	    add		D3,T0,D3
	    and		COEFF,63,T0
	    sll		T0,2,T0
	    add		D1,T1,D1
	    ld		[JTBASE+T0],T0
	    andn	COEFF,63,COEFF
	    jmp		JTBASE+T0
	    inc		4,I

Lcase38r2:
	    ld		[COEFF + 4*23],T0
	    ld		[COEFF + 4*26],T1
	    sub		D2,T0,D2
	    add		D0,T1,D0
	    ldd		[COEFF + 4*28],T0
	    ld		[COEFFLIST+I],COEFF
	    sub		D3,T0,D3
	    and		COEFF,63,T0
	    sll		T0,2,T0
	    sub		D1,T1,D1
	    ld		[JTBASE+T0],T0
	    andn	COEFF,63,COEFF
	    jmp		JTBASE+T0
	    inc		4,I

Lcase38r3:
	    ld		[COEFF + 4*21],T0
	    sub		D2,T0,D2
	    ldd		[COEFF + 4*24],T0
	    add		D0,T0,D0
	    sub		D3,T1,D3
	    ld		[COEFF + 4*26],T0
	    ld		[COEFFLIST+I],COEFF
	    sub		D1,T0,D1
	    and		COEFF,63,T0
	    sll		T0,2,T0
	    ld		[JTBASE+T0],T0
	    andn	COEFF,63,COEFF
	    jmp		JTBASE+T0
	    inc		4,I

Lcase39r0:
Lcase39r3:
	    ld		[COEFF + 4*0],T0
	    ld		[COEFFLIST+I],COEFF
	    add		A0,T0,A0
	    and		COEFF,63,T1
	    sub		A1,T0,A1
	    sll		T1,2,T1
	    sub		A2,T0,A2
	    add		A3,T0,A3
	    ld		[JTBASE+T1],T1
	    andn	COEFF,63,COEFF
	    jmp		JTBASE+T1
	    inc		4,I

Lcase39r1:
Lcase39r2:
	    ld		[COEFF + 4*0],T0
	    ld		[COEFFLIST+I],COEFF
	    sub		A0,T0,A0
	    and		COEFF,63,T1
	    add		A1,T0,A1
	    sll		T1,2,T1
	    add		A2,T0,A2
	    sub		A3,T0,A3
	    ld		[JTBASE+T1],T1
	    andn	COEFF,63,COEFF
	    jmp		JTBASE+T1
	    inc		4,I

Lcase40r0:
	    ld		[COEFF + 4*21],T0
	    ld		[COEFF + 4*24],T1
	    sub		D1,T0,D1
	    add		D3,T1,D3
	    ld		[COEFF + 4*25],T0
	    ld		[COEFF + 4*26],T1
	    ld		[COEFFLIST+I],COEFF
	    add		D0,T0,D0
	    and		COEFF,63,T0
	    sll		T0,2,T0
	    add		D2,T1,D2
	    ld		[JTBASE+T0],T0
	    andn	COEFF,63,COEFF
	    jmp		JTBASE+T0
	    inc		4,I

Lcase40r1:
	    ld		[COEFF + 4*23],T0
	    ld		[COEFF + 4*26],T1
	    add		D1,T0,D1
	    sub		D3,T1,D3
	    ldd		[COEFF + 4*28],T0
	    ld		[COEFFLIST+I],COEFF
	    sub		D0,T0,D0
	    and		COEFF,63,T0
	    sll		T0,2,T0
	    sub		D2,T1,D2
	    ld		[JTBASE+T0],T0
	    andn	COEFF,63,COEFF
	    jmp		JTBASE+T0
	    inc		4,I

Lcase40r2:
	    ldd		[COEFF + 4*20],T0
	    add		D1,T0,D1
	    sub		D3,T1,D3
	    ldd		[COEFF + 4*22],T0
	    ld		[COEFFLIST+I],COEFF
	    sub		D0,T0,D0
	    and		COEFF,63,T0
	    sll		T0,2,T0
	    sub		D2,T1,D2
	    ld		[JTBASE+T0],T0
	    andn	COEFF,63,COEFF
	    jmp		JTBASE+T0
	    inc		4,I

Lcase40r3:
	    ld		[COEFF + 4*22],T0
	    ld		[COEFF + 4*25],T1
	    add		D1,T0,D1
	    sub		D3,T1,D3
	    ld		[COEFF + 4*27],T0
	    ld		[COEFF + 4*28],T1
	    ld		[COEFFLIST+I],COEFF
	    sub		D0,T0,D0
	    and		COEFF,63,T0
	    sll		T0,2,T0
	    sub		D2,T1,D2
	    ld		[JTBASE+T0],T0
	    andn	COEFF,63,COEFF
	    jmp		JTBASE+T0
	    inc		4,I

Lcase41r0:
	    ldd		[COEFF + 4*16],T0
	    ld		[COEFFLIST+I],COEFF
	    sub		A1,T0,A1
	    add		A2,T0,A2
	    add		A0,T1,A0
	    and		COEFF,63,T0
	    sll		T0,2,T0
	    sub		A3,T1,A3
	    ld		[JTBASE+T0],T0
	    andn	COEFF,63,COEFF
	    jmp		JTBASE+T0
	    inc		4,I

Lcase41r1:
	    ldd		[COEFF + 4*18],T0
	    sub		A1,T0,A1
	    add		A2,T0,A2
	    ld		[COEFFLIST+I],COEFF
	    add		A0,T1,A0
	    and		COEFF,63,T0
	    sll		T0,2,T0
	    sub		A3,T1,A3
	    ld		[JTBASE+T0],T0
	    andn	COEFF,63,COEFF
	    jmp		JTBASE+T0
	    inc		4,I

Lcase41r2:
	    ldd		[COEFF + 4*18],T0
	    add		A1,T0,A1
	    sub		A2,T0,A2
	    ld		[COEFFLIST+I],COEFF
	    sub		A0,T1,A0
	    and		COEFF,63,T0
	    sll		T0,2,T0
	    add		A3,T1,A3
	    ld		[JTBASE+T0],T0
	    andn	COEFF,63,COEFF
	    jmp		JTBASE+T0
	    inc		4,I

Lcase41r3:
	    ldd		[COEFF + 4*16],T0
	    add		A1,T0,A1
	    sub		A2,T0,A2
	    ld		[COEFFLIST+I],COEFF
	    sub		A0,T1,A0
	    and		COEFF,63,T0
	    sll		T0,2,T0
	    add		A3,T1,A3
	    ld		[JTBASE+T0],T0
	    andn	COEFF,63,COEFF
	    jmp		JTBASE+T0
	    inc		4,I

Lcase42r0:
	    ldd		[COEFF + 4*20],T0
	    sub		D3,T0,D3
	    add		D2,T1,D2
	    ldd		[COEFF + 4*22],T0
	    ld		[COEFFLIST+I],COEFF
	    sub		D1,T0,D1
	    and		COEFF,63,T0
	    sll		T0,2,T0
	    add		D0,T1,D0
	    ld		[JTBASE+T0],T0
	    andn	COEFF,63,COEFF
	    jmp		JTBASE+T0
	    inc		4,I

Lcase42r1:
	    ld		[COEFF + 4*21],T0
	    ld		[COEFF + 4*24],T1
	    sub		D3,T0,D3
	    add		D2,T1,D2
	    ld		[COEFF + 4*25],T0
	    ld		[COEFF + 4*26],T1
	    ld		[COEFFLIST+I],COEFF
	    sub		D1,T0,D1
	    and		COEFF,63,T0
	    sll		T0,2,T0
	    add		D0,T1,D0
	    ld		[JTBASE+T0],T0
	    andn	COEFF,63,COEFF
	    jmp		JTBASE+T0
	    inc		4,I

Lcase42r2:
	    ld		[COEFF + 4*22],T0
	    ld		[COEFF + 4*25],T1
	    sub		D3,T0,D3
	    add		D2,T1,D2
	    ld		[COEFF + 4*27],T0
	    ld		[COEFF + 4*28],T1
	    ld		[COEFFLIST+I],COEFF
	    sub		D1,T0,D1
	    and		COEFF,63,T0
	    sll		T0,2,T0
	    add		D0,T1,D0
	    ld		[JTBASE+T0],T0
	    andn	COEFF,63,COEFF
	    jmp		JTBASE+T0
	    inc		4,I

Lcase42r3:
	    ld		[COEFF + 4*23],T0
	    ld		[COEFF + 4*26],T1
	    sub		D3,T0,D3
	    add		D2,T1,D2
	    ldd		[COEFF + 4*28],T0
	    ld		[COEFFLIST+I],COEFF
	    sub		D1,T0,D1
	    and		COEFF,63,T0
	    sll		T0,2,T0
	    add		D0,T1,D0
	    ld		[JTBASE+T0],T0
	    andn	COEFF,63,COEFF
	    jmp		JTBASE+T0
	    inc		4,I

Lcase43r0:
	    ldd		[COEFF + 4*8],T0
	    sub		B3,T0,B3
	    add		B2,T1,B2
	    ldd		[COEFF + 4*10],T0
	    ld		[COEFFLIST+I],COEFF
	    sub		B1,T0,B1
	    and		COEFF,63,T0
	    sll		T0,2,T0
	    add		B0,T1,B0
	    ld		[JTBASE+T0],T0
	    andn	COEFF,63,COEFF
	    jmp		JTBASE+T0
	    inc		4,I

Lcase43r1:
	    ldd		[COEFF + 4*12],T0
	    sub		B3,T0,B3
	    add		B2,T1,B2
	    ldd		[COEFF + 4*14],T0
	    ld		[COEFFLIST+I],COEFF
	    sub		B1,T0,B1
	    and		COEFF,63,T0
	    sll		T0,2,T0
	    add		B0,T1,B0
	    ld		[JTBASE+T0],T0
	    andn	COEFF,63,COEFF
	    jmp		JTBASE+T0
	    inc		4,I

Lcase43r2:
	    ldd		[COEFF + 4*12],T0
	    add		B3,T0,B3
	    sub		B2,T1,B2
	    ldd		[COEFF + 4*14],T0
	    ld		[COEFFLIST+I],COEFF
	    add		B1,T0,B1
	    and		COEFF,63,T0
	    sll		T0,2,T0
	    sub		B0,T1,B0
	    ld		[JTBASE+T0],T0
	    andn	COEFF,63,COEFF
	    jmp		JTBASE+T0
	    inc		4,I

Lcase43r3:
	    ldd		[COEFF + 4*8],T0
	    add		B3,T0,B3
	    sub		B2,T1,B2
	    ldd		[COEFF + 4*10],T0
	    ld		[COEFFLIST+I],COEFF
	    add		B1,T0,B1
	    and		COEFF,63,T0
	    sll		T0,2,T0
	    sub		B0,T1,B0
	    ld		[JTBASE+T0],T0
	    andn	COEFF,63,COEFF
	    jmp		JTBASE+T0
	    inc		4,I

Lcase44r0:
	    ld		[COEFF + 4*9],T0
	    ld		[COEFF + 4*13],T1
	    sub		C1,T0,C1
	    add		C2,T0,C2
	    ld		[COEFFLIST+I],COEFF
	    add		C0,T1,C0
	    and		COEFF,63,T0
	    sll		T0,2,T0
	    sub		C3,T1,C3
	    ld		[JTBASE+T0],T0
	    andn	COEFF,63,COEFF
	    jmp		JTBASE+T0
	    inc		4,I

Lcase44r1:
	    ld		[COEFF + 4*11],T0
	    ld		[COEFF + 4*15],T1
	    add		C1,T0,C1
	    sub		C2,T0,C2
	    ld		[COEFFLIST+I],COEFF
	    sub		C0,T1,C0
	    and		COEFF,63,T0
	    sll		T0,2,T0
	    add		C3,T1,C3
	    ld		[JTBASE+T0],T0
	    andn	COEFF,63,COEFF
	    jmp		JTBASE+T0
	    inc		4,I

Lcase44r2:
	    ld		[COEFF + 4*8],T0
	    ld		[COEFF + 4*12],T1
	    add		C1,T0,C1
	    sub		C2,T0,C2
	    ld		[COEFFLIST+I],COEFF
	    sub		C0,T1,C0
	    and		COEFF,63,T0
	    sll		T0,2,T0
	    add		C3,T1,C3
	    ld		[JTBASE+T0],T0
	    andn	COEFF,63,COEFF
	    jmp		JTBASE+T0
	    inc		4,I

Lcase44r3:
	    ld		[COEFF + 4*10],T0
	    ld		[COEFF + 4*14],T1
	    add		C1,T0,C1
	    sub		C2,T0,C2
	    ld		[COEFFLIST+I],COEFF
	    sub		C0,T1,C0
	    and		COEFF,63,T0
	    sll		T0,2,T0
	    add		C3,T1,C3
	    ld		[JTBASE+T0],T0
	    andn	COEFF,63,COEFF
	    jmp		JTBASE+T0
	    inc		4,I

Lcase45r0:
Lcase45r3:
	    ldd		[COEFF + 4*4],T0
	    sub		B1,T0,B1
	    add		B3,T1,B3
	    ldd		[COEFF + 4*6],T0
	    ld		[COEFFLIST+I],COEFF
	    add		B0,T0,B0
	    and		COEFF,63,T0
	    sll		T0,2,T0
	    add		B2,T1,B2
	    ld		[JTBASE+T0],T0
	    andn	COEFF,63,COEFF
	    jmp		JTBASE+T0
	    inc		4,I

Lcase45r1:
Lcase45r2:
	    ldd		[COEFF + 4*4],T0
	    add		B1,T0,B1
	    sub		B3,T1,B3
	    ldd		[COEFF + 4*6],T0
	    ld		[COEFFLIST+I],COEFF
	    sub		B0,T0,B0
	    and		COEFF,63,T0
	    sll		T0,2,T0
	    sub		B2,T1,B2
	    ld		[JTBASE+T0],T0
	    andn	COEFF,63,COEFF
	    jmp		JTBASE+T0
	    inc		4,I

Lcase46r0:
	    ld		[COEFF + 4*6],T0
	    ld		[COEFFLIST+I],COEFF
	    add		C0,T0,C0
	    and		COEFF,63,T1
	    sub		C1,T0,C1
	    sll		T1,2,T1
	    sub		C2,T0,C2
	    add		C3,T0,C3
	    ld		[JTBASE+T1],T1
	    andn	COEFF,63,COEFF
	    jmp		JTBASE+T1
	    inc		4,I

Lcase46r1:
	    ld		[COEFF + 4*4],T0
	    ld		[COEFFLIST+I],COEFF
	    sub		C0,T0,C0
	    and		COEFF,63,T1
	    add		C1,T0,C1
	    sll		T1,2,T1
	    add		C2,T0,C2
	    sub		C3,T0,C3
	    ld		[JTBASE+T1],T1
	    andn	COEFF,63,COEFF
	    jmp		JTBASE+T1
	    inc		4,I

Lcase46r2:
	    ld		[COEFF + 4*7],T0
	    ld		[COEFFLIST+I],COEFF
	    add		C0,T0,C0
	    and		COEFF,63,T1
	    sub		C1,T0,C1
	    sll		T1,2,T1
	    sub		C2,T0,C2
	    add		C3,T0,C3
	    ld		[JTBASE+T1],T1
	    andn	COEFF,63,COEFF
	    jmp		JTBASE+T1
	    inc		4,I

Lcase46r3:
	    ld		[COEFF + 4*5],T0
	    ld		[COEFFLIST+I],COEFF
	    add		C0,T0,C0
	    and		COEFF,63,T1
	    sub		C1,T0,C1
	    sll		T1,2,T1
	    sub		C2,T0,C2
	    add		C3,T0,C3
	    ld		[JTBASE+T1],T1
	    andn	COEFF,63,COEFF
	    jmp		JTBASE+T1
	    inc		4,I

Lcase47r0:
	    ldd		[COEFF + 4*12],T0
	    sub		B2,T0,B2
	    add		B0,T1,B0
	    ldd		[COEFF + 4*14],T0
	    ld		[COEFFLIST+I],COEFF
	    sub		B3,T0,B3
	    and		COEFF,63,T0
	    sll		T0,2,T0
	    sub		B1,T1,B1
	    ld		[JTBASE+T0],T0
	    andn	COEFF,63,COEFF
	    jmp		JTBASE+T0
	    inc		4,I

Lcase47r1:
	    ldd		[COEFF + 4*8],T0
	    add		B2,T0,B2
	    sub		B0,T1,B0
	    ldd		[COEFF + 4*10],T0
	    ld		[COEFFLIST+I],COEFF
	    add		B3,T0,B3
	    and		COEFF,63,T0
	    sll		T0,2,T0
	    add		B1,T1,B1
	    ld		[JTBASE+T0],T0
	    andn	COEFF,63,COEFF
	    jmp		JTBASE+T0
	    inc		4,I

Lcase47r2:
	    ldd		[COEFF + 4*8],T0
	    sub		B2,T0,B2
	    add		B0,T1,B0
	    ldd		[COEFF + 4*10],T0
	    ld		[COEFFLIST+I],COEFF
	    sub		B3,T0,B3
	    and		COEFF,63,T0
	    sll		T0,2,T0
	    sub		B1,T1,B1
	    ld		[JTBASE+T0],T0
	    andn	COEFF,63,COEFF
	    jmp		JTBASE+T0
	    inc		4,I

Lcase47r3:
	    ldd		[COEFF + 4*12],T0
	    add		B2,T0,B2
	    sub		B0,T1,B0
	    ldd		[COEFF + 4*14],T0
	    ld		[COEFFLIST+I],COEFF
	    add		B3,T0,B3
	    and		COEFF,63,T0
	    sll		T0,2,T0
	    add		B1,T1,B1
	    ld		[JTBASE+T0],T0
	    andn	COEFF,63,COEFF
	    jmp		JTBASE+T0
	    inc		4,I

Lcase48r0:
	    ld		[COEFF + 4*11],T0
	    ld		[COEFF + 4*15],T1
	    add		C0,T0,C0
	    sub		C3,T0,C3
	    ld		[COEFFLIST+I],COEFF
	    add		C1,T1,C1
	    and		COEFF,63,T0
	    sll		T0,2,T0
	    sub		C2,T1,C2
	    ld		[JTBASE+T0],T0
	    andn	COEFF,63,COEFF
	    jmp		JTBASE+T0
	    inc		4,I

Lcase48r1:
	    ld		[COEFF + 4*10],T0
	    ld		[COEFF + 4*14],T1
	    sub		C0,T0,C0
	    add		C3,T0,C3
	    ld		[COEFFLIST+I],COEFF
	    sub		C1,T1,C1
	    and		COEFF,63,T0
	    sll		T0,2,T0
	    add		C2,T1,C2
	    ld		[JTBASE+T0],T0
	    andn	COEFF,63,COEFF
	    jmp		JTBASE+T0
	    inc		4,I

Lcase48r2:
	    ld		[COEFF + 4*9],T0
	    ld		[COEFF + 4*13],T1
	    add		C0,T0,C0
	    sub		C3,T0,C3
	    ld		[COEFFLIST+I],COEFF
	    add		C1,T1,C1
	    and		COEFF,63,T0
	    sll		T0,2,T0
	    sub		C2,T1,C2
	    ld		[JTBASE+T0],T0
	    andn	COEFF,63,COEFF
	    jmp		JTBASE+T0
	    inc		4,I

Lcase48r3:
	    ld		[COEFF + 4*8],T0
	    ld		[COEFF + 4*12],T1
	    sub		C0,T0,C0
	    add		C3,T0,C3
	    ld		[COEFFLIST+I],COEFF
	    sub		C1,T1,C1
	    and		COEFF,63,T0
	    sll		T0,2,T0
	    add		C2,T1,C2
	    ld		[JTBASE+T0],T0
	    andn	COEFF,63,COEFF
	    jmp		JTBASE+T0
	    inc		4,I

Lcase49r0:
	    ld		[COEFF + 4*23],T0
	    ld		[COEFF + 4*26],T1
	    sub		D2,T0,D2
	    add		D0,T1,D0
	    ldd		[COEFF + 4*28],T0
	    ld		[COEFFLIST+I],COEFF
	    sub		D3,T0,D3
	    and		COEFF,63,T0
	    sll		T0,2,T0
	    sub		D1,T1,D1
	    ld		[JTBASE+T0],T0
	    andn	COEFF,63,COEFF
	    jmp		JTBASE+T0
	    inc		4,I

Lcase49r1:
	    ld		[COEFF + 4*22],T0
	    ld		[COEFF + 4*25],T1
	    add		D2,T0,D2
	    sub		D0,T1,D0
	    ld		[COEFF + 4*27],T0
	    ld		[COEFF + 4*28],T1
	    ld		[COEFFLIST+I],COEFF
	    add		D3,T0,D3
	    and		COEFF,63,T0
	    sll		T0,2,T0
	    add		D1,T1,D1
	    ld		[JTBASE+T0],T0
	    andn	COEFF,63,COEFF
	    jmp		JTBASE+T0
	    inc		4,I

Lcase49r2:
	    ld		[COEFF + 4*21],T0
	    sub		D2,T0,D2
	    ldd		[COEFF + 4*24],T0
	    add		D0,T0,D0
	    sub		D3,T1,D3
	    ld		[COEFF + 4*26],T1
	    ld		[COEFFLIST+I],COEFF
	    sub		D1,T1,D1
	    and		COEFF,63,T0
	    sll		T0,2,T0
	    ld		[JTBASE+T0],T0
	    andn	COEFF,63,COEFF
	    jmp		JTBASE+T0
	    inc		4,I

Lcase49r3:
	    ldd		[COEFF + 4*20],T0
	    add		D2,T0,D2
	    sub		D0,T1,D0
	    ldd		[COEFF + 4*22],T0
	    ld		[COEFFLIST+I],COEFF
	    add		D3,T0,D3
	    and		COEFF,63,T0
	    sll		T0,2,T0
	    add		D1,T1,D1
	    ld		[JTBASE+T0],T0
	    andn	COEFF,63,COEFF
	    jmp		JTBASE+T0
	    inc		4,I

Lcase50r0:
	    ld		[COEFF + 4*3],T0
	    ld		[COEFFLIST+I],COEFF
	    add		A0,T0,A0
	    and		COEFF,63,T1
	    sub		A1,T0,A1
	    sll		T1,2,T1
	    sub		A2,T0,A2
	    add		A3,T0,A3
	    ld		[JTBASE+T1],T1
	    andn	COEFF,63,COEFF
	    jmp		JTBASE+T1
	    inc		4,I

Lcase50r1:
	    ld		[COEFF + 4*2],T0
	    ld		[COEFFLIST+I],COEFF
	    sub		A0,T0,A0
	    and		COEFF,63,T1
	    add		A1,T0,A1
	    sll		T1,2,T1
	    add		A2,T0,A2
	    sub		A3,T0,A3
	    ld		[JTBASE+T1],T1
	    andn	COEFF,63,COEFF
	    jmp		JTBASE+T1
	    inc		4,I

Lcase50r2:
	    ld		[COEFF + 4*2],T0
	    ld		[COEFFLIST+I],COEFF
	    add		A0,T0,A0
	    and		COEFF,63,T1
	    sub		A1,T0,A1
	    sll		T1,2,T1
	    sub		A2,T0,A2
	    add		A3,T0,A3
	    ld		[JTBASE+T1],T1
	    andn	COEFF,63,COEFF
	    jmp		JTBASE+T1
	    inc		4,I

Lcase50r3:
	    ld		[COEFF + 4*3],T0
	    ld		[COEFFLIST+I],COEFF
	    sub		A0,T0,A0
	    and		COEFF,63,T1
	    add		A1,T0,A1
	    sll		T1,2,T1
	    add		A2,T0,A2
	    sub		A3,T0,A3
	    ld		[JTBASE+T1],T1
	    andn	COEFF,63,COEFF
	    jmp		JTBASE+T1
	    inc		4,I

Lcase51r0:
	    ld		[COEFF + 4*22],T0
	    ld		[COEFF + 4*25],T1
	    sub		D1,T0,D1
	    add		D3,T1,D3
	    ld		[COEFF + 4*27],T0
	    ld		[COEFF + 4*28],T1
	    ld		[COEFFLIST+I],COEFF
	    add		D0,T0,D0
	    and		COEFF,63,T0
	    sll		T0,2,T0
	    add		D2,T1,D2
	    ld		[JTBASE+T0],T0
	    andn	COEFF,63,COEFF
	    jmp		JTBASE+T0
	    inc		4,I

Lcase51r1:
	    ldd		[COEFF + 4*20],T0
	    add		D1,T0,D1
	    sub		D3,T1,D3
	    ldd		[COEFF + 4*22],T0
	    ld		[COEFFLIST+I],COEFF
	    sub		D0,T0,D0
	    and		COEFF,63,T0
	    sll		T0,2,T0
	    sub		D2,T1,D2
	    ld		[JTBASE+T0],T0
	    andn	COEFF,63,COEFF
	    jmp		JTBASE+T0
	    inc		4,I

Lcase51r2:
	    ld		[COEFF + 4*23],T0
	    ld		[COEFF + 4*26],T1
	    sub		D1,T0,D1
	    add		D3,T1,D3
	    ldd		[COEFF + 4*28],T0
	    ld		[COEFFLIST+I],COEFF
	    add		D0,T0,D0
	    and		COEFF,63,T0
	    sll		T0,2,T0
	    add		D2,T1,D2
	    ld		[JTBASE+T0],T0
	    andn	COEFF,63,COEFF
	    jmp		JTBASE+T0
	    inc		4,I

Lcase51r3:
	    ld		[COEFF + 4*21],T0
	    sub		D1,T0,D1
	    ldd		[COEFF + 4*24],T0
	    add		D3,T0,D3
	    add		D0,T1,D0
	    ld		[COEFF + 4*26],T0
	    ld		[COEFFLIST+I],COEFF
	    add		D2,T0,D2
	    and		COEFF,63,T0
	    sll		T0,2,T0
	    ld		[JTBASE+T0],T0
	    andn	COEFF,63,COEFF
	    jmp		JTBASE+T0
	    inc		4,I

Lcase52r0:
Lcase52r3:
	    ldd		[COEFF + 4*2],T0
	    ld		[COEFFLIST+I],COEFF
	    sub		A1,T0,A1
	    add		A2,T0,A2
	    and		COEFF,63,T0
	    add		A0,T1,A0
	    sll		T0,2,T0
	    sub		A3,T1,A3
	    ld		[JTBASE+T0],T0
	    andn	COEFF,63,COEFF
	    jmp		JTBASE+T0
	    inc		4,I

Lcase52r1:
Lcase52r2:
	    ldd		[COEFF + 4*2],T0
	    ld		[COEFFLIST+I],COEFF
	    add		A1,T0,A1
	    sub		A2,T0,A2
	    and		COEFF,63,T0
	    sub		A0,T1,A0
	    sll		T0,2,T0
	    add		A3,T1,A3
	    ld		[JTBASE+T0],T0
	    andn	COEFF,63,COEFF
	    jmp		JTBASE+T0
	    inc		4,I

Lcase53r0:
	    ld		[COEFF + 4*21],T0
	    sub		D3,T0,D3
	    ldd		[COEFF + 4*24],T0
	    add		D2,T0,D2
	    sub		D1,T1,D1
	    ld		[COEFF + 4*26],T0
	    ld		[COEFFLIST+I],COEFF
	    add		D0,T0,D0
	    and		COEFF,63,T0
	    sll		T0,2,T0
	    ld		[JTBASE+T0],T0
	    andn	COEFF,63,COEFF
	    jmp		JTBASE+T0
	    inc		4,I

Lcase53r1:
	    ld		[COEFF + 4*23],T0
	    ld		[COEFF + 4*26],T1
	    add		D3,T0,D3
	    sub		D2,T1,D2
	    ldd		[COEFF + 4*28],T0
	    ld		[COEFFLIST+I],COEFF
	    add		D1,T0,D1
	    and		COEFF,63,T0
	    sll		T0,2,T0
	    sub		D0,T1,D0
	    ld		[JTBASE+T0],T0
	    andn	COEFF,63,COEFF
	    jmp		JTBASE+T0
	    inc		4,I

Lcase53r2:
	    ldd		[COEFF + 4*20],T0
	    add		D3,T0,D3
	    sub		D2,T1,D2
	    ldd		[COEFF + 4*22],T0
	    ld		[COEFFLIST+I],COEFF
	    add		D1,T0,D1
	    and		COEFF,63,T0
	    sll		T0,2,T0
	    sub		D0,T1,D0
	    ld		[JTBASE+T0],T0
	    andn	COEFF,63,COEFF
	    jmp		JTBASE+T0
	    inc		4,I

Lcase53r3:
	    ld		[COEFF + 4*22],T0
	    ld		[COEFF + 4*25],T1
	    add		D3,T0,D3
	    sub		D2,T1,D2
	    ld		[COEFF + 4*27],T0
	    ld		[COEFF + 4*28],T1
	    ld		[COEFFLIST+I],COEFF
	    add		D1,T0,D1
	    and		COEFF,63,T0
	    sll		T0,2,T0
	    sub		D0,T1,D0
	    ld		[JTBASE+T0],T0
	    andn	COEFF,63,COEFF
	    jmp		JTBASE+T0
	    inc		4,I

Lcase54r0:
Lcase54r3:
	    ldd		[COEFF + 4*4],T0
	    sub		B3,T0,B3
	    add		B2,T1,B2
	    ldd		[COEFF + 4*6],T0
	    ld		[COEFFLIST+I],COEFF
	    sub		B1,T0,B1
	    and		COEFF,63,T0
	    sll		T0,2,T0
	    add		B0,T1,B0
	    ld		[JTBASE+T0],T0
	    andn	COEFF,63,COEFF
	    jmp		JTBASE+T0
	    inc		4,I

Lcase54r1:
Lcase54r2:
	    ldd		[COEFF + 4*4],T0
	    add		B3,T0,B3
	    sub		B2,T1,B2
	    ldd		[COEFF + 4*6],T0
	    ld		[COEFFLIST+I],COEFF
	    add		B1,T0,B1
	    and		COEFF,63,T0
	    sll		T0,2,T0
	    sub		B0,T1,B0
	    ld		[JTBASE+T0],T0
	    andn	COEFF,63,COEFF
	    jmp		JTBASE+T0
	    inc		4,I

Lcase55r0:
	    ld		[COEFF + 4*10],T0
	    ld		[COEFF + 4*14],T1
	    sub		C1,T0,C1
	    add		C2,T0,C2
	    ld		[COEFFLIST+I],COEFF
	    add		C0,T1,C0
	    and		COEFF,63,T0
	    sll		T0,2,T0
	    sub		C3,T1,C3
	    ld		[JTBASE+T0],T0
	    andn	COEFF,63,COEFF
	    jmp		JTBASE+T0
	    inc		4,I

Lcase55r1:
	    ld		[COEFF + 4*8],T0
	    ld		[COEFF + 4*12],T1
	    add		C1,T0,C1
	    sub		C2,T0,C2
	    ld		[COEFFLIST+I],COEFF
	    sub		C0,T1,C0
	    and		COEFF,63,T0
	    sll		T0,2,T0
	    add		C3,T1,C3
	    ld		[JTBASE+T0],T0
	    andn	COEFF,63,COEFF
	    jmp		JTBASE+T0
	    inc		4,I

Lcase55r2:
	    ld		[COEFF + 4*11],T0
	    ld		[COEFF + 4*15],T1
	    sub		C1,T0,C1
	    add		C2,T0,C2
	    ld		[COEFFLIST+I],COEFF
	    add		C0,T1,C0
	    and		COEFF,63,T0
	    sll		T0,2,T0
	    sub		C3,T1,C3
	    ld		[JTBASE+T0],T0
	    andn	COEFF,63,COEFF
	    jmp		JTBASE+T0
	    inc		4,I

Lcase55r3:
	    ld		[COEFF + 4*9],T0
	    ld		[COEFF + 4*13],T1
	    sub		C1,T0,C1
	    add		C2,T0,C2
	    ld		[COEFFLIST+I],COEFF
	    add		C0,T1,C0
	    and		COEFF,63,T0
	    sll		T0,2,T0
	    sub		C3,T1,C3
	    ld		[JTBASE+T0],T0
	    andn	COEFF,63,COEFF
	    jmp		JTBASE+T0
	    inc		4,I

Lcase56r0:
	    ldd		[COEFF + 4*12],T0
	    sub		B1,T0,B1
	    add		B3,T1,B3
	    ldd		[COEFF + 4*14],T0
	    ld		[COEFFLIST+I],COEFF
	    add		B0,T0,B0
	    and		COEFF,63,T0
	    sll		T0,2,T0
	    add		B2,T1,B2
	    ld		[JTBASE+T0],T0
	    andn	COEFF,63,COEFF
	    jmp		JTBASE+T0
	    inc		4,I

Lcase56r1:
	    ldd		[COEFF + 4*8],T0
	    add		B1,T0,B1
	    sub		B3,T1,B3
	    ldd		[COEFF + 4*10],T0
	    ld		[COEFFLIST+I],COEFF
	    sub		B0,T0,B0
	    and		COEFF,63,T0
	    sll		T0,2,T0
	    sub		B2,T1,B2
	    ld		[JTBASE+T0],T0
	    andn	COEFF,63,COEFF
	    jmp		JTBASE+T0
	    inc		4,I

Lcase56r2:
	    ldd		[COEFF + 4*8],T0
	    sub		B1,T0,B1
	    add		B3,T1,B3
	    ldd		[COEFF + 4*10],T0
	    ld		[COEFFLIST+I],COEFF
	    add		B0,T0,B0
	    and		COEFF,63,T0
	    sll		T0,2,T0
	    add		B2,T1,B2
	    ld		[JTBASE+T0],T0
	    andn	COEFF,63,COEFF
	    jmp		JTBASE+T0
	    inc		4,I

Lcase56r3:
	    ldd		[COEFF + 4*12],T0
	    add		B1,T0,B1
	    sub		B3,T1,B3
	    ldd		[COEFF + 4*14],T0
	    ld		[COEFFLIST+I],COEFF
	    sub		B0,T0,B0
	    and		COEFF,63,T0
	    sll		T0,2,T0
	    sub		B2,T1,B2
	    ld		[JTBASE+T0],T0
	    andn	COEFF,63,COEFF
	    jmp		JTBASE+T0
	    inc		4,I

Lcase57r0:
	    ld		[COEFF + 4*7],T0
	    ld		[COEFFLIST+I],COEFF
	    add		C0,T0,C0
	    and		COEFF,63,T1
	    sll		T1,2,T1
	    sub		C1,T0,C1
	    sub		C2,T0,C2
	    add		C3,T0,C3
	    ld		[JTBASE+T1],T1
	    andn	COEFF,63,COEFF
	    jmp		JTBASE+T1
	    inc		4,I

Lcase57r1:
	    ld		[COEFF + 4*6],T0
	    ld		[COEFFLIST+I],COEFF
	    sub		C0,T0,C0
	    and		COEFF,63,T1
	    sll		T1,2,T1
	    add		C1,T0,C1
	    add		C2,T0,C2
	    sub		C3,T0,C3
	    ld		[JTBASE+T1],T1
	    andn	COEFF,63,COEFF
	    jmp		JTBASE+T1
	    inc		4,I

Lcase57r2:
	    ld		[COEFF + 4*5],T0
	    ld		[COEFFLIST+I],COEFF
	    add		C0,T0,C0
	    and		COEFF,63,T1
	    sll		T1,2,T1
	    sub		C1,T0,C1
	    sub		C2,T0,C2
	    add		C3,T0,C3
	    ld		[JTBASE+T1],T1
	    andn	COEFF,63,COEFF
	    jmp		JTBASE+T1
	    inc		4,I

Lcase57r3:
	    ld		[COEFF + 4*4],T0
	    ld		[COEFFLIST+I],COEFF
	    sub		C0,T0,C0
	    and		COEFF,63,T1
	    sll		T1,2,T1
	    add		C1,T0,C1
	    add		C2,T0,C2
	    sub		C3,T0,C3
	    ld		[JTBASE+T1],T1
	    andn	COEFF,63,COEFF
	    jmp		JTBASE+T1
	    inc		4,I

Lcase58r0:
	    ld		[COEFF + 4*23],T0
	    ld		[COEFF + 4*26],T1
	    sub		D1,T0,D1
	    add		D3,T1,D3
	    ldd		[COEFF + 4*28],T0
	    ld		[COEFFLIST+I],COEFF
	    add		D0,T0,D0
	    and		COEFF,63,T0
	    sll		T0,2,T0
	    add		D2,T1,D2
	    ld		[JTBASE+T0],T0
	    andn	COEFF,63,COEFF
	    jmp		JTBASE+T0
	    inc		4,I

Lcase58r1:
	    ld		[COEFF + 4*22],T0
	    ld		[COEFF + 4*25],T1
	    add		D1,T0,D1
	    sub		D3,T1,D3
	    ld		[COEFF + 4*27],T0
	    ld		[COEFF + 4*28],T1
	    ld		[COEFFLIST+I],COEFF
	    sub		D0,T0,D0
	    and		COEFF,63,T0
	    sll		T0,2,T0
	    sub		D2,T1,D2
	    ld		[JTBASE+T0],T0
	    andn	COEFF,63,COEFF
	    jmp		JTBASE+T0
	    inc		4,I

Lcase58r2:
	    ld		[COEFF + 4*21],T0
	    sub		D1,T0,D1
	    ldd		[COEFF + 4*24],T0
	    add		D3,T0,D3
	    add		D0,T1,D0
	    ld		[COEFF + 4*26],T0
	    ld		[COEFFLIST+I],COEFF
	    add		D2,T0,D2
	    and		COEFF,63,T0
	    sll		T0,2,T0
	    ld		[JTBASE+T0],T0
	    andn	COEFF,63,COEFF
	    jmp		JTBASE+T0
	    inc		4,I

Lcase58r3:
	    ldd		[COEFF + 4*20],T0
	    add		D1,T0,D1
	    sub		D3,T1,D3
	    ldd		[COEFF + 4*22],T0
	    ld		[COEFFLIST+I],COEFF
	    sub		D0,T0,D0
	    and		COEFF,63,T0
	    sll		T0,2,T0
	    sub		D2,T1,D2
	    ld		[JTBASE+T0],T0
	    andn	COEFF,63,COEFF
	    jmp		JTBASE+T0
	    inc		4,I

Lcase59r0:
	    ld		[COEFF + 4*18],T0
	    ld		[COEFF + 4*19],T1
	    sub		A1,T0,A1
	    add		A2,T0,A2
	    ld		[COEFFLIST+I],COEFF
	    add		A0,T1,A0
	    and		COEFF,63,T0
	    sll		T0,2,T0
	    sub		A3,T1,A3
	    ld		[JTBASE+T0],T0
	    andn	COEFF,63,COEFF
	    jmp		JTBASE+T0
	    inc		4,I

Lcase59r1:
	    ld		[COEFF + 4*16],T0
	    ld		[COEFF + 4*17],T1
	    add		A1,T0,A1
	    sub		A2,T0,A2
	    ld		[COEFFLIST+I],COEFF
	    sub		A0,T1,A0
	    and		COEFF,63,T0
	    sll		T0,2,T0
	    add		A3,T1,A3
	    ld		[JTBASE+T0],T0
	    andn	COEFF,63,COEFF
	    jmp		JTBASE+T0
	    inc		4,I

Lcase59r2:
	    ld		[COEFF + 4*16],T0
	    ld		[COEFF + 4*17],T1
	    sub		A1,T0,A1
	    add		A2,T0,A2
	    ld		[COEFFLIST+I],COEFF
	    add		A0,T1,A0
	    and		COEFF,63,T0
	    sll		T0,2,T0
	    sub		A3,T1,A3
	    ld		[JTBASE+T0],T0
	    andn	COEFF,63,COEFF
	    jmp		JTBASE+T0
	    inc		4,I

Lcase59r3:
	    ld		[COEFF + 4*18],T0
	    ld		[COEFF + 4*19],T1
	    add		A1,T0,A1
	    sub		A2,T0,A2
	    ld		[COEFFLIST+I],COEFF
	    sub		A0,T1,A0
	    and		COEFF,63,T0
	    sll		T0,2,T0
	    add		A3,T1,A3
	    ld		[JTBASE+T0],T0
	    andn	COEFF,63,COEFF
	    jmp		JTBASE+T0
	    inc		4,I

Lcase60r0:
	    ld		[COEFF + 4*22],T0
	    ld		[COEFF + 4*25],T1
	    sub		D3,T0,D3
	    add		D2,T1,D2
	    ld		[COEFF + 4*27],T0
	    ld		[COEFF + 4*28],T1
	    ld		[COEFFLIST+I],COEFF
	    sub		D1,T0,D1
	    and		COEFF,63,T0
	    sll		T0,2,T0
	    add		D0,T1,D0
	    ld		[JTBASE+T0],T0
	    andn	COEFF,63,COEFF
	    jmp		JTBASE+T0
	    inc		4,I

Lcase60r1:
	    ldd		[COEFF + 4*20],T0
	    add		D3,T0,D3
	    sub		D2,T1,D2
	    ldd		[COEFF + 4*22],T0
	    ld		[COEFFLIST+I],COEFF
	    add		D1,T0,D1
	    and		COEFF,63,T0
	    sll		T0,2,T0
	    sub		D0,T1,D0
	    ld		[JTBASE+T0],T0
	    andn	COEFF,63,COEFF
	    jmp		JTBASE+T0
	    inc		4,I

Lcase60r2:
	    ld		[COEFF + 4*23],T0
	    ld		[COEFF + 4*26],T1
	    sub		D3,T0,D3
	    add		D2,T1,D2
	    ldd		[COEFF + 4*28],T0
	    ld		[COEFFLIST+I],COEFF
	    sub		D1,T0,D1
	    and		COEFF,63,T0
	    sll		T0,2,T0
	    add		D0,T1,D0
	    ld		[JTBASE+T0],T0
	    andn	COEFF,63,COEFF
	    jmp		JTBASE+T0
	    inc		4,I

Lcase60r3:
	    ld		[COEFF + 4*21],T0
	    ld		[COEFF + 4*24],T1
	    sub		D3,T0,D3
	    add		D2,T1,D2
	    ld		[COEFF + 4*25],T0
	    ld		[COEFF + 4*26],T1
	    ld		[COEFFLIST+I],COEFF
	    sub		D1,T0,D1
	    and		COEFF,63,T0
	    sll		T0,2,T0
	    add		D0,T1,D0
	    ld		[JTBASE+T0],T0
	    andn	COEFF,63,COEFF
	    jmp		JTBASE+T0
	    inc		4,I

Lcase61r0:
	    ldd		[COEFF + 4*12],T0
	    sub		B3,T0,B3
	    add		B2,T1,B2
	    ldd		[COEFF + 4*14],T0
	    ld		[COEFFLIST+I],COEFF
	    sub		B1,T0,B1
	    and		COEFF,63,T0
	    sll		T0,2,T0
	    add		B0,T1,B0
	    ld		[JTBASE+T0],T0
	    andn	COEFF,63,COEFF
	    jmp		JTBASE+T0
	    inc		4,I

Lcase61r1:
	    ldd		[COEFF + 4*8],T0
	    add		B3,T0,B3
	    sub		B2,T1,B2
	    ldd		[COEFF + 4*10],T0
	    ld		[COEFFLIST+I],COEFF
	    add		B1,T0,B1
	    and		COEFF,63,T0
	    sll		T0,2,T0
	    sub		B0,T1,B0
	    ld		[JTBASE+T0],T0
	    andn	COEFF,63,COEFF
	    jmp		JTBASE+T0
	    inc		4,I

Lcase61r2:
	    ldd		[COEFF + 4*8],T0
	    sub		B3,T0,B3
	    add		B2,T1,B2
	    ldd		[COEFF + 4*10],T0
	    ld		[COEFFLIST+I],COEFF
	    sub		B1,T0,B1
	    and		COEFF,63,T0
	    sll		T0,2,T0
	    add		B0,T1,B0
	    ld		[JTBASE+T0],T0
	    andn	COEFF,63,COEFF
	    jmp		JTBASE+T0
	    inc		4,I

Lcase61r3:
	    ldd		[COEFF + 4*12],T0
	    add		B3,T0,B3
	    sub		B2,T1,B2
	    ldd		[COEFF + 4*14],T0
	    ld		[COEFFLIST+I],COEFF
	    add		B1,T0,B1
	    and		COEFF,63,T0
	    sll		T0,2,T0
	    sub		B0,T1,B0
	    ld		[JTBASE+T0],T0
	    andn	COEFF,63,COEFF
	    jmp		JTBASE+T0
	    inc		4,I

Lcase62r0:
	    ld		[COEFF + 4*11],T0
	    ld		[COEFF + 4*15],T1
	    sub		C1,T0,C1
	    add		C2,T0,C2
	    ld		[COEFFLIST+I],COEFF
	    add		C0,T1,C0
	    and		COEFF,63,T0
	    sll		T0,2,T0
	    sub		C3,T1,C3
	    ld		[JTBASE+T0],T0
	    andn	COEFF,63,COEFF
	    jmp		JTBASE+T0
	    inc		4,I

Lcase62r1:
	    ld		[COEFF + 4*10],T0
	    ld		[COEFF + 4*14],T1
	    add		C1,T0,C1
	    sub		C2,T0,C2
	    ld		[COEFFLIST+I],COEFF
	    sub		C0,T1,C0
	    and		COEFF,63,T0
	    sll		T0,2,T0
	    add		C3,T1,C3
	    ld		[JTBASE+T0],T0
	    andn	COEFF,63,COEFF
	    jmp		JTBASE+T0
	    inc		4,I

Lcase62r2:
	    ld		[COEFF + 4*9],T0
	    ld		[COEFF + 4*13],T1
	    sub		C1,T0,C1
	    add		C2,T0,C2
	    ld		[COEFFLIST+I],COEFF
	    add		C0,T1,C0
	    and		COEFF,63,T0
	    sll		T0,2,T0
	    sub		C3,T1,C3
	    ld		[JTBASE+T0],T0
	    andn	COEFF,63,COEFF
	    jmp		JTBASE+T0
	    inc		4,I

Lcase62r3:
	    ld		[COEFF + 4*8],T0
	    ld		[COEFF + 4*12],T1
	    add		C1,T0,C1
	    sub		C2,T0,C2
	    ld		[COEFFLIST+I],COEFF
	    sub		C0,T1,C0
	    and		COEFF,63,T0
	    sll		T0,2,T0
	    add		C3,T1,C3
	    ld		[JTBASE+T0],T0
	    andn	COEFF,63,COEFF
	    jmp		JTBASE+T0
	    inc		4,I

Lcase63r0:
	    ld		[COEFF + 4*23],T0
	    ld		[COEFF + 4*26],T1
	    sub		D3,T0,D3
	    add		D2,T1,D2
	    ld		[COEFF + 4*28],T0
	    ld		[COEFF + 4*29],T1
	    ld		[COEFFLIST+I],COEFF
	    sub		D1,T0,D1
	    and		COEFF,63,T0
	    sll		T0,2,T0
	    add		D0,T1,D0
	    ld		[JTBASE+T0],T0
	    andn	COEFF,63,COEFF
	    jmp		JTBASE+T0
	    inc		4,I

Lcase63r1:
	    ld		[COEFF + 4*22],T0
	    ld		[COEFF + 4*25],T1
	    add		D3,T0,D3
	    sub		D2,T1,D2
	    ld		[COEFF + 4*27],T0
	    ld		[COEFF + 4*28],T1
	    ld		[COEFFLIST+I],COEFF
	    add		D1,T0,D1
	    and		COEFF,63,T0
	    sll		T0,2,T0
	    sub		D0,T1,D0
	    ld		[JTBASE+T0],T0
	    andn	COEFF,63,COEFF
	    jmp		JTBASE+T0
	    inc		4,I


Lcase63r2:
	    ld		[COEFF + 4*21],T0
	    sub		D3,T0,D3
	    ldd		[COEFF + 4*24],T0
	    add		D2,T0,D2
	    sub		D1,T1,D1
	    ld		[COEFF + 4*26],T0
	    ld		[COEFFLIST+I],COEFF
	    add		D0,T0,D0
	    and		COEFF,63,T0
	    sll		T0,2,T0
	    ld		[JTBASE+T0],T0
	    andn	COEFF,63,COEFF
	    jmp		JTBASE+T0
	    inc		4,I

Lcase63r3:
	    ldd		[COEFF + 4*20],T0
	    add		D3,T0,D3
	    sub		D2,T1,D2
	    ldd		[COEFF + 4*22],T0
	    ld		[COEFFLIST+I],COEFF
	    add		D1,T0,D1
	    and		COEFF,63,T0
	    sll		T0,2,T0
	    sub		D0,T1,D0
	    ld		[JTBASE+T0],T0
	    andn	COEFF,63,COEFF
	    jmp		JTBASE+T0
	    inc		4,I
