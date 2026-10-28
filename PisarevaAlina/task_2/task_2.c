#include <stdio.h>
#include <time.h>
#include <stdlib.h>

int main()
{
    time_t now;
    char buf[64];

    putenv("TZ=PST8");
    tzset();
    time(&now);

    printf("%s", ctime(&now));
    strftime(buf, sizeof buf, "%m/%d/%Y %H:%M %Z\n", localtime(&now));
    printf("%s", buf);

    return 0;
}
