
chapter1
========
* DR/SR 177570; 
* e -v virtual address; d -a -c -m -o -d -h
* tmrk copies blocks from tape to disk; ltap primary boot program;
*  

Setting up UNIX
===============
4000 * 512-byte block, three fairly full RK packs:
* binary version of all programs, source for os itself.
* all remaining source
* manuals

## magtap to binary RK
Load first block +24k(? src/mdec/mboot.s) to address 0

	set cpu 11/40
	set tm0 locked
	attach tm0 dist.tap
	attach rk0 rk0
	attach rk1 rk1
	attach rk2 rk2
	d cpu 100000 012700        ; mov #172526,R0
	d cpu 100002 172526
	d cpu 100004 010040        ; mov R0,-(R0)
	d cpu 100006 012740        ; mov #60003,-(R0) ; 060003 = 24579
	d cpu 100010 060003
	d cpu 100012 000777        ; br 100012
	g 100000

mboot source code:

	as tpboot.s tty.s tm.s
	strip a.out
	ls -l a.out
	cp a.out /usr/mdec/mboot

	sim> e -m 0-770
	0:	BR 20
	2:	BR 177534
	4:	HALT
	6:	HALT
	10:	HALT
	12:	HALT
	14:	HALT
	16:	WAIT
	20:	MOV #137000,SP
	24:	MOV SP,R1
	26:	CMP PC,R1				; 2. when run @137000, they are equal to 137000 !!!
	30:	BCC 60					; 2. jump to 60 @137000
	32:	CLR R0
	34:	CMP (R0),#407
	40:	BNE 46
	42:	MOV #20,R0
	46:	MOV (R0)+,(R1)+			; 1. copy 020 +512 -> 137000
	50:	CMP R1,#140000
	54:	BCS 46
	56:	JMP (SP)				; 1. jump to 137000
	60:	MOV #137454,R5
	64:	MOV #136740,R4
	70:	JSR PC,654
	74:	MOV #75,R0
	100:	JSR PC,(R5)
	102:	MOV R4,R1
	104:	JSR PC,314
	......
	
## run program @0
run loader; loader copy itself to 137000; run from 137000. 
select/run program tmrk. copies tape to disk with the given offsets and counts.
tmrk moving itself from \[0, 512) to  \[48k-512, 48k); 137000 - 140000. 
Tap [0-100] for ?


	sim> go 0
	=tmrk
	disk offset
	0
	tape offset				<- 100 * 512 = 0xc800
	100
	count
	1
	=tmrk
	disk offset
	1
	tape offset
	101
	count
	3999
	=
	Simulation stopped, PC: 137300 (BGE 137274) ; 0137274 = 48828 = 48k - 512
	sim> 


tmrk source:

	as mcopy.s tm.s wrk.s rk.s
	strip a.out
	cp a.out /usr/mdec/tmrk


Either mboot or tmrk first load to @0, then move itself to @137000 and run from
137000, interesting.





