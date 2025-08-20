#ifndef BRAINFUCKJIT_BRAINFUCKIRBUILDER_H
#define BRAINFUCKJIT_BRAINFUCKIRBUILDER_H

#include <llvm/ExecutionEngine/Orc/ThreadSafeModule.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Value.h>
#include "BrainfuckOptimizer.h"

namespace tags {
    inline constexpr auto load_function = "loadCell";

    inline constexpr auto print_character_function = "printCell";
    inline constexpr auto get_character_function = "getCell";

    inline constexpr auto current_cell_index = "current_cell_index";

    inline constexpr auto anon_func = "__anon_func";
}

class BrainfuckIRBuilder {
    llvm::AllocaInst* current_cell;
    std::unique_ptr<llvm::GlobalVariable> current_cell_index;

    std::unique_ptr<llvm::LLVMContext> llvm_ctx;
    std::unique_ptr<llvm::Module> llvm_module;
    std::unique_ptr<llvm::IRBuilder<>> llvm_builder;


    std::map<std::string, llvm::Function*> global_functions;

    BrainfuckOptimizer optimizer;
public:
    BrainfuckIRBuilder();
    ~BrainfuckIRBuilder();

    std::unique_ptr<llvm::LLVMContext>& getContext();
    std::unique_ptr<llvm::Module>& getModule();
    std::unique_ptr<llvm::IRBuilder<>>& getBuilder();

    void setDataLayout(llvm::DataLayout& DL);

    llvm::Function* getFunction(const std::string& name);

    /**
     * Creates an anonymous function and places future instructions inside
     * @return function name
     */
    std::string createAnonymousFunction();

    /**
     * Finishes an anonymous function (return from function)
     */
    void finishAnonymousFunction();

    /**
     * replaces the pointer to the active cell with the given value
     */
    void setCurrentCell(llvm::Value* new_ptr);

    /**
     * @return the value of the active cell
     */
    llvm::Value* getCurrentCellValue();

    /**
     * Stores val in the current cell
     * @param val the value to write into the active cell
     */
    void setCurrentCellValue(llvm::Value *val);

    /**
     * @return the variable that holds the current index of the active cell
     */
    llvm::Value* getCurrentCellPointer();

    void print();
};



template<typename T> constexpr llvm::Type* getLLVMType(llvm::LLVMContext& ctx) {
    return nullptr;
}

template<> constexpr llvm::Type* getLLVMType<void>(llvm::LLVMContext& ctx) {
    return llvm::Type::getVoidTy(ctx);
}

template<> constexpr llvm::Type* getLLVMType<int>(llvm::LLVMContext& ctx) {
    if (sizeof(int) == 1) return llvm::Type::getInt8Ty(ctx);
    if (sizeof(int) == 2) return llvm::Type::getInt16Ty(ctx);
    if (sizeof(int) == 4) return llvm::Type::getInt32Ty(ctx);
    if (sizeof(int) == 8) return llvm::Type::getInt64Ty(ctx);
}

template<> constexpr llvm::Type* getLLVMType<int64_t>(llvm::LLVMContext& ctx) {
    return llvm::Type::getInt64Ty(ctx);
}

template<> constexpr llvm::Type* getLLVMType<int64_t*>(llvm::LLVMContext& ctx) {
    return llvm::Type::getInt64PtrTy(ctx);
}

template<typename R_Type, typename... Arg_Types>
llvm::Function* createFunction(const std::string& name, llvm::Function::LinkageTypes Linkage, llvm::Module* module) {
    std::vector<llvm::Type*> arg_types({getLLVMType<Arg_Types>(module->getContext())...});
    auto ret_type = getLLVMType<R_Type>(module->getContext());

    auto func_type = llvm::FunctionType::get(ret_type, arg_types, false);
    return llvm::Function::Create(func_type, Linkage, name, module);
}


#endif //BRAINFUCKJIT_BRAINFUCKIRBUILDER_H
