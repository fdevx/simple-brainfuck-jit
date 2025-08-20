#include <vector>
#include <memory>
#include "BrainfuckRuntime.h"

static std::unique_ptr<BrainfuckState> brainfuck_state;

extern void setCurrentCellIndex(int64_t index) {
    auto&  bf_state = getBrainfuckState();
    bf_state->current_cell_index = index;
}

extern int64_t getCurrentCellIndex() {
    auto&  bf_state = getBrainfuckState();
    return bf_state->current_cell_index;
}

std::unique_ptr<BrainfuckState>& getBrainfuckState() {
    if (brainfuck_state == nullptr) {
        brainfuck_state = std::make_unique<BrainfuckState>();
        brainfuck_state->cells.resize(brainfuck_state->current_cell_index + 1);
        //brainfuck_state->cells.reserve(30000);
    }
    return brainfuck_state;
}

void resetBrainfuckState() {
    brainfuck_state = std::make_unique<BrainfuckState>();
}


extern "C" int64_t* loadCell(int64_t current_cell_ptr) {
    if (current_cell_ptr < 0) throw brainfuck_out_of_bounds("Tried to access negative cells");
    auto&  bf_state = getBrainfuckState();

    auto& cells = bf_state->cells;
    if (cells.size() <= current_cell_ptr)
        cells.resize(current_cell_ptr + 1);

    auto* cell_val = &cells.at(current_cell_ptr);
    //fprintf(stderr, "Fetching cell [%ld] => %ld (%lX) (%p)\n", current_cell_ptr, *cell_val, *cell_val, cell_val);

    return cell_val;
}

extern "C" void printCell(int64_t cell_value) {
    putc((char)cell_value, stdout);
}


extern "C" int64_t getCell() {
    int c =  getc(stdin);
    auto ret = (int64_t) c;

    return ret;
}