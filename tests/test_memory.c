/* Exercise growth, stable links and exhaustion with real editor allocators. */
#include <assert.h>
#include <stdio.h>
#define PSY_C
#include "psy.h"
#include "psyext.h"
#include "psyfnc.h"

int main(int argc, char **argv)
{
    size_t i;
    cmcellpt_t cell, previous = 0;
    lncellpt_t *lines;
    (void)argc; (void)argv;
    buff_size = 8;
    buff_line = 4;
    buff_no = 3;
    d_tab_len = 4;
    assert(psy_init_memory());
    lines = calloc(MAX_BUFF_LINE, sizeof(*lines));
    assert(lines);

    for (i = 1; i <= MAX_BUFF_SIZE; i++) {
        /* This expression also tests reallocation during link assignment. */
        cell = putnext(previous, new_cm());
        assert(cell != 0);
        putletr(cell, (i % 0x10000) + 1);
        previous = cell;
        if (i == 1) push_cm(cell);
    }
    putnext(previous, 0);
    assert(new_cm() == 0);
    assert(sp_cm - cm_stack == 1);
    assert(pop_cm() == 1);
    cell = getnext(0);
    for (i = 1; i <= MAX_BUFF_SIZE; i++) {
        assert(cell != 0);
        assert(getletr(cell) == (wchar_t)((i % 0x10000) + 1));
        cell = getnext(cell);
    }
    assert(!cell);
    putletr(65536, 0);
    assert(new_cm() == 65536);
    putletr(65536, 'R');
    assert(new_cm() == 0);

    for (i = 0; i < MAX_BUFF_LINE; i++) {
        lines[i] = new_ln();
        assert(lines[i]);
        lines[i]->addr = (cmcellpt_t)(i + 1);
        if (i) lines[i-1]->link = lines[i];
        if (i == 0) push_ln(lines[i]);
    }
    assert(new_ln() == NULL);
    assert(psy_free_lines() == 0);
    assert(sp_ln - ln_stack == 2);
    assert(pop_ln() == lines[0]);
    for (i = 0; i < MAX_BUFF_LINE; i++) {
        assert(lines[i]->addr == i + 1);
        if (i) assert(lines[i-1]->link == lines[i]);
    }
    lines[32768]->addr = 0;
    assert(psy_free_lines() == 1);
    assert(new_ln() == lines[32768]);
    free(lines);
    puts("PASS: growth beyond 16-bit, stable links/stacks, exhaustion and reuse");
    return 0;
}
