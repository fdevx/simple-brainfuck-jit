#include "ExprAST.h"

CellExprAST::CellExprAST(bool should_inc) : should_increment(should_inc) {

}

llvm::Value* CellExprAST::codegen(std::unique_ptr<BrainfuckIRBuilder> &birb) {
    auto& builder = birb->getBuilder();

    auto inc = builder->getInt64(should_increment ? 1 : -1);
    auto cur_val = birb->getCurrentCellValue();
    auto new_val = builder->CreateAdd(cur_val,  inc, "addtmp");

    birb->setCurrentCellValue(new_val);
    return new_val;
}

PtrExprAST::PtrExprAST(bool should_inc) : should_increment(should_inc) {

}

llvm::Value* PtrExprAST::codegen(std::unique_ptr<BrainfuckIRBuilder> &birb) {
    auto& builder = birb->getBuilder();

    llvm::Function* load_func = birb->getFunction(tags::load_function);

    auto ptr_offset = builder->getInt64(should_increment ? 1 : -1);

    auto cur_cell_ptr = birb->getCurrentCellPointer();


    auto cur_ptr_val = builder->CreateLoad(builder->getInt64Ty(), cur_cell_ptr, "cell_idx_val");
    auto new_ptr_val = builder->CreateAdd(cur_ptr_val,  ptr_offset, "addtmp");
    builder->CreateStore(new_ptr_val, cur_cell_ptr);

    std::vector<llvm::Value*> loadArgs({new_ptr_val});
    auto load_call = builder->CreateCall(load_func, loadArgs, "calltmp");

    birb->setCurrentCell(load_call);
    return load_call;
}

LoopExprAST::LoopExprAST(std::vector<std::unique_ptr<ExprAST>> Body) : Body(std::move(Body)){

}

llvm::Value *LoopExprAST::codegen(std::unique_ptr<BrainfuckIRBuilder> &birb) {
    auto& builder = birb->getBuilder();
    auto* cur_function = builder->GetInsertBlock()->getParent();

    llvm::BasicBlock* loopBody = llvm::BasicBlock::Create(*birb->getContext(), "loop", cur_function);
    llvm::BasicBlock* loopCond =  llvm::BasicBlock::Create(*birb->getContext(), "loopCond", cur_function);

    // unconditional jump - always taken by branch predictor
    builder->CreateBr(loopCond);
    builder->SetInsertPoint(loopBody);

    for (auto& expr : Body) {
        expr->codegen(birb);
    }

    builder->CreateBr(loopCond);
    builder->SetInsertPoint(loopCond);

    // jump back if cur_cell_val != 0
    auto cur_cell_val = birb->getCurrentCellValue();
    auto end_cond = builder->CreateICmpNE(cur_cell_val, builder->getInt64(0), "loopcond");

    llvm::BasicBlock* afterLoop = llvm::BasicBlock::Create(*birb->getContext(), "afterLoop", cur_function);

    // backward jump - always taken by branch predictor
    auto branch  = builder->CreateCondBr(end_cond, loopBody, afterLoop);
    builder->SetInsertPoint(afterLoop);
    return branch;
}

llvm::Value* WriteExprAST::codegen(std::unique_ptr<BrainfuckIRBuilder> &birb) {
    auto& builder = birb->getBuilder();
    llvm::Function* print_char = birb->getFunction(tags::print_character_function);

    std::vector<llvm::Value*> print_charArgs({birb->getCurrentCellValue()});
    return builder->CreateCall(print_char, print_charArgs, "calltmp");
}

llvm::Value* ReadExprAST::codegen(std::unique_ptr<BrainfuckIRBuilder> &birb) {
    auto& builder = birb->getBuilder();
    llvm::Function* get_char = birb->getFunction(tags::get_character_function);

    std::vector<llvm::Value*> get_charArgs;
    auto get_char_call = builder->CreateCall(get_char, get_charArgs, "calltmp");
    birb->setCurrentCellValue(get_char_call);
    return get_char_call;
}
