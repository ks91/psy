/* Terminal geometry and resize-aware input. Never draw from a signal handler. */
#include <stdio.h>
#include <errno.h>
#include <fcntl.h>
#include <poll.h>
#include <signal.h>
#include <stdarg.h>
#include <sys/ioctl.h>
#include "psy.h"
#include "psyext.h"
#include "psyfnc.h"

int psy_columns = 80, psy_rows = 24;
static int resize_pipe[2] = {-1, -1};
static volatile sig_atomic_t resize_pending;
static int too_small;
static int unread_byte = EOF;
static char status_text[PATH_SIZE * 2 + 1024];

static void resized(int sig)
{
    int saved_errno = errno;
    char byte = 0;
    (void)sig;
    resize_pending = 1;
    if (resize_pipe[1] >= 0) (void)write(resize_pipe[1], &byte, 1);
    errno = saved_errno;
}

int psy_screen_small(void) { return too_small; }

void psy_locate(int x, int y)
{
    if (too_small) return;
    if (x < 0) x = 0;
    if (x >= WIDTH) x = WIDTH - 1;
    if (y < 0) y = 0;
    if (y > BOTTOM) y = BOTTOM;
    printf("\033[%d;%dH", y + 1, x + 1);
}

void psy_print_clipped(const char *text, int columns)
{
    char_t *next = (char_t *)text;
    int c, width, used = 0;
    while (*next) {
        c = psy_next_char(&next);
        if (c < ' ' || c == DEL || (c >= 0x80 && c < 0xa0)) c = '?';
        width = psy_char_width(c);
        if (used + width > columns) break;
        putwchar(c);
        used += width;
    }
}

static void draw_status(void)
{
    if (too_small) return;
    locate(0, BOTTOM);
    cls_ln();
    psy_print_clipped(status_text, WIDTH - 1);
}

void psy_statusf(const char *format, ...)
{
    va_list args;
    va_start(args, format);
    vsnprintf(status_text, sizeof(status_text), format, args);
    va_end(args);
    draw_status();
}

void psy_mode_bar(void)
{
    int i;
    const char *prefix = "Psy:=[=]=[=====]=[";
    locate(0, ceiling + height);
    cls_ln();
    psy_print_clipped(prefix, WIDTH - 1);
    for (i = (int)strlen(prefix); i < WIDTH - 1; i++) putchar('=');
}

void psy_mode_name(const char *name)
{
    locate(18, ceiling + height);
    cls_ln();
    psy_print_clipped(name, WIDTH - 20);
    putchar(']');
}

static void resize_screen(int force)
{
    struct winsize size;
    wdcell_t *wd;
    int count = 0, total = 0, remaining, top = 0, weight;
    int saved_repeat = rp;
    int columns = psy_columns, rows = psy_rows;
    int was_small = too_small;

    if (!cur_wdw) return;
    resize_pending = 0;
    if (ioctl(STDIN_FILENO, TIOCGWINSZ, &size) == 0) {
        if (size.ws_col) columns = size.ws_col;
        if (size.ws_row) rows = size.ws_row;
    }
    for (wd = wdw_list; wd; wd = wd->wd_link) {
        count++;
        total += wd->wd_heig;
    }
    /* Every edit window needs a text row and mode row, plus two bottom rows. */
    too_small = columns < 20 || rows < count * 2 + 2;
    if (too_small) {
        printf("\033[m\033[2J\033[1;1H");
        psy_print_clipped("Enlarge terminal", columns > 1 ? columns - 1 : 0);
        fflush(stdout);
        return;
    }
    if (!force && !was_small && columns == psy_columns && rows == psy_rows) return;

    sav_wat(cur_wdw);
    psy_columns = columns;
    psy_rows = rows;
    (void)resizeterm(rows, columns);
    /* Keep split proportions, with at least one text row in every window. */
    remaining = rows - 2 - count;
    weight = total;
    for (wd = wdw_list; wd; wd = wd->wd_link) {
        int old_height = wd->wd_heig;
        int next_height = count == 1 ? remaining :
            1 + (int)((long)(remaining - count) * old_height / weight);
        wd->wd_ceil = top;
        wd->wd_heig = next_height;
        top += next_height + 1;
        remaining -= next_height;
        weight -= old_height;
        count--;
    }
    edit_wdw->wd_ceil = rows - 2;
    edit_wdw->wd_heig = 1;
    loa_wat(cur_wdw); /* dsp_scr must save the new geometry, not the old one. */
    cls();
    dsp_scr();
    rp = saved_repeat;
    draw_status();
    locate(loc_x(), loc_y());
    fflush(stdout);
}

void psy_terminal_resume(void)
{
    struct sigaction action;
    int i;
    if (resize_pipe[0] < 0) {
        if (pipe(resize_pipe)) { perror("psy: resize pipe"); exit(1); }
        for (i = 0; i < 2; i++) {
            if (fcntl(resize_pipe[i], F_SETFL, O_NONBLOCK) == -1 ||
                fcntl(resize_pipe[i], F_SETFD, FD_CLOEXEC) == -1) {
                perror("psy: resize pipe flags"); exit(1);
            }
        }
    }
    memset(&action, 0, sizeof(action));
    sigemptyset(&action.sa_mask);
    action.sa_handler = resized;
    if (sigaction(SIGWINCH, &action, NULL)) {
        perror("psy: SIGWINCH"); exit(1);
    }
    resize_screen(1);
}

void psy_terminal_unread(int c) { unread_byte = c; }

int psy_terminal_byte(void)
{
    struct pollfd inputs[2];
    char discarded[64];
    unsigned char byte;
    ssize_t n;
    for (;;) {
        if (resize_pending) {
            while (read(resize_pipe[0], discarded, sizeof(discarded)) > 0) {}
            resize_screen(0);
        }
        if (!too_small && unread_byte != EOF) {
            int result = unread_byte;
            unread_byte = EOF;
            return result;
        }
        fflush(stdout);
        inputs[0].fd = too_small ? -1 : STDIN_FILENO;
        inputs[0].events = POLLIN;
        inputs[1].fd = resize_pipe[0];
        inputs[1].events = POLLIN;
        if (poll(inputs, 2, -1) < 0) {
            if (errno == EINTR) continue;
            return EOF;
        }
        if (inputs[1].revents & POLLIN) {
            resize_pending = 1;
            continue;
        }
        if (inputs[0].revents & (POLLIN | POLLHUP | POLLERR)) {
            n = read(STDIN_FILENO, &byte, 1);
            if (n == 1) return byte;
            if (n < 0 && errno == EINTR) continue;
            return EOF;
        }
    }
}
