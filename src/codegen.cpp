#include "node.h"
#include "codegen.h"
#include "parser.hpp"
#include "utils.h"

using namespace std;
LLVMContext MyContext;  // 一个容纳各种llvm组件的“容器”，用于提供隔离特性，从而并发编译
llvm::IRBuilder<> CodeBuilder(MyContext);  // 用于向语句块中添加指令的工具类
/* Compile the AST into a module */

void CodeGenContext::generateIRCode(NBlock& root)
{
#ifdef DEBUG
	std::cout << "Generating code..." << std::endl;
#endif
	
	/* Create the top level interpreter function to call as entry */
	vector<Type*> argTypes;
	// construct a FunctionType
	FunctionType* ftype = FunctionType::get(Type::getVoidTy(MyContext), makeArrayRef(argTypes), false); 
	// create a new function and attach it to a module
	// topFunction = Function::Create(ftype, GlobalValue::ExternalLinkage, "_top_f", module); 
	topFunction = Function::Create(ftype, GlobalValue::InternalLinkage, "_top_f", module); 
	
	BasicBlock* bblock = BasicBlock::Create(MyContext, "entry", topFunction, 0);
	this->setCurrentFunction(topFunction);
	// push symbol table
	pushBlock(bblock);
	PushSymbolTable(); 
	CodeBuilder.SetInsertPoint(bblock);
	// emit code 
	this->setRightValue(); 
	root.codeGen(*this);
	if(!mainFunction){
		throw std::runtime_error("No main function defined.");
		return;
	}
	// std::vector<Value*> args;
	// CodeBuilder.CreateCall(mainFunction, args);
	CodeBuilder.CreateRet(nullptr);
	// pop symbol table
	popBlock();
	PopSymbolTable(); 
	this->leaveCurrentFunction();

#ifdef DEBUG
	std::cout << "Code is generated." << std::endl;
#endif
}

void CodeGenContext::dumpIR(std::string FileName){
	// get the string of entire IR code
	std::string sout_string;
	llvm::raw_string_ostream sout(sout_string);
	module->print(sout, nullptr);
#ifdef DEBUG
	// print into screen
	std::cout << "########" << std::endl;
	std::cout << sout.str() << std::endl;
	std::cout << "########" << std::endl;
#endif
	// write into *.ll file
	std::ofstream ofs;
	ofs.open(FileName,ios::out);
	ofs << sout.str();
	ofs.close();
}

//Generate object code
void CodeGenContext::generateObjectCode(std::string FileName) {
	auto TargetTriple = llvm::sys::getDefaultTargetTriple();
	llvm::InitializeAllTargetInfos();
	llvm::InitializeAllTargets();
	llvm::InitializeAllTargetMCs();
	llvm::InitializeAllAsmParsers();
	llvm::InitializeAllAsmPrinters();
	std::string Error;
	auto Target = llvm::TargetRegistry::lookupTarget(TargetTriple, Error);
	if (!Target) {
		throw std::runtime_error(Error);
		return;
	}
	auto CPU = "generic";
	auto Features = "";
	llvm::TargetOptions opt;
	auto RM = llvm::Optional<llvm::Reloc::Model>();
	auto TargetMachine = Target->createTargetMachine(TargetTriple, CPU, Features, opt, RM);
	module->setDataLayout(TargetMachine->createDataLayout());
	module->setTargetTriple(TargetTriple);
	std::error_code EC;
	llvm::raw_fd_ostream Dest(FileName, EC, llvm::sys::fs::OF_None);
	if (EC) {
		throw std::runtime_error("Could not open file: " + EC.message());
		return;
	}
	auto FileType = llvm::CGFT_ObjectFile;
	llvm::legacy::PassManager PM;
	if (TargetMachine->addPassesToEmitFile(PM, Dest, nullptr, FileType)) {
		throw std::runtime_error("TargetMachine can't emit a file of this type");
		return;
	}
	PM.run(*module);
	Dest.flush();
}

// run single file
GenericValue CodeGenContext::runCode() {
	std::cout << "Running code..." << std::endl;
	ExecutionEngine* ee = EngineBuilder( unique_ptr<Module>(module) ).create();
	ee->finalizeObject();
	vector<GenericValue> noargs;
	GenericValue v = ee->runFunction(mainFunction, noargs);
	std::cout << "Code was run." << std::endl;
	return v;
}

