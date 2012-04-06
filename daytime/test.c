#include<stdio.h>
#include<time.h>
#include<string.h>

int main(int argc, char *argv[]){
	time_t seconds;
	char *date;
	seconds = time(NULL);
	date = ctime(&seconds);
	printf("%s %d\n", date, (int)strlen(date));
	return 0;
}
