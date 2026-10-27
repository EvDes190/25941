#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <poll.h>
#include <signal.h>
#include <termios.h>
#include <unistd.h>

#define MAX_LINE 1024

static struct termios saved_terminal;
static int terminal_active = 0;

static void restore_terminal(void)
{
    if (terminal_active)
        tcsetattr(STDIN_FILENO, TCSANOW, &saved_terminal);
}

static void stop_input(int sig)
{
    restore_terminal();
    signal(sig, SIG_DFL);
    raise(sig);
}

static int setup_terminal(void)
{
    struct termios settings;
    if (!isatty(STDIN_FILENO))
        return 0;
    if (tcgetattr(STDIN_FILENO, &saved_terminal) == -1)
        return -1;
    settings = saved_terminal;
    settings.c_lflag &= ~(ICANON | ECHO | ECHONL);
    settings.c_cc[VMIN] = 1;
    settings.c_cc[VTIME] = 0;
    if (tcsetattr(STDIN_FILENO, TCSANOW, &settings) == -1)
        return -1;
    terminal_active = 1;
    atexit(restore_terminal);
    signal(SIGINT, stop_input);
    signal(SIGTERM, stop_input);
    signal(SIGHUP, stop_input);
    signal(SIGQUIT, stop_input);
    return 0;
}

static int read_byte(void)
{
    unsigned char c;
    ssize_t result;
    do {
        result = read(STDIN_FILENO, &c, 1);
    } while (result == -1 && errno == EINTR);
    return result == 1 ? c : EOF;
}

/* Consume terminal key sequences, including ESC [ A and ESC O A. */
static void skip_escape(void)
{
    struct pollfd input;
    int c;
    int first = 1;
    input.fd = STDIN_FILENO;
    input.events = POLLIN;
    while (poll(&input, 1, 100) > 0)
    {
        c = read_byte();
        if (c == EOF)
            return;
        if (first)
        {
            if (c != '[' && c != 'O')
                return;
            first = 0;
        }
        else if (c >= 0x40 && c <= 0x7e)
            return;
    }
}

static char *read_line(char *buffer, size_t capacity)
{
    size_t length = 0;
    int c;
    while ((c = read_byte()) != EOF)
    {
        if (c == 27)
        {
            skip_escape();
            continue;
        }
        if (terminal_active && c == saved_terminal.c_cc[VEOF])
        {
            if (length == 0)
                return NULL;
            break;
        }
        if (terminal_active && (c == 127 || c == 8))
        {
            if (length > 0)
            {
                /* Remove the complete UTF-8 character. */
                do { length--; }
                while (length > 0 &&
                       ((unsigned char)buffer[length] & 0xc0) == 0x80);
                fputs("\b \b", stdout);
                fflush(stdout);
            }
            continue;
        }
        if (c == '\n')
        {
            buffer[length++] = '\n';
            if (terminal_active)
                putchar('\n');
            break;
        }
        if (c < 32 || c == 127)
            continue;
        if (length < capacity - 2)
        {
            buffer[length++] = (char)c;
            if (terminal_active)
            {
                putchar(c);
                fflush(stdout);
            }
        }
    }
    buffer[length] = '\0';
    return length == 0 ? NULL : buffer;
}

struct Node
{
    char *text;
    struct Node *next;
};

int main(void)
{
    struct Node *head = NULL;
    struct Node *tail = NULL;

    char buffer[MAX_LINE];

    if (setup_terminal() == -1)
    {
        perror("terminal");
        return 1;
    }

    printf("Enter strings.\n");
    printf("Enter '.' to finish.\n\n");
    fflush(stdout);

    while (1)
    {
        if (read_line(buffer, sizeof(buffer)) == NULL)
        {
            break;
        }

        if (buffer[0] == '.')
        {
            break;
        }

        size_t length = strlen(buffer);

        struct Node *new_node = malloc(sizeof(struct Node));

        if (new_node == NULL)
        {
            perror("malloc");
            return 1;
        }

        new_node->text = malloc(length + 1);

        if (new_node->text == NULL)
        {
            perror("malloc");
            free(new_node);
            return 1;
        }

        strcpy(new_node->text, buffer);

        new_node->next = NULL;

        if (head == NULL)
        {
            head = new_node;
            tail = new_node;
        }
        else
        {
            tail->next = new_node;
            tail = new_node;
        }
    }

    restore_terminal();
    terminal_active = 0;
    printf("\n--- LIST ---\n");

    struct Node *current = head;

    while (current != NULL)
    {
        printf("%s", current->text);
        current = current->next;
    }

    current = head;

    while (current != NULL)
    {
        struct Node *next = current->next;

        free(current->text);
        free(current);

        current = next;
    }

    return 0;
}