//Set current function
void CodeGenContext::setCurrentFunction(llvm::Function* Func) {
	this->currentFunc = Func;
}

//Remove current function
void CodeGenContext::leaveCurrentFunction(void) {
	this->currentFunc = nullptr;
}

//Get the current function
llvm::Function* CodeGenContext::getCurrentFunc(){
  return this->currentFunc;
}

/* used for continue and break in for and while */
void CodeGenContext::EnterLoop(llvm::BasicBlock* ContinueBB, llvm::BasicBlock* BreakBB) {
#ifdef DEBUG
	std::cout << "enter loop" << std::endl;
#endif
	this->ContinueBlockStack.push_back(ContinueBB);
	this->BreakBlockStack.push_back(BreakBB);
}


void CodeGenContext::LeaveLoop(void) {
#ifdef DEBUG
	std::cout << "leave loop" << std::endl;
#endif
	if (this->ContinueBlockStack.size() == 0 || this->BreakBlockStack.size() == 0) return;
	this->ContinueBlockStack.pop_back();
	this->BreakBlockStack.pop_back();
}


llvm::BasicBlock* CodeGenContext::GetContinueBlock(void) {
	if (this->ContinueBlockStack.size())
		return this->ContinueBlockStack.back();
	else
		return nullptr;
}


llvm::BasicBlock* CodeGenContext::GetBreakBlock(void) {
	if (this->BreakBlockStack.size())
		return this->BreakBlockStack.back();
	else
		return nullptr;
}

/* used for symbol table maintainance */
void CodeGenContext::PushSymbolTable(void){
	SymbolTableStack.push_back(new SymbolTable);
	SymbolTypeStack.push_back(new SymbolType);
}


void CodeGenContext::PopSymbolTable(void){
	if (SymbolTableStack.size() == 0) return;
	delete SymbolTableStack.back();
	SymbolTableStack.pop_back();
	if (SymbolTypeStack.size() == 0) return;
	delete SymbolTypeStack.back();
	SymbolTypeStack.pop_back();
}


llvm::Value* CodeGenContext::FindVariable(std::string Name){
	if (SymbolTableStack.size() == 0) return nullptr;
	for (auto it = SymbolTableStack.end() - 1; it >= SymbolTableStack.begin(); it--) {
		auto PairIter = (**it).find(Name);
		if (PairIter != (**it).end()){
#ifdef DEBUG
			std::cout << "Finding variable: " << Name << std::endl;
			// PairIter->second->getType()->dump();//debug
#endif
			return PairIter->second;
		}
	}
	return nullptr;
}


MyType* CodeGenContext::GetVariableType(std::string Name){
	if (SymbolTypeStack.size() == 0) return nullptr;
	for (auto it = SymbolTypeStack.end() - 1; it >= SymbolTypeStack.begin(); it--) {
		auto PairIter = (**it).find(Name);
		if (PairIter != (**it).end())
			return PairIter->second;
	}
	return nullptr;
}


bool CodeGenContext::AddVariable(std::string Name, llvm::Value* Variable, MyType* type){
#ifdef DEBUG
	std::cout << "Adding variable: " << Name << std::endl;
#endif
	if (SymbolTableStack.size() == 0) return false;
	auto& TopTable = *(SymbolTableStack.back());
	auto PairIter = TopTable.find(Name);
	if (PairIter != TopTable.end())
		return false;
	TopTable[Name] = Variable;
	auto& TopType = *(SymbolTypeStack.back());
	auto PairIter2 = TopType.find(Name);
	if (PairIter2 != TopType.end())
		return false;
	TopType[Name] = type;
	return true;
}


/* 错误处理 */
Value* LogErrorV(const char* s) {
	std::cerr << "Error: " << s << std::endl;
	return nullptr;
}


/* -- Code Generation -- */

//统一所有字面量

