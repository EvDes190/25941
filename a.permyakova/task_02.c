#include <stdio.h>
#include <time.h>
#include <stdlib.h>

int main(void) {
    time_t t = time(NULL) - 8 * 3600;
    struct tm *lt = gmtime(&t);

    char buf[64];
    strftime(buf, sizeof buf, "%a %b %d %H:%M:%S %Y PST", lt);
    printf("%s\n", buf);

    return 0;
}