!
!   @(#)xili_memset.S	1.4\t97/04/30  
!
!   xil version of memset (does double loads and stores and unrolls loops)
!
!       Project: XIL

! variables used throughout
#define DST      %i0
#define VALUE    %i1
#define N        %i2
#define SAVEDS1  %l0
#define VALUE64  %l4

#include <sys/asm_linkage.h>
	.section ".text"

	ENTRY(xili_memset)
	save	%sp,-SA(MINFRAME),%sp
        mov	DST,SAVEDS1

! get VALUE as a byte-replicated full 32-bit value 
        and	VALUE,0x000000ff,%l4
        sll	%l4,24,%l5
        sll	%l4,16,%l6
        or	%l5,%l6,%l5
        sll	%l4,8,%l6
        or	%l5,%l6,%l5
        or	%l4,%l5,%l5
! make it a 64-bit value
	mov	%l5,VALUE64

! if N < minfast (15) then just move bytes
! there is code that depends on the value 15, so don't make it lower.
        cmp	N,16
	bge	L_MAIN
        nop

        b	L_DBYTECP
	deccc	N
! while((int)dst & 7) {
!	*dst = value;
!       dst++;
!       n--;
! }
L_INT1:
	dec	N
	stb	VALUE,[DST]
	inc	DST
L_MAIN:
	andcc   DST,7,%g0
	bne,a	L_INT1
        nop

! do 8-bytes at a time
	b       L_set_8_aligned
	nop
        
L_ENDALIGNED:
	deccc	N
! while (--n >= 0) {
!	*dst = VALUE;
!	dst++;
! }
L_DBYTECP:
	bneg	L_DONE
	nop
L_BWHILE:
	stb	VALUE,[DST]
	deccc	N
	bpos	L_BWHILE
	inc	DST
L_DONE:
	ret
	restore %g0,SAVEDS1,%o0

! The L_set_8_aligned algorithm:
! if(n < 64) {
!     while(n-64 > 0) {
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
! reset s2,n,and dst for dbytecp

!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!1
! code for L_set_8_aligned
#define ITERATION(offset) \
	std     VALUE64,[DST+offset];

L_set_8_aligned:
! if (n < 64) {
	cmp	N,64
	bl,a	LESSTHAN64_8
	cmp	N,8

! setup destination addrs
	dec     64, N
! while (n-64 > 0) {
LWHILE8:
	ITERATION(0)
	ITERATION(8)
	ITERATION(16)
	ITERATION(24)
	ITERATION(32)
	ITERATION(40)
	ITERATION(48)
	ITERATION(56)
! n -= 64;
	deccc	64,N
	bpos	LWHILE8
	inc	64,DST
! } end while

	inc     64,N
	cmp	N,8
! } end if
LESSTHAN64_8:
	bl    	L_ENDALIGNED
	nop
	dec     8,N
! while (n-8 > 0) {
L_SWHILE8:
	ITERATION(0)
	deccc	8,N
	bpos	L_SWHILE8
	inc	8,DST
! }
	
	inc     8,N
L_DONE8:
        b	L_ENDALIGNED
        nop

