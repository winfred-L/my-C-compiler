#include "utils.h"

llvm::Value* Cast2I1(llvm::Value* Value) {
	if (Value->getType() == CodeBuilder.getInt1Ty())
		return Value;
	else if (Value->getType()->isIntegerTy())
		return CodeBuilder.CreateICmpNE(Value, llvm::ConstantInt::get((llvm::IntegerType*)Value->getType(), 0, true));
	else if (Value->getType()->isFloatingPointTy())
		return CodeBuilder.CreateFCmpONE(Value, llvm::ConstantFP::get(Value->getType(), 0.0));
	else if (Value->getType()->isPointerTy())
		return CodeBuilder.CreateICmpNE(CodeBuilder.CreatePtrToInt(Value, CodeBuilder.getInt32Ty()), CodeBuilder.getInt32(0));
	else {
		throw std::logic_error("Cannot cast to bool type.");
		return nullptr;
	}
}

/* 一种更安全的CodeBuilder.CreateBr(llvm::BasicBlock* BB)方式。
 * 如果当前块已经有跳转语句，则无需再生成另一个跳转，否则会有异常的运行结果。
 */
llvm::BranchInst* TerminateBlockByBr(llvm::BasicBlock* BB) {
	if (!CodeBuilder.GetInsertBlock()->getTerminator())
		return CodeBuilder.CreateBr(BB);
	else
		return nullptr;
}