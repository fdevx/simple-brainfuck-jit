#include "BrainfuckJIT.h"
#include "BrainfuckRuntime.h"
#include <llvm/IR/PassManager.h>
#include <llvm/Transforms/Scalar.h>
#include <llvm/Transforms/Scalar/GVN.h>
#include <llvm/ExecutionEngine/Orc/ExecutionUtils.h>

static llvm::ExitOnError ExitOnErr;


BrainfuckJIT::BrainfuckJIT(std::unique_ptr<llvm::orc::ExecutionSession> ES,
                           llvm::orc::JITTargetMachineBuilder JTMB,
                           llvm::DataLayout DL) :
        ES(std::move(ES)),
        DL(std::move(DL)),
        mangle(*this->ES, this->DL),
        objectLayer(*this->ES, [](){
                               return std::make_unique<llvm::SectionMemoryManager>();
                           }),
        compileLayer(*this->ES, objectLayer, std::make_unique<llvm::orc::ConcurrentIRCompiler>(std::move(JTMB))),
        mainJd(this->ES->createBareJITDylib("<main>")) {
    mainJd.addGenerator(
            cantFail(llvm::orc::DynamicLibrarySearchGenerator::GetForCurrentProcess(
                    DL.getGlobalPrefix(),
                    [](llvm::orc::SymbolStringPtr symbol) {
                        bool allow = false;
                        if (*symbol == tags::load_function || //symbol == tags::store_function ||
                            *symbol == tags::get_character_function || *symbol == tags::print_character_function) {
                            allow = true;
                        }

                        /*fprintf(stderr, "Requested symbol: %s [%s]\n", (*symbol).data(),
                                allow ? "APPROVED" : "REJECTED");*/
                        return allow;
                    })));

    if (JTMB.getTargetTriple().isOSBinFormatCOFF()) {
        objectLayer.setOverrideObjectFlagsWithResponsibilityFlags(true);
        objectLayer.setAutoClaimResponsibilityForObjectSymbols(true);
    }
}

std::unique_ptr<BrainfuckJIT> BrainfuckJIT::Create() {
    static bool executeOnce = true;
    if (executeOnce) {
        executeOnce = false;

        LLVMInitializeNativeTarget();
        LLVMInitializeNativeAsmPrinter();
        LLVMInitializeNativeAsmParser();
    }

    auto EPC_exp = llvm::orc::SelfExecutorProcessControl::Create();
    if (!EPC_exp)
        throw std::runtime_error(toString(EPC_exp.takeError()));

    auto ES = std::make_unique<llvm::orc::ExecutionSession>(std::move(*EPC_exp));

    auto JTMB = llvm::orc::JITTargetMachineBuilder(ES->getExecutorProcessControl().getTargetTriple());

    auto DL_exp = JTMB.getDefaultDataLayoutForTarget();
    if (!DL_exp)
        throw std::runtime_error(toString(DL_exp.takeError()));

    return std::make_unique<BrainfuckJIT>(std::move(ES), std::move(JTMB), std::move(*DL_exp));
}

std::unique_ptr<BrainfuckIRBuilder> BrainfuckJIT::createIRBuilder() {
    std::unique_ptr<BrainfuckIRBuilder> brainfuckIR = std::make_unique<BrainfuckIRBuilder>();
    brainfuckIR->setDataLayout(this->DL);
    return brainfuckIR;
}

llvm::orc::JITDylib &BrainfuckJIT::getMainJITDylib() {
    return mainJd;
}

llvm::Error BrainfuckJIT::addModule(llvm::orc::ThreadSafeModule TSM, llvm::orc::ResourceTrackerSP RT) {
    if (!RT)
        RT = mainJd.getDefaultResourceTracker();

    return compileLayer.add(RT, std::move(TSM));
}

llvm::Expected<llvm::orc::ExecutorSymbolDef> BrainfuckJIT::lookup(std::string& name) {
    return ES->lookup({&mainJd}, mangle(name));
}

int64_t BrainfuckJIT::executeFunction(std::unique_ptr<BrainfuckIRBuilder>& birb, std::string& name) {
    auto RT = getMainJITDylib().createResourceTracker();

    auto TSM = llvm::orc::ThreadSafeModule(std::move(birb->getModule()), std::move(birb->getContext()));

    ExitOnErr(addModule(std::move(TSM), RT));

    auto ExprSymbol = ExitOnErr(lookup(name));

    auto* FP = ExprSymbol.getAddress().toPtr<int64_t (*)()>();

    std::exception_ptr eptr = nullptr;
    int64_t cell_value;
    try {
        cell_value = FP();

        auto tmp = ES->lookup({&mainJd}, mangle(tags::current_cell_index));
        auto addr = tmp.get().getAddress();
        //fprintf(stdout, "Current cell index after execution: %ld\n", *(int64_t*)(addr.getValue()));
        setCurrentCellIndex(*(int64_t*)(addr.getValue()));

        fflush(stdout);

        //fprintf(stderr, "Evaluated to %ld (%lX)\n", cell_value, cell_value);
    } catch(...) {
        eptr = std::current_exception();
    }

    ExitOnErr(RT->remove());

    if (eptr != nullptr) rethrow_exception(eptr);

    return cell_value;
}

void BrainfuckJIT::resetBrainfuckState() {
    ::resetBrainfuckState();
}