Value* NLiteral::codeGen(CodeGenContext& context){
// #ifdef DEBUG
//     std::cout << "Creating " << this->type << ":" << character_value << integer_value << real_value << std::endl;
// #endif

    switch(this->typeID){
      case _Char: return ConstantInt::get(Type::getInt8Ty(MyContext), character_value, true);
      case _Int: return ConstantInt::get(Type::getInt32Ty(MyContext), integer_value, true);
      case _Double: return ConstantFP::get(Type::getDoubleTy(MyContext), real_value);
      case _Bool: return ConstantInt::get(Type::getInt1Ty(MyContext), bool_value, true);
      default: {
        std::cout << "Unknown type." << std::endl; return nullptr;
      }
    }
    // if(strcmp(this->type.c_str() , "char") == 0) return ConstantInt::get(Type::getInt8Ty(MyContext), character_value, true);
    // else if(strcmp(this->type.c_str() , "int") == 0){ return ConstantInt::get(Type::getInt32Ty(MyContext), integer_value, true);}
    // else if(strcmp(this->type.c_str() , "double") == 0) return ConstantFP::get(Type::getDoubleTy(MyContext), real_value);
    // else if(strcmp(this->type.c_str() , "bool") == 0) return ConstantInt::get(Type::getInt1Ty(MyContext), bool_value, true);
    // else {std::cout << "Unknown type." << std::endl; return nullptr;}

}

Value* NIdentifier::codeGen(CodeGenContext& context)
{
#ifdef DEBUG
	std::cout << "Creating identifier reference: " << name << std::endl;
#endif
	llvm::Value* retV = context.FindVariable(name);
	if (!retV) {
		throw std::runtime_error("undeclared or unreachable variable " + name);
		return NULL;
	}

  if(context.isLeftValue()){
    return retV;
  }
  else{
    MyType* type = context.GetVariableType(name);
    return CodeBuilder.CreateLoad(type->getLLVMType(), retV); // 返回的不是指针类型
  }

	
}


Value* NUnaryOperator::codeGen(CodeGenContext& context)
{
#ifdef DEBUG
	std::cout << "Creating unary operation " << op << std::endl;
#endif
	context.setRightValue();
	Value* V = exp.codeGen(context);
	if(!V) return nullptr;

	Value* zero = CodeBuilder.getInt32(0);
	Value* boolV = Cast2I1(V);

	switch(op){
		case TMINUS: return CodeBuilder.CreateSub(zero, V, "negative");
		case TNOT: return CodeBuilder.CreateICmpEQ(boolV, CodeBuilder.getInt1(false));
		default: 
			throw std::runtime_error("invalid unary operator");
			return nullptr;
	}
}

Value* NBinaryOperator::codeGen(CodeGenContext& context)
{
#ifdef DEBUG
	std::cout << "Creating binary operation " << op << std::endl;
#endif

	context.setRightValue();
	Value* L = lhs.codeGen(context);
	context.setRightValue();
	Value* R = rhs.codeGen(context);
	if(!L || !R) return nullptr;

#ifdef DEBUG
	L->getType()->dump();
	R->getType()->dump();
#endif

	if(L->getType()->isVoidTy() || R->getType()->isVoidTy()){
		throw std::runtime_error("can't do operation on void");
		return nullptr;
	}

	bool flag = L->getType()->isDoubleTy() || R->getType()->isDoubleTy();
	if(flag){
		if(L->getType()->isIntegerTy()){
			L = CodeBuilder.CreateSIToFP(L, CodeBuilder.getDoubleTy());
		}
		else if(R->getType()->isIntegerTy()){
			R = CodeBuilder.CreateSIToFP(R, CodeBuilder.getDoubleTy());
		}
	}
	else if(L->getType()->isIntegerTy() && R->getType()->isIntegerTy()){
		llvm::IntegerType* Ltype = (llvm::IntegerType*)(L->getType());
		llvm::IntegerType* Rtype = (llvm::IntegerType*)(R->getType());
		if(Ltype->getBitWidth() > Rtype->getBitWidth()){
			R = CodeBuilder.CreateSExt(R, L->getType());
		}
		else if(Ltype->getBitWidth() < Rtype->getBitWidth()){
			L = CodeBuilder.CreateSExt(L, R->getType());
		}
	}
			

#ifdef DEBUG
	L->getType()->dump();
	R->getType()->dump();
#endif

	switch(op){
		case TPLUS: return flag? CodeBuilder.CreateFAdd(L, R, "addtmpf") : CodeBuilder.CreateAdd(L, R, "addtmp");
		case TMINUS: return flag? CodeBuilder.CreateFSub(L, R, "subtmpf") : CodeBuilder.CreateSub(L, R, "subtmp");
		case TMUL: return flag? CodeBuilder.CreateFMul(L, R, "multmpf") : CodeBuilder.CreateMul(L, R, "multmp");
		case TDIV: 
			if(flag){
				throw std::runtime_error("can't do div on double");
				return nullptr;
			}else{
				return CodeBuilder.CreateSDiv(L, R, "divtmp"); // 只有整数除法
			}
		case TMOD:
			if(flag){
				throw std::runtime_error("can't do mod on double");
				return nullptr;
			}else{
				return CodeBuilder.CreateSRem(L, R, "modtmp"); // 只有整数除法
			}
		case TAND:
			L = Cast2I1(L); R = Cast2I1(R);
			return CodeBuilder.CreateLogicalAnd(L, R, "andtmp");
		case TOR:
			L = Cast2I1(L); R = Cast2I1(R);
			return CodeBuilder.CreateLogicalOr(L, R, "ortmp");

		case TCEQ: return flag? CodeBuilder.CreateFCmpOEQ(L, R, "feqtmp") : CodeBuilder.CreateICmpEQ(L, R, "eqtmp");
		case TCNE: return flag? CodeBuilder.CreateFCmpONE(L, R, "fnetmp") : CodeBuilder.CreateICmpNE(L, R, "netmp");
		case TCLT: return flag? CodeBuilder.CreateFCmpOLT(L, R, "flttmp") : CodeBuilder.CreateICmpSLT(L, R, "slttmp");
		case TCLE: return flag? CodeBuilder.CreateFCmpOLE(L, R, "fletmp") : CodeBuilder.CreateICmpSLE(L, R, "sletmp");
		case TCGT: return flag? CodeBuilder.CreateFCmpOGT(L, R, "fgttmp") : CodeBuilder.CreateICmpSGT(L, R, "sgttmp");
		case TCGE: return flag? CodeBuilder.CreateFCmpOGE(L, R, "fgetmp") : CodeBuilder.CreateICmpSGE(L, R, "sgetmp");

		default:
			throw std::runtime_error("invalid binary operator");
			return nullptr;
	}

}

