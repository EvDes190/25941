#include <sys/types.h>
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
extern char *tzname[];

int main() {
	time_t now;
	struct tm *sp;

	(void) time( &now );
	now -= 15 * 60 * 60;
	printf("%s", ctime( &now ) );

	sp = localtime(&now);
	printf("%d/%d/%02d %d:%02d %s\n",
        sp->tm_mon + 1, sp->tm_mday,
        sp->tm_year, sp->tm_hour,
        sp->tm_min, "-7");
	exit(0);
}
