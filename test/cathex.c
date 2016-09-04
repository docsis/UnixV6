#include <stdio.h>


/* 
control+v control+c 03, kernel translate keypad code to 
special meaning, tty translate to signal? 
https://en.wikipedia.org/wiki/C0_and_C1_control_codes 
showkey -a
http://www.linuxjournal.com/article/1080?page=0,1
*/
int main(int argc, char *argv[])
{
	int c;

	while ((c=getchar()) != EOF) {
		printf("%02x ", c);
	}
	return 0;
}