Value* NAssignment::codeGen(CodeGenContext& context)
{
#ifdef DEBUG
	std::cout << "Creating assignment" << std::endl;
#endif
	context.setLeftValue();
	Value* L = lhs.codeGen(context);//generate a left value
	context.setRightValue();
	Value* R = rhs.codeGen(context);
	
	if(L->getType()->isVoidTy() || R->getType()->isVoidTy()){
		throw std::runtime_error("can't do assignment of void");
		return nullptr;
	}
	// do type transform
	Type* lt = L->getType()->getPointerElementType();
	Type* rt = R->getType();
	if(lt->isDoubleTy() && rt->isIntegerTy()){
		R = CodeBuilder.CreateSIToFP(R, CodeBuilder.getDoubleTy());
	}
	else if(lt->isIntegerTy() && rt->isDoubleTy()){
		R = CodeBuilder.CreateFPToSI(R, L->getType());
	}
	else if(lt->isIntegerTy() && rt->isIntegerTy()){
		R = CodeBuilder.CreateSExtOrTrunc(R, lt);
	}
	// assignment
	return CodeBuilder.CreateStore(R , L);
}

Value* NBlock::codeGen(CodeGenContext& context)
{
#ifdef DEBUG
	std::cout << "Creating block" << std::endl;
#endif
	StatementList::const_iterator it;
	Value* last = nullptr;
	context.PushSymbolTable(); ////
	for (it = statements.begin(); it != statements.end(); it++) {
		context.setRightValue();
		last = (**it).codeGen(context);
	}
	context.PopSymbolTable(); ////
	return last;
}

Value* NExpressionStatement::codeGen(CodeGenContext& context)
{
#ifdef DEBUG
	std::cout << "Generating code for " << typeid(expression).name() << std::endl;
#endif
	context.setRightValue();
	return expression.codeGen(context);
}

Value* NReturnStatement::codeGen(CodeGenContext& context)
{
	if(expression){
#ifdef DEBUG
		std::cout << "Generating return code for " << typeid(*expression).name() << std::endl;
#endif
		context.setRightValue();
		Value* returnValue = expression->codeGen(context);
		CodeBuilder.CreateRet(returnValue);
		return returnValue;
	}
	else{
#ifdef DEBUG
		std::cout << "Generating void return." << std::endl;
#endif
		CodeBuilder.CreateRet(nullptr);
		return nullptr;
	}
}


