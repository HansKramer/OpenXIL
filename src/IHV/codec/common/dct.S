! @(#)dct.S	1.4 95/03/30 Sun Microsystems North Carolina Development Center
! 

#define X	%i0
#define DX	%i1
#define XPTR	%i2
#define HALF	%i3

#define A0	%i4
#define A1	%i5
#define A2	%l0
#define A3	%l1
#define A4	%l2
#define A5	%l3
#define A6	%l4
#define A7	%l5

#define B0	%l6
#define B1	%l7
#define B2	%o0
#define B3	%o1
#define B4	%o2
#define B5	%o3
#define B6	%o4
#define B7	%o5

#define TEMP0	%i2
#define TEMP1	%i4

#define DCT8    dct8

! Multiplies are calculated as follows:
!
! COS_1 = (x << 16) - (x5 << 8) + (x5 << 2) + x
! COS_2 = (x << 16) - (x << 12) - (x << 10) + (x << 7) + x3
! COS_3 = (x << 16) - (x3 << 12) + (x << 10) + (x3 << 6) + (x3 << 3) + x3
! COS_4 = (x5 << 13) + (x << 12) + (x5 << 8) + x5
! COS_5 = (x << 15) + (x7 << 9) + (x7 << 3) + (x << 1)
! COS_6 = (x << 15) - (x << 13) + (x << 9) - (x << 3)
! COS_7 = (x3 << 12) + (x << 9) - (x << 4) + x
!

	.section ".text"
	.global DCT8
DCT8:
	save	%sp,-120,%sp
	sll	DX,2,DX

!   half = 32768;
	mov	1,HALF
	sll	HALF,15,HALF

!   b0 = *xptr;	xptr += dx;
!   b1 = *xptr;	xptr += dx;
!   b2 = *xptr;	xptr += dx;
!   b3 = *xptr;	xptr += dx;
!   b4 = *xptr;	xptr += dx;
!   b5 = *xptr;	xptr += dx;
!   b6 = *xptr;	xptr += dx;
!   b7 = *xptr;
	ld	[X],B0
	add	X,DX,XPTR
	ld	[XPTR],B1
	add	XPTR,DX,XPTR
	ld	[XPTR],B2
	add	XPTR,DX,XPTR
	ld	[XPTR],B3
	add	XPTR,DX,XPTR
	ld	[XPTR],B4
	add	XPTR,DX,XPTR
	ld	[XPTR],B5
	add	XPTR,DX,XPTR
	ld	[XPTR],B6
	add	XPTR,DX,XPTR
	ld	[XPTR],B7

!   a0 =  b0 + b7;
!   a1 =  b1 + b6;
!   a2 =  b2 + b5;
!   a3 =  b3 + b4;
!   a4 = -b4 + b3;
!   a5 = -b5 + b2;
!   a6 = -b6 + b1;
!   a7 = -b7 + b0;
	add	B3,B4,A3
	add	B2,B5,A2
	add	B1,B6,A1
	add	B0,B7,A0
	sub	B3,B4,A4
	sub	B2,B5,A5
	sub	B1,B6,A6
	sub	B0,B7,A7

!   b0 =  a0 + a3;
!   b1 =  a1 + a2;
!   b2 = -a2 + a1;
!   b3 = -a3 + a0;
	add	A0,A3,B0
	add	A1,A2,B1
	sub	A1,A2,B2
	sub	A0,A3,B3

!   b5 = ((-a5 + a6) * COS_4 + half) >> 16;
	sub	A6,A5,B5
	sll	B5,2,TEMP0
	add	B5,TEMP0,TEMP0
	sll	B5,12,B5
	add	TEMP0,B5,B5
	sll	TEMP0,8,TEMP0
	add	TEMP0,B5,B5
	sll	TEMP0,5,TEMP0
	add	TEMP0,B5,B5
	add	HALF,B5,B5
	sra	B5,16,B5

!   b7 = (( a5 + a6) * COS_4 + half) >> 16;
	add	A6,A5,B7
	sll	B7,2,TEMP0
	add	B7,TEMP0,TEMP0
	sll	B7,12,B7
	add	TEMP0,B7,B7
	sll	TEMP0,8,TEMP0
	add	TEMP0,B7,B7
	sll	TEMP0,5,TEMP0
	add	TEMP0,B7,B7
	add	HALF,B7,B7
	sra	B7,16,B7

!   b4 =  a4 + b5;
!   b5 =  a4 - b5;
!   b6 = -b7 + a7;
!   b7 =  b7 + a7;
	add	A4,B5,B4
	sub	A4,B5,B5
	sub	A7,B7,B6
	add	A7,B7,B7

!   a0 = (b0 + b1) * COS_4;
	add	B0,B1,A0
	sll	A0,2,TEMP0
	add	A0,TEMP0,TEMP0
	sll	A0,12,A0
	add	TEMP0,A0,A0
	sll	TEMP0,8,TEMP0
	add	TEMP0,A0,A0
	sll	TEMP0,5,TEMP0
	add	TEMP0,A0,A0

!   *x = (a0 + half) >> 16; x += dx;
	add	A0,HALF,A0
	sra	A0,16,A0
	st	A0,[X]
	add	X,DX,X

!   a1 = b4 * COS_7 + b7 * COS_1;
	sll	B4,12,A1
	sll	B4,13,TEMP0
	add	A1,TEMP0,A1
	sll	B4,9,TEMP0
	add	A1,TEMP0,A1
	sll	B4,4,TEMP0
	sub	A1,TEMP0,A1
	add	B4,A1,A1
	sll	B7,2,TEMP0
	add	B7,TEMP0,TEMP0
	add	A1,B7,A1
	sll	TEMP0,2,TEMP1
	add	A1,TEMP1,A1
	sll	TEMP0,8,TEMP1
	sub	A1,TEMP1,A1
	sll	B7,16,TEMP1
	add	A1,TEMP1,A1

!   *x = (a1 + half) >> 16; x += dx;
	add	A1,HALF,A1
	sra	A1,16,A1
	st	A1,[X]
	add	X,DX,X

!   a2 = b2 * COS_6 + b3 * COS_2;
	sll	B2,3,A2
	sll	B2,9,TEMP0
	sub	TEMP0,A2,A2
	sll	B2,13,TEMP0
	sub	A2,TEMP0,A2
	sll	B2,15,TEMP0
	add	A2,TEMP0,A2
	add	A2,B3,A2
	sll	B3,1,TEMP0
	add	A2,TEMP0,A2
	sll	B3,7,TEMP0
	add	A2,TEMP0,A2
	sll	B3,10,TEMP0
	sub	A2,TEMP0,A2
	sll	B3,12,TEMP0
	sub	A2,TEMP0,A2
	sll	B3,16,TEMP0
	add	A2,TEMP0,A2

!   *x = (a2 + half) >> 16; x += dx;
	add	A2,HALF,A2
	sra	A2,16,A2
	st	A2,[X]
	add	X,DX,X

!   a3 = b6 * COS_3 - b5 * COS_5;
	sll	B6,1,TEMP0
	add	B6,TEMP0,TEMP0
	sll	TEMP0,3,TEMP1
	add	TEMP0,TEMP1,A3
	sll	TEMP0,6,TEMP1
	add	A3,TEMP1,A3
	sll	TEMP0,12,TEMP1
	sub	A3,TEMP1,A3
	sll	B6,10,TEMP1
	add	A3,TEMP1,A3
	sll	B6,16,TEMP1
	add	A3,TEMP1,A3
	sll	B5,3,TEMP0
	sub	TEMP0,B5,TEMP0
	sll	B5,15,TEMP1
	sub	A3,TEMP1,A3
	sll	TEMP0,9,TEMP1
	sub	A3,TEMP1,A3
	sll	TEMP0,3,TEMP1
	sub	A3,TEMP1,A3
	sll	B5,1,TEMP1
	sub	A3,TEMP1,A3

!   *x = (a3 + half) >> 16; x += dx;
	add	A3,HALF,A3
	sra	A3,16,A3
	st	A3,[X]
	add	X,DX,X

!   a4 = (b0 - b1) * COS_4;
	sub	B0,B1,A4
	sll	A4,2,TEMP0
	add	A4,TEMP0,TEMP0
	sll	A4,12,A4
	add	TEMP0,A4,A4
	sll	TEMP0,8,TEMP0
	add	TEMP0,A4,A4
	sll	TEMP0,5,TEMP0
	add	TEMP0,A4,A4

!   *x = (a4 + half) >> 16; x += dx;
	add	A4,HALF,A4
	sra	A4,16,A4
	st	A4,[X]
	add	X,DX,X

!    a5 = b5 * COS_3 + b6 * COS_5;
	sll	B5,1,TEMP0
	add	B5,TEMP0,TEMP0
	sll	TEMP0,3,TEMP1
	add	TEMP0,TEMP1,A5
	sll	TEMP0,6,TEMP1
	add	A5,TEMP1,A5
	sll	TEMP0,12,TEMP1
	sub	A5,TEMP1,A5
	sll	B5,10,TEMP1
	add	A5,TEMP1,A5
	sll	B5,16,TEMP1
	add	A5,TEMP1,A5
	sll	B6,3,TEMP0
	sub	TEMP0,B6,TEMP0
	sll	B6,15,TEMP1
	add	A5,TEMP1,A5
	sll	TEMP0,9,TEMP1
	add	A5,TEMP1,A5
	sll	TEMP0,3,TEMP1
	add	A5,TEMP1,A5
	sll	B6,1,TEMP1
	add	A5,TEMP1,A5

!   *x = (a5 + half) >> 16; x += dx;
	add	A5,HALF,A5
	sra	A5,16,A5
	st	A5,[X]
	add	X,DX,X

!   a6 = b3 * COS_6 - b2 * COS_2;
	sll	B3,3,A6
	sll	B3,9,TEMP0
	sub	TEMP0,A6,A6
	sll	B3,13,TEMP0
	sub	A6,TEMP0,A6
	sll	B3,15,TEMP0
	add	A6,TEMP0,A6
	sub	A6,B2,A6
	sll	B2,1,TEMP0
	sub	A6,TEMP0,A6
	sll	B2,7,TEMP0
	sub	A6,TEMP0,A6
	sll	B2,10,TEMP0
	add	A6,TEMP0,A6
	sll	B2,12,TEMP0
	add	A6,TEMP0,A6
	sll	B2,16,TEMP0
	sub	A6,TEMP0,A6

!   *x = (a6 + half) >> 16; x += dx;
	add	A6,HALF,A6
	sra	A6,16,A6
	st	A6,[X]
	add	X,DX,X

!   a7 = b7 * COS_7 - b4 * COS_1;
	sll	B7,12,A7
	sll	B7,13,TEMP0
	add	A7,TEMP0,A7
	sll	B7,9,TEMP0
	add	A7,TEMP0,A7
	sll	B7,4,TEMP0
	sub	A7,TEMP0,A7
	add	B7,A7,A7
	sll	B4,2,TEMP0
	add	B4,TEMP0,TEMP0
	sub	A7,B4,A7
	sll	TEMP0,2,TEMP1
	sub	A7,TEMP1,A7
	sll	TEMP0,8,TEMP1
	add	A7,TEMP1,A7
	sll	B4,16,TEMP1
	sub	A7,TEMP1,A7

!   *x = (a7 + half) >> 16;
	add	A7,HALF,A7
	sra	A7,16,A7
	st	A7,[X]

	ret
	restore

	
	

	
	
	
