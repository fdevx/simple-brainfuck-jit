#ifndef BRAINFUCKJIT_BRAINFUCKOPTIMIZER_H
#define BRAINFUCKJIT_BRAINFUCKOPTIMIZER_H

#include <llvm/ExecutionEngine/Orc/JITTargetMachineBuilder.h>
#include <llvm/ExecutionEngine/Orc/ExecutorProcessControl.h>
#include <llvm/IR/PassManager.h>
#include <llvm/Passes/PassBuilder.h>

class BrainfuckOptimizer {
    std::unique_ptr<llvm::FunctionPassManager> llvm_fpm;
    std::unique_ptr<llvm::FunctionAnalysisManager> llvm_fam;
    std::unique_ptr<llvm::LoopAnalysisManager> llvm_lam;
    std::unique_ptr<llvm::ModuleAnalysisManager> llvm_mam;
    std::unique_ptr<llvm::CGSCCAnalysisManager> llvm_cgam;
public:
    BrainfuckOptimizer();

    void run(llvm::Function* func);
};


#endif //BRAINFUCKJIT_BRAINFUCKOPTIMIZER_H
