#include <iostream>
#include "BrainfuckIRBuilder.h"
#include "BrainfuckJIT.h"
#include "Lexer.h"
#include "Parser.h"
#include "BrainfuckRuntime.h"
#include <chrono>
#include <unistd.h>
#include <fstream>
#include <filesystem>

#define MAX_NUM_SHOW 20

// https://llvm.org/docs/tutorial/MyFirstLanguageFrontend/LangImpl02.html
bool showPerformanceStatistics = false;
bool printStateAtExit = true;
bool printState = false;

void printBFStateStats() {
    auto& state = getBrainfuckState();

    std::cout << std::format("Current cell [{}] = {}\n", state->current_cell_index, state->cells[state->current_cell_index]);
    std::cout << std::format("Number of used cells: {}\n", state->cells.size());

    auto first = state->cells.begin();
    auto last = state->cells.size() >= MAX_NUM_SHOW ? state->cells.begin() + MAX_NUM_SHOW : state->cells.end();
    std::stringstream join;
    join << *first;
    for(auto it = ++first; it != last; it++) {
        join << ", " << *it;
    }

    if (last != state->cells.end()) {
        join << "...";
    }
    std::cout << "Cells: \n" << join.str() << "\n";
}

void executeInputStream(std::unique_ptr<BrainfuckJIT>& jit, std::istream& istream) {
    std::unique_ptr<BrainfuckIRBuilder> brainfuckIR = jit->createIRBuilder();
    Lexer lex(istream);
    Parser p(lex);

    auto IRGenerationStart = std::chrono::high_resolution_clock::now();

    auto func = brainfuckIR->createAnonymousFunction();

    bool hasCode = false;

    try {
        for (auto expr = p.nextExpression(); expr.has_value(); expr = p.nextExpression()) {
            expr.value()->codegen(brainfuckIR);
            hasCode = true;
        }
    } catch (parse_error& e) {
        std::cout << "Parse error: " << e.what() << std::endl;
        return;
    }

    if (!hasCode) {
        std::cout << "No code to execute\n";
        return;
    }

    auto OptimizationBegin = std::chrono::high_resolution_clock::now();
    brainfuckIR->finishAnonymousFunction();

    //brainfuckIR->print();

    std::cout.flush();
    auto ExecutionStart = std::chrono::high_resolution_clock::now();
    int64_t cur_cell_val;
    try {
        cur_cell_val = jit->executeFunction(brainfuckIR, func);
    } catch (std::exception& e) {
        std::cout << "Error: " << e.what() << std::endl;
    }

    auto ExecutionEnd = std::chrono::high_resolution_clock::now();

    using namespace std::chrono_literals;
    using namespace std::chrono;
    using namespace std;

    std::cout << endl;
    std::cout << "Active cell value: " << cur_cell_val << "\n";

    if (printState) {
        printBFStateStats();
    }

    if (showPerformanceStatistics) {
        cout << "\n";
        cout << "Brainfuck JIT Runtime: " << duration<double>(ExecutionEnd - IRGenerationStart) << "\n";
        cout << "    IR Generation:     " << duration<double, milli>(OptimizationBegin - IRGenerationStart) << "\n";
        cout << "    Optimization:      " << duration<double, milli>(ExecutionStart - OptimizationBegin) << "\n";
        cout << "    Execution:         " << duration<double>(ExecutionEnd - ExecutionStart) << endl;
    }
}

void executeString(std::unique_ptr<BrainfuckJIT>& jit, std::string s) {
    std::istringstream isstream(s);
    executeInputStream(jit, isstream);
}

void interactiveInput(std::unique_ptr<BrainfuckJIT>& jit) {
    std::cout << "Brainfuck JIT compiler" << std::endl;

    while(!std::cin.eof()) {
        std::string brainfuck_command;
        std::cout << "ready> ";
        std::getline(std::cin, brainfuck_command);
        if (brainfuck_command == "exit") break;

        executeString(jit, brainfuck_command);
    }

    if (printStateAtExit) {
        printBFStateStats();
    }
}

std::string readFile(std::string fileName) {
    try {
        std::ifstream file(fileName);
        std::stringstream buffer;
        buffer << file.rdbuf();
        return buffer.str();
    } catch (std::exception& e) {
        return "";
    }
}


int main(int argc, char **argv) {
    std::string fileName;

    int opt;
    while ((opt = getopt(argc, argv, "ps")) != -1) {
        switch (opt) {
            case 'p': showPerformanceStatistics = true; break;
            case 's': printState = true; break;
            default:
                fprintf(stderr, "Usage: %s [-ps] [file]\n"
                                " -p\t\tShow detailed performance statistics for each execution\n"
                                " -s\t\tPrint some information about the current brainfuck state after execution\n", argv[0]);
                exit(EXIT_FAILURE);
        }
    }

    if (optind < argc) {
        fileName = argv[optind];
    }

    std::unique_ptr<BrainfuckJIT> brainfuckJit = BrainfuckJIT::Create();

    std::filesystem::path p(__BASE_FILE__);
    std::string bf_code_path = p.remove_filename().string() + "test/";

    std::string hello_world = readFile(bf_code_path + "hello_world.b");
    std::string mandelbrot = readFile(bf_code_path + "mandelbrot.b");


    if (!fileName.empty()) {
        try {
            std::ifstream file(fileName);
            //std::cout << "Executing \"" << fileName << "\"" << std::endl;
            executeInputStream(brainfuckJit, file);
        } catch (std::exception& e) {
            std::cout << "Error while reading file \"" << fileName << "\": \n";
            std::cout << e.what() << std::endl;
            return -1;
        }
    } else {
        interactiveInput(brainfuckJit);
    }

    //executeString(brainfuckJit, mandelbrot);

    return 0;
}
