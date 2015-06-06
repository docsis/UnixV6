
Generate doc

	nroff -ms /usr/doc/start/start > /dev/lp0


# 
[Installing Unix v6 (PDP-11) on SIMH]()

	# STTY -LCASE

	http://gunkies.org/wiki/Installing_Unix_v6_(PDP-11)_on_SIMH#Booting_up_to_single_user_mode)

	sim>do s1.txt 

	set cpu 11/40
	set tm0 locked
	attach tm0 dist.tap
	attach rk0 rk0
	attach rk1 rk1
	attach rk2 rk2
	d cpu 100000 012700        ; mov #172526,R0
	d cpu 100002 172526
	d cpu 100004 010040        ; mov R0,-(R0)
	d cpu 100006 012740        ; mov #60003,-(R0)
	d cpu 100010 060003
	d cpu 100012 000777        ; br 100012
	g 100000

	ctrl+e
	g 0

	set cpu 11/40
	set tto 7b
	set tm0 locked
	attach tm0 dist.tap
	attach rk0 rk0
	attach rk1 rk1
	attach rk2 rk2
	dep system sr 173030
	boot rk0

	do s2.txt

	as m40.s
	mv a.out m40.o
	cc -c c.c
	as l.s
	ld -x a.out m40.o c.o ../lib1 ../lib2
	mv a.out /unix

	/etc/mknod /dev/rk0 b 0 0
	/etc/mknod /dev/rk1 b 0 1
	/etc/mknod /dev/rk2 b 0 2
	/etc/mknod /dev/mt0 b 3 0
	/etc/mknod /dev/tap0 b 4 0
	/etc/mknod /dev/rrk0 c 9 0
	/etc/mknod /dev/rrk1 c 9 1
	/etc/mknod /dev/rrk2 c 9 2
	/etc/mknod /dev/rmt0 c 12 0
	/etc/mknod /dev/lp0 c 2 0
	/etc/mknod /dev/tty0 c 3 0
	/etc/mknod /dev/tty1 c 3 1
	/etc/mknod /dev/tty2 c 3 2
	/etc/mknod /dev/tty3 c 3 3
	/etc/mknod /dev/tty4 c 3 4
	/etc/mknod /dev/tty5 c 3 5
	/etc/mknod /dev/tty6 c 3 6
	/etc/mknod /dev/tty7 c 3 7
	chmod 640 /dev/*rk*
	chmod 640 /dev/*mt*
	chmod 640 /dev/*tap*
