#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

static volatile sig_atomic_t skip = 0;

void on_alarm(int s) {
  (void)s;
  skip = 1;
}

typedef struct {
  off_t offset;
  size_t length;
} LineInfo;

static void build_table_mmap(const char *p, off_t size, LineInfo **table,
                             size_t *count) {
  size_t cap = 0;
  *count = 0;
  *table = NULL;
  off_t start = 0;

  for (off_t i = 0; i < size; i++) {
    if (p[(size_t)i] == '\n') {
      size_t len = (size_t)(i - start);
      if (*count == cap) {
        cap = cap ? cap * 2 : 16;
        *table = realloc(*table, cap * sizeof(LineInfo));
      }
      (*table)[*count].offset = start;
      (*table)[*count].length = len;
      (*count)++;
      start = i + 1;
    }
  }
  if (size > start) {
    size_t len = (size_t)(size - start);
    if (*count == cap) {
      cap = cap ? cap * 2 : 16;
      *table = realloc(*table, cap * sizeof(LineInfo));
    }
    (*table)[*count].offset = start;
    (*table)[*count].length = len;
    (*count)++;
  }
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

static void print_line_mmap(const char *p, const LineInfo *table, size_t count,
                            int n) {
  if (n < 1 || (size_t)n > count) {
    puts("Not line");
    return;
  }
  LineInfo li = table[n - 1];
  size_t start = (size_t)li.offset;
  for (size_t i = 0; i < li.length; i++) {
    putchar(p[start + i]);
  }
  putchar('\n');
}

int main(void) {
  int fd = open("file.txt", O_RDONLY);
  if (fd < 0) {
    perror("open");
    return 1;
  }

  struct stat st;
  fstat(fd, &st);
  off_t size = st.st_size;

  char *p = NULL;
  if (size > 0) {
    p = mmap(NULL, size, PROT_READ, MAP_PRIVATE, fd, 0);
    if (p == MAP_FAILED) {
      perror("mmap");
      close(fd);
      return 1;
    }
  }

  LineInfo *table = NULL;
  size_t count = 0;
  if (size > 0) {
    build_table_mmap(p, size, &table, &count);
  }

  print_table(table, count);

  signal(SIGALRM, on_alarm);
  skip = 0;
  printf("Line: ");
  fflush(stdout);
  int n;
  alarm(5);
  int r = scanf("%d", &n);
  alarm(0);

  if (skip) {
    for (off_t i = 0; i < size; i++)
      putchar(p[i]);
    putchar('\n');
  }

  while (1 && !skip) {
    fflush(stdout);
    if (r == 1) {
      if (n == 0)
        break;
      print_line_mmap(p, table, count, n);
    } else {
      printf("Not chislo\n");
      int c;
      while ((c = getchar()) != '\n' && c != EOF)
        ;
    }
    printf("Line: ");
    fflush(stdout);
    r = scanf("%d", &n);
  }

  free(table);
  if (p != NULL) {
    munmap(p, size);
  }
  close(fd);
  return 0;
}
