#pragma once
#include "codegen.h"

llvm::Value* Cast2I1(llvm::Value* Value);
llvm::BranchInst* TerminateBlockByBr(llvm::BasicBlock* BB);