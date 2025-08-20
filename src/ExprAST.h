#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Value.h>
#include <memory>
#include "BrainfuckIRBuilder.h"


class ExprAST {
public:
    virtual ~ExprAST() = default;
    virtual llvm::Value* codegen(std::unique_ptr<BrainfuckIRBuilder> &birb) = 0;
};

class CellExprAST : public ExprAST {
    bool should_increment;
public:
    explicit CellExprAST(bool should_inc);

    llvm::Value* codegen(std::unique_ptr<BrainfuckIRBuilder> &birb) override;
};

class PtrExprAST : public ExprAST {
    bool should_increment;
public:
    explicit PtrExprAST(bool should_inc);

    llvm::Value* codegen(std::unique_ptr<BrainfuckIRBuilder> &birb) override;
};

class WriteExprAST : public ExprAST {
public:
    llvm::Value* codegen(std::unique_ptr<BrainfuckIRBuilder> &birb) override;
};

class ReadExprAST : public ExprAST {
public:
    llvm::Value* codegen(std::unique_ptr<BrainfuckIRBuilder> &birb) override;
};

class LoopExprAST : public ExprAST {
    std::vector<std::unique_ptr<ExprAST>> Body;
public:
    explicit LoopExprAST(std::vector<std::unique_ptr<ExprAST>> Body);

    llvm::Value* codegen(std::unique_ptr<BrainfuckIRBuilder> &birb) override;
};
