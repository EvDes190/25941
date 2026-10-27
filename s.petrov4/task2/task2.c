#include <sys/types.h>
#include <stdio.h>
#include <time.h>
#include <stdlib.h>

extern char *tzname[];

int main()
{
    time_t now;
    struct tm *sp;

    // Устанавливаем временную зону Калифорнии
    putenv("TZ=PST8PDT");
    tzset();

    // Получаем текущее время
    (void) time(&now);

    // Печатаем время строкой
    printf("%s", ctime(&now));

    // Получаем локальное время с учётом TZ
    sp = localtime(&now);

    // Печатаем дату, время и название временной зоны
    printf("%d/%d/%02d %d:%02d %s\n",
        sp->tm_mon + 1,
        sp->tm_mday,
        sp->tm_year+1900,
        sp->tm_hour,
        sp->tm_min,
        tzname[sp->tm_isdst]);

    exit(0);
}
