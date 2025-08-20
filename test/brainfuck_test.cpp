#include <gtest/gtest.h>
#include <fcntl.h>
#include "../src/BrainfuckJIT.h"
#include "../src/ExprAST.h"
#include "../src/BrainfuckRuntime.h"

std::unique_ptr<BrainfuckJIT> brainfuckJit;

class BrainfuckEnvironment : public ::testing::Environment {
public:
    // Override this to define how to set up the environment.
    void SetUp() override {
        brainfuckJit = BrainfuckJIT::Create();
    }
};

testing::Environment* const brainfuck_env = testing::AddGlobalTestEnvironment(new BrainfuckEnvironment);

class BrainfuckTest : public testing::Test {
public:
    void SetUp() override {
        if (brainfuckJit != nullptr) {
            brainfuckIR = brainfuckJit->createIRBuilder();
        }
    }

    // You can define per-test tear-down logic as usual.
    void TearDown() override {
        if (brainfuckJit != nullptr) {
            brainfuckJit->resetBrainfuckState();
        }
    }

    static std::unique_ptr<BrainfuckIRBuilder> brainfuckIR;
};

class BasicInstructions : public BrainfuckTest {};

std::unique_ptr<BrainfuckIRBuilder> BrainfuckTest::brainfuckIR;


TEST_F(BasicInstructions, Add) {
    auto func = this->brainfuckIR->createAnonymousFunction();

    auto cell_expr = CellExprAST(true);
    cell_expr.codegen(brainfuckIR);
    cell_expr.codegen(brainfuckIR);
    cell_expr.codegen(brainfuckIR);
    cell_expr.codegen(brainfuckIR);

    brainfuckIR->finishAnonymousFunction();

    auto cell_val = brainfuckJit->executeFunction(brainfuckIR, func);
    EXPECT_EQ(cell_val, 4);
}

TEST_F(BasicInstructions, Sub) {
    auto func = brainfuckIR->createAnonymousFunction();

    auto cell_expr = CellExprAST(false);
    cell_expr.codegen(brainfuckIR);
    cell_expr.codegen(brainfuckIR);
    cell_expr.codegen(brainfuckIR);
    cell_expr.codegen(brainfuckIR);

    brainfuckIR->finishAnonymousFunction();

    auto cell_val = brainfuckJit->executeFunction(brainfuckIR, func);
    EXPECT_EQ(cell_val, -4);
}

TEST_F(BasicInstructions, IncPtr) {
    auto func = brainfuckIR->createAnonymousFunction();

    auto cell_expr = CellExprAST(true);
    auto ptr_expr = PtrExprAST(true);
    cell_expr.codegen(brainfuckIR);
    ptr_expr.codegen(brainfuckIR);

    cell_expr.codegen(brainfuckIR);
    ptr_expr.codegen(brainfuckIR);

    cell_expr.codegen(brainfuckIR);
    ptr_expr.codegen(brainfuckIR);

    cell_expr.codegen(brainfuckIR);
    ptr_expr.codegen(brainfuckIR);

    brainfuckIR->finishAnonymousFunction();

    auto cell_val = brainfuckJit->executeFunction(brainfuckIR, func);
    EXPECT_EQ(cell_val, 0);
}

TEST_F(BasicInstructions, DecPtr) {
    auto func = brainfuckIR->createAnonymousFunction();

    auto cell_expr = CellExprAST(true);
    auto ptr_expr_inc = PtrExprAST(true);
    auto ptr_expr_dec = PtrExprAST(false);

    cell_expr.codegen(brainfuckIR);
    cell_expr.codegen(brainfuckIR);
    cell_expr.codegen(brainfuckIR);
    cell_expr.codegen(brainfuckIR);
    ptr_expr_inc.codegen(brainfuckIR);

    cell_expr.codegen(brainfuckIR);
    ptr_expr_dec.codegen(brainfuckIR);

    brainfuckIR->finishAnonymousFunction();

    auto cell_val = brainfuckJit->executeFunction(brainfuckIR, func);
    EXPECT_EQ(cell_val, 4);
}

TEST_F(BasicInstructions, Print) {
    auto old_stdout = dup(1);

    int pipefd[2];
    pipe(pipefd);

    std::string wanted_output = "Hello World!";

    try {
        auto func = brainfuckIR->createAnonymousFunction();

        auto cell_expr_inc = CellExprAST(true);
        auto cell_expr_dec = CellExprAST(false);
        auto ptr_expr_inc = PtrExprAST(true);
        auto cell_print = WriteExprAST();

        int cell_val = 0;
        for (auto c : wanted_output) {
            if (c > cell_val) {
                for (; cell_val < c; cell_val++) {
                    cell_expr_inc.codegen(brainfuckIR);
                }
            } else {
                for (; cell_val > c; cell_val--) {
                    cell_expr_dec.codegen(brainfuckIR);
                }
            }

            cell_print.codegen(brainfuckIR);
            //ptr_expr_inc.codegen(brainfuckIR);
        }
        brainfuckIR->finishAnonymousFunction();

        dup2(pipefd[1], 1);
        brainfuckJit->executeFunction(brainfuckIR, func);
    } catch (...) {
    }
    dup2(old_stdout, 1);

    char buff[1024];
    std::memset(buff, 0, sizeof buff);
    read(pipefd[0], buff, sizeof buff);

    EXPECT_EQ(wanted_output, buff);

    close(pipefd[0]);
    close(pipefd[1]);
}

TEST_F(BasicInstructions, Read) {
    auto old_stdin = dup(0);

    int pipefd[2];
    pipe(pipefd);

    try {
        char wanted_input = '!';
        write(pipefd[1], &wanted_input, 1);

        auto func = brainfuckIR->createAnonymousFunction();

        auto cell_print = ReadExprAST();
        cell_print.codegen(brainfuckIR);

        brainfuckIR->finishAnonymousFunction();

        dup2(pipefd[0], 0);
        auto cell_value = brainfuckJit->executeFunction(brainfuckIR, func);

        EXPECT_EQ(wanted_input, cell_value);
    } catch (...) {
    }

    dup2(old_stdin, 0);
    close(pipefd[0]);
    close(pipefd[1]);
}

TEST_F(BrainfuckTest, ModifiesState) {
    auto func = brainfuckIR->createAnonymousFunction();

    auto& builder = brainfuckIR->getBuilder();
    brainfuckIR->setCurrentCellValue(builder->getInt64(42));

    brainfuckIR->finishAnonymousFunction();

    brainfuckJit->executeFunction(brainfuckIR, func);

    auto& state = getBrainfuckState();
    EXPECT_EQ(state->cells[0], 42);
}

TEST_F(BrainfuckTest, NoNegativeCells) {
    auto func = brainfuckIR->createAnonymousFunction();

    auto ptr_expr_inc = PtrExprAST(false);
    ptr_expr_inc.codegen(brainfuckIR);

    brainfuckIR->finishAnonymousFunction();

    EXPECT_THROW(brainfuckJit->executeFunction(brainfuckIR, func), brainfuck_out_of_bounds);
}