#include <iostream>
#include <format>
#include "Parser.h"

Parser::Parser(Lexer &lex) : lexer(lex) {

}

std::optional<std::unique_ptr<ExprAST>> Parser::tokenToExpr(Lexer::Token token) {
    switch(token) {
        case Lexer::ptr_inc:
            return std::make_unique<PtrExprAST>(true);
        case Lexer::ptr_dec:
            return std::make_unique<PtrExprAST>(false);

        case Lexer::cell_inc:
            return std::make_unique<CellExprAST>(true);
        case Lexer::cell_dec:
            return std::make_unique<CellExprAST>(false);

        case Lexer::read:
            return std::make_unique<ReadExprAST>();
        case Lexer::write:
            return std::make_unique<WriteExprAST>();

        case Lexer::jmp_fwd:
            return parseLoop();
        case Lexer::jmp_bwd:
            throw parse_error(std::format("Unexpected token '{}' at {}", toString(Lexer::jmp_bwd), lexer.getPosition()));

        default:
            return std::nullopt;
    }
}

std::optional<std::unique_ptr<ExprAST>> Parser::nextExpression() {
    return tokenToExpr(lexer.getNext());
}

std::optional<std::unique_ptr<LoopExprAST>> Parser::parseLoop() {
    std::vector<std::unique_ptr<ExprAST>> loopBody;
    auto startPos = lexer.getPosition();

    for(auto t = lexer.getNext(); t != Lexer::jmp_bwd; t = lexer.getNext()) {
        auto expr = tokenToExpr(t);

        if (!expr.has_value()) {
            // This error handling is not the best but at least you know something went wrong and the program doesn't crash
            if (t == Lexer::eof) {
                throw parse_error(std::format("Token '{}' at {} has no matching '{}'", toString(Lexer::jmp_fwd),
                                          startPos, toString(Lexer::jmp_bwd)));
            } else {
                throw parse_error(std::format("Unexpected token '{}' at {}", toString(t), lexer.getPosition()));
            }
            return std::nullopt;
        }
        loopBody.emplace_back(std::move(expr.value()));
    }

    return std::make_unique<LoopExprAST>(std::move(loopBody));
}