Value* NVariableDeclaration::codeGen(CodeGenContext& context)
{
#ifdef DEBUG
	std::cout << "Creating variable declaration " << type << " " << id.name << std::endl;
#endif
	AllocaInst* alloc;  
  alloc = CodeBuilder.CreateAlloca(type->getLLVMType(), 0, id.name.c_str());
  context.AddVariable( this->id.name, alloc, this->type );

	if (assignmentExpr != NULL) {
		NAssignment assn(id, *assignmentExpr);
    context.setRightValue();
		assn.codeGen(context);
	}
	return alloc;
}


Value* NExternDeclaration::codeGen(CodeGenContext& context)
{
#ifdef DEBUG
    std::cout << "extern" << std::endl;
#endif
	vector<Type*> argTypes;
    VariableList::const_iterator it;
    for (it = arguments.begin(); it != arguments.end(); it++) {
        argTypes.push_back((**it).type->getLLVMType());//only support build-in type
    }
	FunctionType* ftype = FunctionType::get(type->getLLVMType(), makeArrayRef(argTypes), false);
    Function* function = Function::Create(ftype, GlobalValue::ExternalLinkage, id.name.c_str(), context.module);
	return function;
}

Value* NFunctionDeclaration::codeGen(CodeGenContext& context)
{
#ifdef DEBUG
	std::cout << "Creating function: " << id.name << std::endl;
#endif
	vector<Type*> argTypes;
	VariableList::const_iterator it;
	for (it = arguments.begin(); it != arguments.end(); it++) {
    if( (**it).type->isArrayTy() ){

    }
    else if( (**it).type ->isPointerTy() ){

    }
		else argTypes.push_back((**it).type->getLLVMType());//array and pointer

	}
	FunctionType* FuncType = FunctionType::get(type->getLLVMType(), makeArrayRef(argTypes), false);
	Function* function = Function::Create(FuncType, GlobalValue::ExternalLinkage, id.name.c_str(), context.module);
	// Function* function = Function::Create(FuncType, GlobalValue::InternalLinkage, id.name.c_str(), context.module);
	BasicBlock* bblock = BasicBlock::Create(MyContext, "entry", function, 0);

	if(id.name == "main") context.mainFunction = function;

	context.pushBlock(bblock);
	context.PushSymbolTable(); ////
	CodeBuilder.SetInsertPoint(context.currentBlock());
	context.setCurrentFunction(function);
	Function::arg_iterator argsValues = function->arg_begin();
	Value* argumentValue;

	for (it = arguments.begin(); it != arguments.end(); it++) {
    //不需要设置左值，因为VariableDecl没有用到，一定返回左值
		(**it).codeGen(context);//给函数参数分配内存
		
		argumentValue = &*argsValues++;//Argument是Value的子类
		argumentValue->setName((*it)->id.name.c_str());
		CodeBuilder.CreateStore(argumentValue, context.FindVariable((*it)->id.name));
	}
	context.setRightValue();
	block->codeGen(context);
	

	context.popBlock();
	context.PopSymbolTable(); ////
	CodeBuilder.SetInsertPoint(context.currentBlock());
	context.leaveCurrentFunction();
	return function;
}


Value* NIfStatement::codeGen(CodeGenContext& context)
{
#ifdef DEBUG
	std::cout << "Creating if statement" << std::endl;
#endif
	context.setRightValue();//设置成右值
	Value* CondV = condition.codeGen(context);
	if(!CondV) return nullptr;

	// condition transformed into boolean
	if (!(CondV = Cast2I1(CondV))) {
		throw std::logic_error("The condition value of if-statement must be either an integer, or a floating-point number, or a pointer.");
		return nullptr;
	}

	// get the embedding function of current code block
	Function* currentFunc = context.getCurrentFunc();
	BasicBlock* ThenBB = BasicBlock::Create(MyContext, "then");
	BasicBlock* ElseBB = BasicBlock::Create(MyContext, "else");
	BasicBlock* MergeBB = BasicBlock::Create(MyContext, "merge");
	Value* branch = CodeBuilder.CreateCondBr(CondV, ThenBB, ElseBB);

	// emit then part
	currentFunc->getBasicBlockList().push_back(ThenBB);
	CodeBuilder.SetInsertPoint(ThenBB);
	if(thenStmt){
    context.setRightValue();
		thenStmt->codeGen(context);
	}
	TerminateBlockByBr(MergeBB);

	// emit else part 
	currentFunc->getBasicBlockList().push_back(ElseBB);
  	CodeBuilder.SetInsertPoint(ElseBB);  
	if(elseStmt){
    context.setRightValue();
		elseStmt->codeGen(context);
	}
	TerminateBlockByBr(MergeBB);

	// finish if
  	currentFunc->getBasicBlockList().push_back(MergeBB);
	CodeBuilder.SetInsertPoint(MergeBB);
	
	return branch;
}

