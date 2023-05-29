#pragma once
#include <stack>
#include <vector>
#include <map>
#include <string>
#include <typeinfo>
#include <exception>
#include <iostream>
#include <fstream>
#include <llvm/IR/Module.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/Type.h>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/LegacyPassManager.h>
#include <llvm/IR/Value.h>
#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/PassManager.h>
#include <llvm/IR/GlobalVariable.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/CallingConv.h>
#include <llvm/IR/IRPrintingPasses.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/ValueSymbolTable.h>
#include <llvm/IR/Verifier.h>
#include <llvm/IRReader/IRReader.h>
#include <llvm/Pass.h>
#include <llvm/Passes/PassBuilder.h>
#include <llvm/Passes/PassPlugin.h>
#include <llvm/Bitstream/BitstreamReader.h>
#include <llvm/Bitstream/BitstreamWriter.h>
#include <llvm/Support/TargetSelect.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/Support/SourceMgr.h>
#include <llvm/Support/ManagedStatic.h>
#include <llvm/Support/MemoryBuffer.h>
#include <llvm/Support/DynamicLibrary.h>
#include <llvm/Support/FileSystem.h>
#include <llvm/Support/Host.h>
#include <llvm/ExecutionEngine/ExecutionEngine.h>
#include <llvm/ExecutionEngine/MCJIT.h>
#include <llvm/ExecutionEngine/GenericValue.h>
#include <llvm/ExecutionEngine/Interpreter.h>
#include <llvm/ExecutionEngine/SectionMemoryManager.h>
#include <llvm/Target/TargetMachine.h>
#include <llvm/Target/TargetOptions.h>
#include <llvm/Transforms/InstCombine/InstCombine.h>
#include <llvm/Transforms/Scalar.h>
#include <llvm/Transforms/Scalar/GVN.h>
#include <llvm/MC/TargetRegistry.h>
#include "node.h"

using namespace llvm;

class NBlock; // define in node.h
class MyType;
extern LLVMContext MyContext; // 一个容纳各种llvm组件的“容器”，用于提供隔离特性，从而并发编译
extern llvm::IRBuilder<> CodeBuilder; // 用于向语句块中添加指令的工具类


class CodeGenContext {
  using SymbolTable = std::map<std::string, llvm::Value*>; //std::map<std::string, Symbol>;
  using SymbolType = std::map<std::string, MyType*>;

private:
  llvm::Function* currentFunc;
  llvm::Function* topFunction;
  std::stack<llvm::BasicBlock*> blocks;
  std::vector<llvm::BasicBlock*> ContinueBlockStack;
	std::vector<llvm::BasicBlock*> BreakBlockStack;
  bool isLValue;
  std::vector<SymbolTable*> SymbolTableStack;
  std::vector<SymbolType*> SymbolTypeStack;

public:
    Module* module;
    llvm::Function* mainFunction = nullptr;
    
    CodeGenContext() { 
      module = new Module("_top", MyContext); 
      currentFunc = nullptr;
      isLValue = false;
    }
    void generateIRCode(NBlock& root);
    void dumpIR(std::string FileName);
    void generateObjectCode(std::string FileName);
    GenericValue runCode();

    // used for code block generation
    BasicBlock* currentBlock() { return blocks.top(); }
    // BasicBlock* currentBlock() { return blocks.top()->block; }
    
    


    /* 创建一个新语句块，并压入栈顶 */
    void pushBlock(BasicBlock* block) {
        blocks.push(block);
    }

    void popBlock() { blocks.pop();}

    // used for function generation
    void setCurrentFunction(llvm::Function* func);
    void leaveCurrentFunction(void) ;
    llvm::Function* getCurrentFunc();

    // used for continue and break in for and while
    void EnterLoop(llvm::BasicBlock* ContinueBB, llvm::BasicBlock* BreakBB);
    void LeaveLoop(void);
    llvm::BasicBlock* GetContinueBlock(void);
    llvm::BasicBlock* GetBreakBlock(void);

    // used for symbol table maintainance
	  void PushSymbolTable(void);
	  void PopSymbolTable(void);
    llvm::Value* FindVariable(std::string Name);
    MyType* GetVariableType(std::string Name);
    bool AddVariable(std::string Name, llvm::Value* Variable, MyType* type);

    // used in NIdentifier to set as left or right value
    bool isLeftValue() { return isLValue; }
    void setLeftValue() { isLValue = true; }
    void setRightValue() { isLValue = false; }
};

