#include "BrainfuckOptimizer.h"

#include <llvm/Transforms/Scalar/DeadStoreElimination.h>
#include <llvm/Transforms/InstCombine/InstCombine.h>
#include <llvm/Transforms/Scalar/SimplifyCFG.h>
#include <llvm/Transforms/Utils/Mem2Reg.h>
#include <llvm/Transforms/Scalar/GVN.h>
#include <llvm/ADT/APFloat.h>
#include <llvm/ADT/STLExtras.h>
#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/Constants.h>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/PassManager.h>
#include <llvm/IR/Type.h>
#include <llvm/IR/Verifier.h>
#include <llvm/Passes/PassBuilder.h>
#include <llvm/Passes/StandardInstrumentations.h>
#include <llvm/Support/TargetSelect.h>
#include <llvm/Target/TargetMachine.h>
#include <llvm/Transforms/InstCombine/InstCombine.h>
#include <llvm/Transforms/Scalar.h>
#include <llvm/Transforms/Scalar/GVN.h>
#include <llvm/Transforms/Scalar/Reassociate.h>
#include <llvm/Transforms/Scalar/SimplifyCFG.h>
#include <llvm/Transforms/Utils.h>
#include <llvm/Transforms/Scalar/LICM.h>

BrainfuckOptimizer::BrainfuckOptimizer() :
        llvm_fpm(std::make_unique<llvm::FunctionPassManager>()),
        llvm_fam(std::make_unique<llvm::FunctionAnalysisManager>()),
        llvm_lam(std::make_unique<llvm::LoopAnalysisManager>()),
        llvm_mam(std::make_unique<llvm::ModuleAnalysisManager>()),
        llvm_cgam(std::make_unique<llvm::CGSCCAnalysisManager>()) {


    llvm_fpm->addPass(llvm::PromotePass());
    llvm_fpm->addPass(llvm::InstCombinePass());
    llvm_fpm->addPass(llvm::ReassociatePass());

    /*auto licm = llvm::LICMPass(llvm::LICMOptions());
    llvm_fpm->addPass(std::move(licm));*/

    llvm_fpm->addPass(llvm::GVNPass());
    llvm_fpm->addPass(llvm::DSEPass());
    llvm_fpm->addPass(llvm::SimplifyCFGPass());

    // llvm::createDeadCodeEliminationPass();

    // Register analysis passes used in these transform passes.
    llvm::PassBuilder PB;
    PB.registerModuleAnalyses(*llvm_mam);
    PB.registerFunctionAnalyses(*llvm_fam);
    PB.crossRegisterProxies(*llvm_lam, *llvm_fam, *llvm_cgam, *llvm_mam);
}

void BrainfuckOptimizer::run(llvm::Function *func) {
    llvm_fpm->run(*func, *llvm_fam);
}
