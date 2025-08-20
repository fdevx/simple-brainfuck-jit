#include <iostream>
#include <format>
#include "Lexer.h"

Lexer::Lexer(std::istream &input) : input_stream(input), end_of_stream(false) {

}

Lexer::Token Lexer::getNext() {
    if (end_of_stream) return Lexer::eof;

    char c;
    while (true) {
        input_stream.get(c);

        if (input_stream.eof()) {
            end_of_stream = true;
            input_stream.clear();
            return Lexer::eof;
        }

        switch(c) {
            case '>': return Lexer::ptr_inc;
            case '<': return Lexer::ptr_dec;
            case '+': return Lexer::cell_inc;
            case '-': return Lexer::cell_dec;
            case '[': return Lexer::jmp_fwd;
            case ']': return Lexer::jmp_bwd;
            case ',': return Lexer::read;
            case '.': return Lexer::write;
            default:
                continue;
        }
    }
}

std::string Lexer::getPosition() {
    return std::string("position ") + std::to_string(input_stream.tellg());
}


std::string toString(Lexer::Token t) {
    switch (t) {
        case Lexer::ptr_inc: return ">";
        case Lexer::ptr_dec: return "<";
        case Lexer::cell_inc: return "+";
        case Lexer::cell_dec: return "-";
        case Lexer::jmp_fwd: return "[";
        case Lexer::jmp_bwd: return "]";
        case Lexer::read: return ",";
        case Lexer::write: return ".";
        case Lexer::eof: return "end of file";
    }
}
