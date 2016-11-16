#include <stdlib.h>
#include <sys/stat.h>

/*
 * cp oldfile newfile
 * newfile is dir or just file name
 * cp aa/bb cc 		# cc/bb if cc is dir
 * same mode
 */

main(argc,argv)
char **argv;
{
	static int buf[256];
	int fold, fnew, n;
	register char *p1, *p2, *bp;
	int mode;
	struct stat s1;
	struct stat s2;

	if(argc != 3) {
		write(1, "Usage: cp oldfile newfile\n", 26);
		exit(1);
	}
	if((fold = open(argv[1], 0)) < 0) {
		write(1, "Cannot open old file.\n", 22);
		exit(1);
	}
	fstat(fold, &s1);
	mode = s1.st_mode;
	/* is target a directory? */
	if (stat(argv[2], &s2)>=0 && S_ISDIR(s2.st_mode)) {
		p1 = argv[1];
		p2 = argv[2];
		bp = (char *)(buf+100);
		while(*bp++ = *p2++);
		bp[-1] = '/';
		p2 = bp;
		while(*bp = *p1++)
			if(*bp++ == '/')
				bp = p2;
		argv[2] = (char *)(buf+100);
	}
	//printf(">> %s\n", argv[2]);

	if (stat(argv[2], &s2) >= 0) {
		if (s1.st_ino == s2.st_ino) {
			write(1, "Copying file to itself.\n", 24);
			exit(1);
		}
	}
	if ((fnew = creat(argv[2], mode)) < 0) {
		write(1, "Can't create new file.\n", 23);
		exit(1);
	}
	while(n = read(fold,  buf,  512)) {
	if(n < 0) {
		write(1, "Read error\n", 11);
		exit(1);
	} else
		if(write(fnew, buf, n) != n){
			write(1, "Write error.\n", 13);
			exit(1);
		}
	}
	exit(0);
}
