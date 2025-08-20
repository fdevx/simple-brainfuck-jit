#ifndef BRAINFUCKJIT_BRAINFUCKJIT_H
#define BRAINFUCKJIT_BRAINFUCKJIT_H

#include <llvm/ADT/StringRef.h>
#include <llvm/ExecutionEngine/JITSymbol.h>
#include <llvm/ExecutionEngine/Orc/CompileUtils.h>
#include <llvm/ExecutionEngine/Orc/Core.h>
#include <llvm/ExecutionEngine/Orc/ExecutionUtils.h>
#include <llvm/ExecutionEngine/Orc/ExecutorProcessControl.h>
#include <llvm/ExecutionEngine/Orc/IRCompileLayer.h>
#include <llvm/ExecutionEngine/Orc/JITTargetMachineBuilder.h>
#include <llvm/ExecutionEngine/Orc/RTDyldObjectLinkingLayer.h>
#include <llvm/ExecutionEngine/Orc/Shared/ExecutorSymbolDef.h>
#include <llvm/ExecutionEngine/SectionMemoryManager.h>
#include <llvm/IR/DataLayout.h>
#include <llvm/IR/LLVMContext.h>
#include <map>
#include "BrainfuckIRBuilder.h"

// Inspired by https://github.com/llvm/llvm-project/blob/main/llvm/examples/Kaleidoscope/include/KaleidoscopeJIT.h

// https://llvm.org/docs/tutorial/MyFirstLanguageFrontend/LangImpl04.html

class BrainfuckJIT {
private:
    std::unique_ptr<llvm::orc::ExecutionSession> ES;
    llvm::DataLayout DL;

    llvm::orc::MangleAndInterner mangle;

    llvm::orc::RTDyldObjectLinkingLayer objectLayer;
    llvm::orc::IRCompileLayer compileLayer;

    llvm::orc::JITDylib& mainJd;

    llvm::orc::ThreadSafeModule TSM;
public:
    BrainfuckJIT(std::unique_ptr<llvm::orc::ExecutionSession> ES, llvm::orc::JITTargetMachineBuilder JTMB, llvm::DataLayout DL);

    static std::unique_ptr<BrainfuckJIT> Create();
    std::unique_ptr<BrainfuckIRBuilder> createIRBuilder();

    void resetBrainfuckState();

    llvm::orc::JITDylib& getMainJITDylib();

    llvm::Error addModule(llvm::orc::ThreadSafeModule TSM, llvm::orc::ResourceTrackerSP RT = nullptr);
    llvm::Expected<llvm::orc::ExecutorSymbolDef> lookup(std::string& name);

    int64_t executeFunction(std::unique_ptr<BrainfuckIRBuilder>& birb, std::string& name);
};



#endif //BRAINFUCKJIT_BRAINFUCKJIT_H
