#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

static volatile sig_atomic_t time_skip = 0;

void on_alarm(int sig) {
  (void)sig;
  time_skip = 1;
}

typedef struct {
  off_t offset;
  size_t length;
} LineInfo;

static void build_table(int fd, LineInfo **table, size_t *count) {
  size_t cap = 0;
  *count = 0;
  *table = NULL;
  off_t pos = 0, start = 0;
  char ch;

  lseek(fd, 0, SEEK_SET);
  while (read(fd, &ch, 1) == 1) {
    pos++;
    if (ch == '\n') {
      size_t len = (size_t)(pos - start - 1);
      if (*count == cap) {
        cap = cap ? cap * 2 : 16;
        *table = realloc(*table, cap * sizeof(LineInfo));
      }
      (*table)[*count].offset = start;
      (*table)[*count].length = len;
      (*count)++;
      start = pos;
    }
  }
  if (pos > start) {
    size_t len = (size_t)(pos - start);
    if (*count == cap) {
      cap = cap ? cap * 2 : 16;
      *table = realloc(*table, cap * sizeof(LineInfo));
    }
    (*table)[*count].offset = start;
    (*table)[*count].length = len;
    (*count)++;
  }
  lseek(fd, 0, SEEK_SET);
}

static void print_table(const LineInfo *table, size_t count) {
  printf("+-------+--------+--------+\n");
  printf("| line  | offset | length |\n");
  printf("+-------+--------+--------+\n");
  for (size_t i = 0; i < count; i++) {
    printf("| %5zu | %6lld | %6zu |\n", i + 1, (long long)table[i].offset,
           table[i].length);
  }
  printf("+-------+--------+--------+\n");
}

static void print_line(int fd, const LineInfo *table, size_t count, int n) {
  if (n < 1 || (size_t)n > count) {
    puts("Not this line");
    return;
  }
  LineInfo li = table[n - 1];
  lseek(fd, li.offset, SEEK_SET);
  char buf[4096];
  size_t left = li.length;
  while (left > 0) {
    size_t toread = left < sizeof(buf) ? left : sizeof(buf);
    ssize_t r = read(fd, buf, toread);
    if (r <= 0)
      break;
    write(STDOUT_FILENO, buf, (size_t)r);
    left -= (size_t)r;
  }
  putchar('\n');
}

int main(void) {
  int fd = open("file.txt", O_RDONLY);
  if (fd < 0) {
    perror("open");
    return 1;
  }

  LineInfo *table = NULL;
  size_t count = 0;
  build_table(fd, &table, &count);

  print_table(table, count);

  int n;
  signal(SIGALRM, on_alarm);
  alarm(5);
  printf("Line:");
  fflush(stdout);
  int r = scanf("%d", &n);
  alarm(0);

  if (time_skip) {
    char ch;
    lseek(fd, 0, SEEK_SET);
    while (read(fd, &ch, 1) == 1)
      putchar(ch);
    putchar('\n');
  }

  while (!time_skip) {
    if (r == 1) {
      if (n == 0)
        break;
      print_line(fd, table, count, n);
    } else {
      printf("Not chislo\n");
      if (r != 1) {
        int c;
        while ((c = getchar()) != '\n' && c != EOF) {
        }
      }
    }
    printf("Line:");
    fflush(stdout);
    r = scanf("%d", &n);
  }

  free(table);
  close(fd);
  return 0;
}
