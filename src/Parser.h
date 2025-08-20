#ifndef BRAINFUCKJIT_PARSER_H
#define BRAINFUCKJIT_PARSER_H

#include <optional>
#include "Lexer.h"
#include "ExprAST.h"

class Parser {
    Lexer& lexer;

    std::optional<std::unique_ptr<ExprAST>> tokenToExpr(Lexer::Token token);
public:
    explicit Parser(Lexer& lex);

    std::optional<std::unique_ptr<ExprAST>> nextExpression();

    std::optional<std::unique_ptr<LoopExprAST>> parseLoop();
};

class parse_error : public std::runtime_error {
public:
    explicit parse_error(const char* c) : std::runtime_error(c) {}
    explicit parse_error(const std::string s) : std::runtime_error(s) {}
};

#endif //BRAINFUCKJIT_PARSER_H
