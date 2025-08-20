#ifndef BRAINFUCKJIT_LEXER_H
#define BRAINFUCKJIT_LEXER_H

#include <sstream>

class Lexer {
public:
    enum Token {
        eof, // end of file stop reading

        ptr_inc, // '>' increment pointer
        ptr_dec, // '<' decrement pointer

        cell_inc, // '+' increment active cell
        cell_dec, // '-' decrement active cell

        jmp_fwd, // '[' jump to matching jmp_bwd if active cell is 0
        jmp_bwd, // ']' jump to matching jmp_fwd if active cell is NOT 0

        read, // ',' read one byte from stdin and store in active cell
        write // '.' write one byte to stdout from active cell
    };
private:
    std::istream& input_stream;
    bool end_of_stream;
public:
    explicit Lexer(std::istream& input);

    /**
     * @return Next valid token found in input
     */
    virtual Token getNext();

    virtual std::string getPosition();
};

std::string toString(Lexer::Token t);

#endif //BRAINFUCKJIT_LEXER_H