Value* NForStatement::codeGen(CodeGenContext& context)
{
#ifdef DEBUG
	std::cout << "Creating for statement" << std::endl;
#endif
	Function* currentFunc = context.getCurrentFunc();
	llvm::BasicBlock* ForCondBB = llvm::BasicBlock::Create(MyContext, "ForCond");
	llvm::BasicBlock* ForLoopBB = llvm::BasicBlock::Create(MyContext, "ForLoop");
	llvm::BasicBlock* ForTailBB = llvm::BasicBlock::Create(MyContext, "ForTail");
	llvm::BasicBlock* ForEndBB = llvm::BasicBlock::Create(MyContext, "ForEnd");

	// emit initial part
	if (initStmt) {
		context.PushSymbolTable();
		context.setRightValue();
		initStmt->codeGen(context);
	}
	TerminateBlockByBr(ForCondBB);

	// emit condition part
	currentFunc->getBasicBlockList().push_back(ForCondBB);
	CodeBuilder.SetInsertPoint(ForCondBB);
	if (condition) {
    context.setRightValue();
		llvm::Value* CondV = condition->codeGen(context);
		// condition transformed into boolean
		if (!(CondV = Cast2I1(CondV))) {
			throw std::logic_error("The condition value of for-statement must be either an integer, or a floating-point number, or a pointer.");
			return nullptr;
		}
		CodeBuilder.CreateCondBr(CondV, ForLoopBB, ForEndBB);
	}
	else {
		TerminateBlockByBr(ForLoopBB);
	}

	// emit loop body
	currentFunc->getBasicBlockList().push_back(ForLoopBB);
	CodeBuilder.SetInsertPoint(ForLoopBB);
	if (loopBody) {
		context.EnterLoop(ForTailBB, ForEndBB); // used for continue and break
		context.PushSymbolTable();
		context.setRightValue();
		loopBody->codeGen(context);
		context.PopSymbolTable();
		context.LeaveLoop(); // used for continue and break
	}
	TerminateBlockByBr(ForTailBB);

	// emit tail part
	currentFunc->getBasicBlockList().push_back(ForTailBB);
	CodeBuilder.SetInsertPoint(ForTailBB);
	if (tailStmt)
	{
		context.setRightValue();
		tailStmt->codeGen(context);
	}
	TerminateBlockByBr(ForCondBB);

	// finish for
	currentFunc->getBasicBlockList().push_back(ForEndBB);
	CodeBuilder.SetInsertPoint(ForEndBB);
	if (initStmt) {
		context.PopSymbolTable();
	}

	return nullptr;
}


Value* NWhileStatement::codeGen(CodeGenContext& context)
{
#ifdef DEBUG
	std::cout << "Creating while statement" << std::endl;
#endif
	Function* currentFunc = context.getCurrentFunc();
	llvm::BasicBlock* WhileCondBB = llvm::BasicBlock::Create(MyContext, "WhileCond");
	llvm::BasicBlock* WhileLoopBB = llvm::BasicBlock::Create(MyContext, "WhileLoop");
	llvm::BasicBlock* WhileEndBB = llvm::BasicBlock::Create(MyContext, "WhileEnd");
	
	TerminateBlockByBr(WhileCondBB);

	// emit condition part
	currentFunc->getBasicBlockList().push_back(WhileCondBB);
	CodeBuilder.SetInsertPoint(WhileCondBB);
	context.setRightValue();
	llvm::Value* CondV = condition->codeGen(context);
	if (!(CondV = Cast2I1(CondV))) {
		throw std::logic_error("The condition value of while-statement must be either an integer, or a floating-point number, or a pointer.");
		return nullptr;
	}
	CodeBuilder.CreateCondBr(CondV, WhileLoopBB, WhileEndBB);

	// emit loop body
	currentFunc->getBasicBlockList().push_back(WhileLoopBB);
	CodeBuilder.SetInsertPoint(WhileLoopBB);
	if (loopBody) {
		context.EnterLoop(WhileCondBB, WhileEndBB); // used for continue and break
		context.PushSymbolTable();
		context.setRightValue();
		loopBody->codeGen(context);
		context.PopSymbolTable();
		context.LeaveLoop(); // used for continue and break
	}
	TerminateBlockByBr(WhileCondBB);

	// finish while
	currentFunc->getBasicBlockList().push_back(WhileEndBB);
	CodeBuilder.SetInsertPoint(WhileEndBB);

	return nullptr;
}


