/* Growing cell pools. Character links are indices, line addresses stay stable. */
#include <stdio.h>
#include "psy.h"
#include "psyext.h"

struct line_chunk {
    struct line_chunk *next;
    size_t count;
    lncell_t cells[];
};
static struct line_chunk *chunks, *current_chunk;
static size_t line_index;

static int grow_lines(size_t count)
{
    struct line_chunk *chunk;
    lncellpt_t *stack;
    size_t used = sp_ln ? (size_t)(sp_ln - ln_stack) : 0;
    size_t total = chunks ? (size_t)buff_line : 0;
    if (count > MAX_BUFF_LINE - total) return 0;
    chunk = calloc(1, sizeof(*chunk) + count * sizeof(lncell_t));
    if (!chunk) return 0;
    stack = realloc(ln_stack, (total + count + 2) * sizeof(*stack));
    if (!stack) { free(chunk); return 0; }
    /* Entry zero is reserved so spln_base never points before the allocation. */
    ln_stack = stack;
    sp_ln = stack + used;
    spln_base = stack;
    chunk->count = count;
    if (!chunks) {
        chunks = chunk;
        cellbase = chunk->cells;
        celltop = cellbase + count;
    } else {
        chunk->next = chunks;
        chunks = chunk;
    }
    current_chunk = chunk;
    line_index = 0;
    buff_line = (long)(total + count);
    return 1;
}

static int grow_chars(size_t count)
{
    cmcell_t *cells;
    cmcellpt_t *stack;
    size_t old = heap ? (size_t)buff_size : 0;
    size_t used = sp_cm ? (size_t)(sp_cm - cm_stack) : 0;
    if (count <= old || count > MAX_BUFF_SIZE) return 0;
    stack = realloc(cm_stack, (count + 2) * sizeof(*stack));
    if (!stack) return 0;
    cm_stack = stack;
    sp_cm = stack + used;
    cells = realloc(heap, (count + 1) * sizeof(*cells));
    if (!cells) return 0;
    heap = cells;
    memset(heap + old + 1, 0, (count - old) * sizeof(*cells));
    if (!old) memset(heap, 0, sizeof(*heap));
    buff_size = (long)count;
    return 1;
}

int psy_init_memory(void)
{
    size_t characters = (size_t)buff_size, lines = (size_t)buff_line;
    if (characters < 8 || lines < 4 || buff_no < 3 || tab_len < 0 ||
        d_tab_len < 1 || d_tab_len > MAX_TAB_LENGTH) return 0;
    if (!grow_chars(characters) || !grow_lines(lines)) return 0;
    sp_ln = ln_stack + 1;
    cm_free = 1;
    return 1;
}

cmcellpt_t new_cm(void)
{
    cmcellpt_t start = cm_free;
    do {
        if (!getletr(cm_free)) return cm_free;
        if (++cm_free > (cmcellpt_t)buff_size) cm_free = 1;
    } while (cm_free != start);
    {
        size_t old = (size_t)buff_size;
        size_t next = old > MAX_BUFF_SIZE / 2 ? MAX_BUFF_SIZE : old * 2;
        if (!grow_chars(next)) return 0;
        cm_free = (cmcellpt_t)(old + 1);
        return cm_free;
    }
}

lncellpt_t new_ln(void)
{
    size_t visited = 0;
    while (visited++ < (size_t)buff_line) {
        lncellpt_t cell = &current_chunk->cells[line_index];
        if (!cell->addr) return cell;
        if (++line_index == current_chunk->count) {
            line_index = 0;
            current_chunk = current_chunk->next ? current_chunk->next : chunks;
        }
    }
    {
        size_t more = (size_t)buff_line;
        if (more > MAX_BUFF_LINE - buff_line) more = MAX_BUFF_LINE - buff_line;
        if (!more || !grow_lines(more)) return NULL;
        return &current_chunk->cells[0];
    }
}

long psy_free_lines(void)
{
    struct line_chunk *chunk;
    size_t i;
    long count = 0;
    for (chunk = chunks; chunk; chunk = chunk->next)
        for (i = 0; i < chunk->count; i++)
            if (!chunk->cells[i].addr) count++;
    return count;
}

/* Evaluate index and value before accessing heap: new_cm() may relocate it. */
cmcellpt_t psy_putnext(cmcellpt_t cell, cmcellpt_t next)
{
    return heap[cell].next = next;
}
