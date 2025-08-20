#ifndef BRAINFUCKJIT_BRAINFUCKRUNTIME_H
#define BRAINFUCKJIT_BRAINFUCKRUNTIME_H

#include <cstdint>

typedef struct {
    std::vector<int64_t> cells;
    int64_t current_cell_index;
} BrainfuckState;

class brainfuck_out_of_bounds : public std::runtime_error {
public:
    explicit brainfuck_out_of_bounds(const char* err) : std::runtime_error(err) {}
};

extern "C" int64_t* loadCell(int64_t current_cell_ptr);

/**
 * Prints the @param cell_value to stdout as a character
 */
extern "C" void printCell(int64_t cell_value);

/**
 * Reads a character from stdin and returns it
 */
extern "C" int64_t getCell();


// Management functions
extern void setCurrentCellIndex(int64_t index);
extern int64_t getCurrentCellIndex();

extern std::unique_ptr<BrainfuckState>& getBrainfuckState();
extern void resetBrainfuckState();


#endif //BRAINFUCKJIT_BRAINFUCKRUNTIME_H
