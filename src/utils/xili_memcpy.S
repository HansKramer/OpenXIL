!------------------------------------------------------------------------
!
!  File:       xili_memcpy.S
!  Project:    XIL
!  Revision:   1.8
!  Last Mod:   16:17:57, 04/30/97
!
!  Description:
!
!    xil version of memcpy (does double loads and stores and unrolls loops)
!
!
!------------------------------------------------------------------------
!    COPYRIGHT
!------------------------------------------------------------------------
! @(#)xili_memcpy.S	1.8 97/04/30 &Q&

! variables used throughout
#define DIFF %i0
#define S2   %i1
#define N    %i2
#define SAVEDS1 %l0
#define DIFF1 %l1
#define DIFF2 %l2
#define DIFF3 %l3
#define DIFF4 %l4
#define DIFF5 %l5
#define DIFF6 %l6
#define DIFF7 %l7
#define MISALIGN %o5

#define MINFAST 32
#define TMP8 %o1
#define SRC %o2
#define DST %o3
#define DSTEND %o4

#include <sys/asm_linkage.h>
	.section ".text"

	ENTRY(xili_memcpy)
	save	%sp,-SA(MINFRAME),%sp
! if N < minfast then just move bytes
! there is code that depends on the value 32, so don't make it lower.
        mov     DIFF,SAVEDS1
	cmp	N,MINFAST
	bge	L_PRELOOP
	sub	DIFF,S2,DIFF

	b	L_DBYTECP
	deccc	N

!
! Copy bytes from src to dst until we've populated 
! the first two dst double words. This is to avoid UMRs
! caused by reading the unitialized dst. (bug #4013042)
! (lperry)
!
L_PRELOOP:
        mov     SAVEDS1, DST
        mov     S2, SRC
        add     23, DST, DSTEND   ! make sure we get 2 dwords plus any partials
        and     DSTEND,0xfffffff8, DSTEND

L_TEST_DST:
        ldub    [SRC], TMP8
        stb     TMP8, [DST]
        inc     DST
        cmp     DST, DSTEND
        bne     L_TEST_DST
        inc     SRC
!
! Since we've populated at least two dst dwords, its
! safe to advance the src pointer by 8 and drop the byte count by 8
!
        inc     8, S2
        b       L_MAIN
        dec     8, N

! while ((int)s2 & 7) {
!	s2[diff] = s2[0];
!       s2++;
!       n--;
! }
L_INT1:
	dec	N
	stb	%o1,[S2+DIFF]
	inc	S2
L_MAIN:
	andcc   S2,7,%g0
	bne,a	L_INT1
	ldsb	[S2],%o1

! misalignment = diff & 0x7;
! switch (misalignment)
	andcc	DIFF,7,MISALIGN
	bz      L_cpy_8_aligned
	cmp	MISALIGN,4
	be	L_cpy_4_aligned
	nop
	bl	L_cpy_under4_aligned
	nop
	b       L_cpy_over4_aligned	
	nop
L_ENDSWITCH:
	deccc	N
! while (--n >= 0) {
!	s2[diff] = s2[0];
!	s2++;
! }
L_DBYTECP:
	bneg	L_DONE
	nop
L_BWHILE:
	ldsb	[S2],%o0
	deccc	N
	stb	%o0,[S2+DIFF]
	bpos	L_BWHILE
	inc	S2
L_DONE:
	ret
	restore %g0,SAVEDS1,%o0

! each of the L_cpy_x_aligned follows the same algorithm:
! if (n < 64) {
!     INITIALIZE offsets and residuals
!     setup differences;
!     while (n-64 > 0) {
!         UNROLLED_LOOP
!	  n-=64
!	  s2+=64;
!     }
! }
! while (n-8 > 0) {
!     SINGLE_LOOP
!     n-=8;
!     s2+=8;
! }
! reset s2,n,and diff for dbytecp

#define SETUP_DIFFS \
	add     DIFF,8,  DIFF1; \
	add	DIFF,16, DIFF2; \
	add     DIFF,24, DIFF3; \
	add     DIFF,32, DIFF4; \
	add	DIFF,40, DIFF5; \
	add	DIFF,48, DIFF6; \
	add	DIFF,56, DIFF7; 

!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!1
! code for L_cpy_8_aligned
#define TMP   %o0  

#define ITERATION(offset,reg) \
	ldd	[S2+offset],TMP; \
	std     TMP,[S2+reg];

L_cpy_8_aligned:
! if (n < 64) {
	cmp	N,64
	bl,a	LESSTHAN64_8
	cmp	N,8

! setup differences
	dec     64, N	
	SETUP_DIFFS
! while (n-64 > 0) {
LWHILE8:
	ITERATION(0, DIFF)
	ITERATION(8, DIFF1)
	ITERATION(16,DIFF2)
	ITERATION(24,DIFF3)
	ITERATION(32,DIFF4)
	ITERATION(40,DIFF5)
	ITERATION(48,DIFF6)
	ITERATION(56,DIFF7)
! n -= 64;
	deccc	64,N
	bpos	LWHILE8
	inc	64,S2
! } end while

	inc     64,N
	cmp	N,8
! } end if
LESSTHAN64_8:
	bl    	L_ENDSWITCH
	nop
	dec     8,N
! while (n-8 > 0) {
L_SWHILE8:
	ITERATION(0, DIFF)
	deccc	8,N
	bpos	L_SWHILE8
	inc	8,S2
! }
	
	inc     8,N
L_DONE8:
        b	L_ENDSWITCH
        nop

#undef TMP
#undef ITERATION

!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
! L_cpy_4_aligned:
!

#define ITERATION0(offset,reg) \
	ldd	[S2+offset],%o0; \
	mov     %o3, %o2; \
	mov     %o0, %o3; \
	std     %o2,[S2+reg]; 

#define ITERATION1(offset,reg) \
	ldd	[S2+offset],%o2; \
	mov     %o1, %o0; \
	mov     %o2, %o1; \
	std     %o0,[S2+reg]; 

L_cpy_4_aligned:
	dec	4, DIFF;
	ld      [S2+DIFF],%o3
! if (n < 64) {
	cmp	N,64
	bl,a	LESSTHAN64_4
	cmp	N,8

! setup differences
	dec     64, N	
	SETUP_DIFFS
! while (n-64 > 0) {
LWHILE4:
	ITERATION0(0, DIFF)
	ITERATION1(8, DIFF1)
	ITERATION0(16,DIFF2)
	ITERATION1(24,DIFF3)
	ITERATION0(32,DIFF4)
	ITERATION1(40,DIFF5)
	ITERATION0(48,DIFF6)
	ITERATION1(56,DIFF7)
! n -= 64;
	deccc	64,N
	bpos	LWHILE4
	inc	64,S2
! } end while

	inc     64,N
	cmp	N,8
! } end if
LESSTHAN64_4:
	bl    	L_DONE4
	nop
	dec     8,N
! while (n-8 > 0) {
L_SWHILE4:
	ITERATION0(0, DIFF)
	mov	%o1,%o3
	deccc	8,N
	bpos	L_SWHILE4
	inc	8,S2
! }
	
	inc     8,N
L_DONE4:
	inc     4, N
	dec     4, S2
	inc	4, DIFF
        b	L_ENDSWITCH
        nop

#undef ITERATION

!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
! L_cpy_over4_aligned:

#define DTMP   %o0
#define DTMP0  %o0
#define DTMP1  %o1
#define RESIDUAL  %o2
#define RESIDUAL0 %o2
#define RESIDUAL1 %o3
#define STMP      %o4
#define MASK      %i3
#define RIGHT     %i4
#define LEFT      %i5

#define ITERATION(offset,reg) \
 	ldd	[S2+offset], DTMP;    		\
	srl	DTMP0, RIGHT, STMP;		\
	or	RESIDUAL1, STMP, RESIDUAL1;	\
	std	RESIDUAL, [S2+reg];		\
	sll	DTMP0, LEFT, RESIDUAL0;		\
	srl	DTMP1, RIGHT, STMP;		\
	or	RESIDUAL0, STMP, RESIDUAL0;	\
	sll	DTMP1, LEFT, RESIDUAL1;


L_cpy_over4_aligned:
! right = 8 * (misalignment - 4);
! left = 32 - right
! mask = 0xffffffff << left
	sub     MISALIGN,4,RIGHT
	sll     RIGHT,3,RIGHT
	sub     %g0,RIGHT,LEFT
	mov     -1,MASK
	sll	MASK,LEFT,MASK
	inc     32,LEFT
	sub     DIFF,MISALIGN,DIFF
	ldd     [S2+DIFF],RESIDUAL
	and     RESIDUAL1, MASK, RESIDUAL1
! if (n < 64) {
	cmp	N,64
	bl,a	LESSTHAN64_O
	cmp	N,8

! setup differences
	dec     64, N	
	SETUP_DIFFS
! while (n-64 > 0) {
LWHILEO:
	ITERATION(0, DIFF)
	ITERATION(8, DIFF1)
	ITERATION(16,DIFF2)
	ITERATION(24,DIFF3)
	ITERATION(32,DIFF4)
	ITERATION(40,DIFF5)
	ITERATION(48,DIFF6)
	ITERATION(56,DIFF7)
! n -= 64;
	deccc	64,N
	bpos	LWHILEO
	inc	64,S2
! } end while

	inc     64,N
	cmp	N,8
! } end if
LESSTHAN64_O:
	bl    	L_DONEO
	nop
	dec     8,N
! while (n-8 > 0) {
L_SWHILEO:
	ITERATION(0, DIFF)
	deccc	8,N
	bpos	L_SWHILEO
	inc	8,S2
! }
	
	inc     8,N
L_DONEO:
	add	 N,MISALIGN,N
	sub      S2,MISALIGN,S2
	add	 DIFF,MISALIGN,DIFF
        b	L_ENDSWITCH
        nop

#undef DTMP
#undef DTMP0
#undef DTMP1
#undef RESIDUAL
#undef RESIDUAL0
#undef RESIDUAL1
#undef STMP
#undef MASK
#undef RIGHT
#undef LEFT
#undef ITERATION

!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
! L_cpy_under4_aligned:

#define DTMP   %o0
#define DTMP0  %o0
#define DTMP1  %o1
#define S1TMP  %o2
#define S1TMP0 %o2
#define S1TMP1 %o3
#define RESIDUAL  %o4
#define MASK      %i3
#define RIGHT     %i4
#define LEFT      %i5

#define ITERATION(offset,reg) \
 	ldd	[S2+offset], DTMP;    		\
	srl	DTMP0, RIGHT, S1TMP0;		\
	or	S1TMP0, RESIDUAL, S1TMP0;	\
	sll     DTMP0, LEFT, RESIDUAL;		\
	srl	DTMP1, RIGHT, S1TMP1;		\
	or      S1TMP1, RESIDUAL, S1TMP1;	\
	std     S1TMP, [S2+reg];		\
	sll	DTMP1, LEFT, RESIDUAL;



L_cpy_under4_aligned:
! right = mislignment * 8;
! left = 32 - right
! mask = 0xffffffff << left
	sll     MISALIGN,3,RIGHT
	sub     %g0,RIGHT,LEFT
	mov     -1,MASK
	sll	MASK,LEFT,MASK
	inc     32,LEFT
	sub     DIFF,MISALIGN,DIFF
	ld      [S2+DIFF],RESIDUAL
	and     RESIDUAL, MASK, RESIDUAL
! if (n < 64) {
	cmp	N,64
	bl,a	LESSTHAN64_U
	cmp	N,8

! setup differences
	dec     64, N	
	SETUP_DIFFS
! while (n-64 > 0) {
LWHILEU:
	ITERATION(0, DIFF)
	ITERATION(8, DIFF1)
	ITERATION(16,DIFF2)
	ITERATION(24,DIFF3)
	ITERATION(32,DIFF4)
	ITERATION(40,DIFF5)
	ITERATION(48,DIFF6)
	ITERATION(56,DIFF7)
! n -= 64;
	deccc	64,N
	bpos	LWHILEU
	inc	64,S2
! } end while

	inc     64,N
	cmp	N,8
! } end if
LESSTHAN64_U:
	bl    	L_DONEU
	nop
	dec     8,N
! while (n-8 > 0) {
L_SWHILEU:
	ITERATION(0, DIFF)
	deccc	8,N
	bpos	L_SWHILEU
	inc	8,S2
! }
	
	inc     8,N
L_DONEU:
	add	N,MISALIGN,N
	sub     S2,MISALIGN,S2
	add	DIFF,MISALIGN,DIFF
        b	L_ENDSWITCH
        nop














