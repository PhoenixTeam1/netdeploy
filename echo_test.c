#include <unistd.h>
#include <stdio.h>
#include <string.h>

int main() {
	/*int c;
	c = 0;
	while (c != EOF) {
		printf("Say something to me\n");
		c = getchar();
		getchar(); // dismiss carraige return
		putchar(c);
	}*/

	char input[500];
	while(fgets(input, 500, stdin)){  //read from STDIN (aka command-line)
		printf("%s", input);  //print out what user typed in
		memset(input, 0, strlen(input));  //reset string to all 0's
	}
	return 0;
}

