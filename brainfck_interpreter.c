#include <stdlib.h>
#include <stdio.h>

#define NUM_CELLS 30000
#define INSTR_ARR_MIN_SIZE 16
#define INSTR_PREV '<'
#define INSTR_NEXT '>'
#define INSTR_INCR '+'
#define INSTR_DECR '-'
#define INSTR_PRNT '.'
#define INSTR_READ ','
#define INSTR_CJMP_FWD '['
#define INSTR_CJMP_BCK ']'
#define INSTR_STOP '\0'
#define BF_STATUS_TERMINATE (-1)
#define BF_STATUS_SUCCESS 0
#define BF_STATUS_SYNTAX_ERR 1

typedef char instr_t;
typedef char cell_t;

int is_ignored_instr(instr_t c) {
        return     c != INSTR_PREV
                && c != INSTR_NEXT
                && c != INSTR_INCR
                && c != INSTR_DECR
                && c != INSTR_PRNT
                && c != INSTR_READ
                && c != INSTR_CJMP_FWD
                && c != INSTR_CJMP_BCK
                && c != INSTR_STOP;
}

instr_t *load_program(FILE *stream, size_t *ret_num_instructions) {
        if (stream == NULL) return NULL;
        size_t curr_size = 0, max_size = INSTR_ARR_MIN_SIZE;
        int c;
        instr_t *instructions = (instr_t *)malloc(max_size * sizeof(instr_t));

        while ((c = getc(stream)) != EOF) {
                if (curr_size+1 >= max_size) {
                        max_size *= 2;
                        instructions = (instr_t *)realloc(instructions, max_size * sizeof(instr_t));
                }
                instructions[curr_size++] = c;
        }
        instructions[curr_size++] = INSTR_STOP;
        *ret_num_instructions = curr_size;
        return instructions;
}

// <
int instr_prev(size_t *cell_index) {
        if (*cell_index == 0) return BF_STATUS_SUCCESS;
        *cell_index = *cell_index - 1;
        return BF_STATUS_SUCCESS;
}

// >
int instr_next(size_t *cell_index) {
        if (*cell_index >= NUM_CELLS-1) return BF_STATUS_SUCCESS;
        *cell_index = *cell_index + 1;
        return BF_STATUS_SUCCESS;
}

// +
int instr_incr(cell_t *cells, size_t cell_index) {
        cells[cell_index]++;
        return BF_STATUS_SUCCESS;
}

// -
int instr_decr(cell_t *cells, size_t cell_index) {
        cells[cell_index]--;
        return BF_STATUS_SUCCESS;
}

// .
int instr_prnt(cell_t *cells, size_t cell_index) {
        printf("%c", cells[cell_index]);
        return BF_STATUS_SUCCESS;
}

// ,
int instr_read(cell_t *cells, size_t cell_index) {
        cell_t c = getchar();
        cells[cell_index] = c;
        return BF_STATUS_SUCCESS;
}

// [
int instr_cjmp_fwd(cell_t *cells, size_t cell_index, instr_t *instructions, size_t *instr_index, size_t num_instructions) {
        if (cells[cell_index] != 0) return BF_STATUS_SUCCESS;
        if (*instr_index == num_instructions-1) return BF_STATUS_SYNTAX_ERR;
        int no_match_found = 1, ignore_count = 0;
        size_t curr_index = *instr_index + 1;
        for (; curr_index < num_instructions; ++curr_index) {
                instr_t curr = instructions[curr_index];
                if (curr == INSTR_CJMP_FWD) ++ignore_count;
                if (curr == INSTR_CJMP_BCK) {
                        if (ignore_count > 0) {
                                ignore_count--;
                        } else {
                                no_match_found = 0;
                                break;
                        }
                }
        }
        if (no_match_found) return BF_STATUS_SYNTAX_ERR;
        *instr_index = curr_index;
        return BF_STATUS_SUCCESS;
}

// ]
int instr_cjmp_bck(cell_t *cells, size_t cell_index, instr_t *instructions, size_t *instr_index) {
        if (cells[cell_index] == 0) return BF_STATUS_SUCCESS;
        if (*instr_index == 0) return BF_STATUS_SYNTAX_ERR;
        int no_match_found = 1, ignore_count = 0;
        size_t curr_index = *instr_index - 1;
        for (; curr_index >= 0; --curr_index) {
                instr_t curr = instructions[curr_index];
                if (curr == INSTR_CJMP_BCK) ++ignore_count;
                if (curr == INSTR_CJMP_FWD) {
                        if (ignore_count > 0) {
                                ignore_count--;
                        } else {
                                no_match_found = 0;
                                break;
                        }
                }
        }
        if (no_match_found) return BF_STATUS_SYNTAX_ERR;
        *instr_index = curr_index;
        return BF_STATUS_SUCCESS;
}

void execute_program(instr_t *instructions, size_t num_instructions) {
        size_t cell_index = 0;
        size_t instr_index = 0;
        int status = 0;
        cell_t cells[NUM_CELLS];
        for (size_t i = 0; i < NUM_CELLS; ++i) cells[i] = 0;
        while (status == 0) {
                //printf("cell_i: %lu, instr_i: %lu, status: %d\n", cell_index, instr_index, status);
                if (instr_index >= num_instructions) break;
                instr_t instr = instructions[instr_index];
                //printf("instr: %c, %d\n", instr, instr);
                if (is_ignored_instr(instr)) {
                        instr_index++;
                        continue;
                }
                switch (instr) {
                        case INSTR_PREV:
                                status = instr_prev(&cell_index);
                                break;
                        case INSTR_NEXT:
                                status = instr_next(&cell_index);
                                break;
                        case INSTR_INCR:
                                status = instr_incr(cells, cell_index);
                                break;
                        case INSTR_DECR:
                                status = instr_decr(cells, cell_index);
                                break;
                        case INSTR_PRNT:
                                status = instr_prnt(cells, cell_index);
                                break;
                        case INSTR_READ:
                                status = instr_read(cells, cell_index);
                                break;
                        case INSTR_CJMP_FWD:
                                status = instr_cjmp_fwd(cells, cell_index, instructions, &instr_index, num_instructions);
                                break;
                        case INSTR_CJMP_BCK:
                                status = instr_cjmp_bck(cells, cell_index, instructions, &instr_index);
                                break;
                        case INSTR_STOP:
                        default:
                                status = BF_STATUS_TERMINATE;
                                break;
                }
                instr_index++;
        }
}

int main(int argc, char *argv[]) {
        if (argc != 2) {
                fprintf(stderr, "Usage: %s <path_to_brainfck_code>\n", argv[0]);
                return EXIT_FAILURE;
        }
        FILE *f = fopen(argv[1], "r");
        if (f == NULL) {
                fprintf(stderr, "Could not open file %s\n", argv[1]);
                return EXIT_FAILURE;
        }
        size_t num_instructions;
        instr_t *instructions = NULL;
        instructions = load_program(f, &num_instructions);
        fclose(f);
        execute_program(instructions, num_instructions);
        free(instructions);
        instructions = NULL;
        return 0;
}
