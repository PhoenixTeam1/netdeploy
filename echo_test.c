#include <unistd.h>
#include <stdio.h>
#include <string.h>

int main() {
	char input[500];
	int i;
	i = 5;
	while(i > 0 && fgets(input, 500, stdin)){  //read from STDIN (aka command-line)
		printf("You sent:%s", input);  //print out what user typed in
		printf("I will echo %d more times\n",--i);
		memset(input, 0, strlen(input));  //reset string to all 0's
	}
	return 0;
}

