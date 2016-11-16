#include <stdio.h>


/* 
 last arg append '\n', other append ' '
 */
int main(int argc, char *argv[])
{
	int i;

	argc--;
	for(i=1; i<=argc; i++)
		printf("%s%c", argv[i], i==argc? '\n': ' ');

	return 0;
}
