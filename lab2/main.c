#include <sys/types.h>
#include <stdio.h>
#include <time.h>
#include <stdlib.h>

extern char *tzname[];

void print_time(const struct tm *sp) {
    printf("%02d/%02d/%4d %02d:%02d %s\n",
        sp->tm_mon + 1,          // Месяц
        sp->tm_mday,             // День
        sp->tm_year + 1900,      // Год
        sp->tm_hour,             // Часы
        sp->tm_min,              // Минуты
        tzname[sp->tm_isdst > 0 ? 1 : 0]);  // PST или PDT
}

int main()
{
    time_t now;

    setenv("TZ", "America/Los_Angeles", 1);
    
    tzset();

    (void) time(&now);

    putenv("TZ=PDT7");
    printf("%s", ctime(&now));
    print_time(localtime(&now));

    putenv("TZ=PST8");
    printf("%s", ctime(&now));
    print_time(localtime(&now));
        
    return 0;
}