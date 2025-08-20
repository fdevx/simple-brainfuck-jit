#include "BrainfuckIRBuilder.h"
#include "BrainfuckRuntime.h"


BrainfuckIRBuilder::BrainfuckIRBuilder() :
        llvm_ctx(std::make_unique<llvm::LLVMContext>()),
        llvm_module(std::make_unique<llvm::Module>("Brainfuck jit", *llvm_ctx)),
        llvm_builder(std::make_unique<llvm::IRBuilder<>>(*llvm_ctx)) {
    // Create a new builder for the module.
    auto int64_type = getLLVMType<int64_t>(*llvm_ctx);
    auto zero_val = getBuilder()->getInt64(0);

    /*current_cell = std::make_unique<llvm::GlobalVariable>(
            *llvm_module, getLLVMType<int64_t*>(*llvm_ctx), false,
            llvm::GlobalValue::CommonLinkage, zero_val, "current_cell");*/

    current_cell_index = std::make_unique<llvm::GlobalVariable>(
            *llvm_module, int64_type, false,
            llvm::GlobalValue::CommonLinkage,
            zero_val,
            tags::current_cell_index);

    std::vector<llvm::Type*> store_args({int64_type, llvm::Type::getInt64PtrTy(*llvm_ctx), int64_type});

    /**
     * int64_t* loadCell(int64_t current_cell_index)
     * @param offset the amount to offset the current_cell_index in order to load the next cell value
     * @return the requested cell value
     */
    auto load_func = createFunction<int64_t*, int64_t>(tags::load_function, llvm::Function::ExternalLinkage, llvm_module.get());
    global_functions.emplace(tags::load_function, load_func);

    /**
     * Write a character to stdout.
     * int putchar(int __c)
     */
    auto put_character = createFunction<void, int64_t>(tags::print_character_function, llvm::Function::ExternalLinkage, llvm_module.get());
    global_functions.emplace(tags::print_character_function, put_character);

    /**
     * Read a character from stdin.
     * int getchar(void)
     */
    auto get_character = createFunction<int64_t>(tags::get_character_function, llvm::Function::ExternalLinkage, llvm_module.get());
    global_functions.emplace(tags::get_character_function, get_character);
}

std::unique_ptr<llvm::LLVMContext>& BrainfuckIRBuilder::getContext() {
    return llvm_ctx;
}

std::unique_ptr<llvm::Module>& BrainfuckIRBuilder::getModule() {
    return llvm_module;
}

std::unique_ptr<llvm::IRBuilder<>>& BrainfuckIRBuilder::getBuilder() {
    return llvm_builder;
}

llvm::Function* BrainfuckIRBuilder::getFunction(const std::string &name) {
    auto it = global_functions.find(name);
    return it != global_functions.end() ? it->second : nullptr;
}

void BrainfuckIRBuilder::setCurrentCell(llvm::Value* new_ptr) {
    //auto int_ptr = getLLVMType<int64_t*>(*getContext());
    auto& builder = getBuilder();

    builder->CreateStore(new_ptr, current_cell);
}

llvm::Value* BrainfuckIRBuilder::getCurrentCellValue() {
    auto int_ptr = getLLVMType<int64_t*>(*getContext());
    auto& builder = getBuilder();

    auto load_ptr = builder->CreateLoad(int_ptr, current_cell);
    return builder->CreateLoad(getBuilder()->getInt64Ty(), load_ptr, "cell_val");
}

void BrainfuckIRBuilder::setCurrentCellValue(llvm::Value *val) {
    auto int_ptr = getLLVMType<int64_t*>(*getContext());
    auto& builder = getBuilder();

    auto load_ptr = getBuilder()->CreateLoad(int_ptr, current_cell);
    builder->CreateStore(val, load_ptr);
}

llvm::Value* BrainfuckIRBuilder::getCurrentCellPointer() {
    return current_cell_index.get();
}

std::string BrainfuckIRBuilder::createAnonymousFunction() {
    auto anon_func = createFunction<int64_t>(tags::anon_func, llvm::Function::ExternalLinkage, getModule().get());
    auto bb = llvm::BasicBlock::Create(*getContext(), "entry", anon_func);
    auto& builder = getBuilder();
    builder->SetInsertPoint(bb);

    // Set current cell pointer
    builder->CreateStore(builder->getInt64(getCurrentCellIndex()), getCurrentCellPointer());

    auto int_ptr = getLLVMType<int64_t*>(*getContext());
    current_cell = builder->CreateAlloca(int_ptr);

    // Load pointer to current cell into "current_cell"
    llvm::Function* load_func = getFunction(tags::load_function);
    auto cur_ptr_val = builder->CreateLoad(builder->getInt64Ty(), getCurrentCellPointer(), "cell_idx_val");

    std::vector<llvm::Value*> loadArgs({cur_ptr_val});
    auto load_call = builder->CreateCall(load_func, loadArgs, "cell_ptr");
    setCurrentCell(load_call);

    return anon_func->getName().str();
}

void BrainfuckIRBuilder::finishAnonymousFunction() {
    auto& builder = getBuilder();
    llvm::Function* func = builder->GetInsertBlock()->getParent();

    builder->CreateRet(getCurrentCellValue());

    optimizer.run(func);
}

void BrainfuckIRBuilder::setDataLayout(llvm::DataLayout& DL) {
    llvm_module->setDataLayout(DL);
}

BrainfuckIRBuilder::~BrainfuckIRBuilder() {
    // No idea why but if current_cell is not released an exception is thrown
    // "double free or corruption (!prev)" so I guess this doesn't count as a memory leak?
    current_cell_index.release(); // NOLINT(*-unused-return-value)
}

void BrainfuckIRBuilder::print() {
    getModule()->print(llvm::outs(), nullptr);
}