Value* NContinueStatement::codeGen(CodeGenContext& context)
{
#ifdef DEBUG
	std::cout << "continue" << std::endl;
#endif
	llvm::BasicBlock* ContinueTarget = context.GetContinueBlock();
	if (ContinueTarget)
		CodeBuilder.CreateBr(ContinueTarget);
	else
		throw std::logic_error("Continue statement should only be used in loops or switch statements.");
	return nullptr;
}



Value* NBreakStatement::codeGen(CodeGenContext& context)
{
#ifdef DEBUG
	std::cout << "break" << std::endl;
#endif
	llvm::BasicBlock* BreakTarget = context.GetBreakBlock();
	if (BreakTarget)
		CodeBuilder.CreateBr(BreakTarget);
	else
		throw std::logic_error("Break statement should only be used in loops or switch statements.");
	return nullptr;
}


Value* NMethodCall::codeGen(CodeGenContext& context)
{
#ifdef DEBUG
	std::cout << "Creating method call: " << id.name << std::endl;
#endif
	Function* function = context.module->getFunction(id.name.c_str());
	if (function == nullptr) {
		throw std::runtime_error("no such function" + id.name);
		return nullptr;
	}
	std::vector<Value*> args;
	ExpressionList::const_iterator it;
	//arguments are R value
	for (it = arguments.begin(); it != arguments.end(); it++) {
    context.setRightValue();
		args.push_back((**it).codeGen(context));
	}
	CallInst* call = CodeBuilder.CreateCall(function, args);  
	return call;
}


Value* NArrayIndex::codeGen(CodeGenContext& context)
{
#ifdef DEBUG
	std::cout << "Creating array" << std::endl;
#endif
  bool currentLValue = context.isLeftValue();
  //Get the pointer
  context.setLeftValue();
  Value* arrayPtr = this->array.codeGen(context);
  if (!arrayPtr->getType()->isPointerTy()) {
    throw std::logic_error("Subscript operator \"[]\" must be applied to pointers or arrays.");
    return nullptr;
  }
  

  Type* baseType = context.GetVariableType(this->array.name)->getBaseType(); 

  arrayPtr = CodeBuilder.CreateBitCast(arrayPtr, baseType->getPointerTo());//把指针的类型从数组转换成元素的指针
  

  //Value * Type* 目标类型
  //Get the index value
  context.setRightValue();
  Value* subscript = this->index.codeGen(context);
  if (!(subscript->getType()->isIntegerTy())) {//变量转成constantInt
    throw std::logic_error("Subscription should be an integer.");
    return nullptr;
  }
  //Do the pointer add
  Value* sumtmp =  CodeBuilder.CreateGEP(arrayPtr->getType()->getPointerElementType(), arrayPtr, subscript);
	//test if the value is a left value
  if(currentLValue) return sumtmp;
  else return CodeBuilder.CreateLoad(sumtmp->getType()->getPointerElementType(), sumtmp);

}


Value* NAddressOf::codeGen(CodeGenContext& context)
{
  context.setLeftValue();
  return operand.codeGen(context); //做左值
}


Value* NDereference::codeGen(CodeGenContext& context)
{
  Type* baseType = context.GetVariableType(ptr.name)->getBaseType();
  if(context.isLeftValue())
  {
    context.setRightValue();
    return ptr.codeGen(context);//指针
  }
  else
  {
    return CodeBuilder.CreateLoad( baseType , ptr.codeGen(context) );
  }
}