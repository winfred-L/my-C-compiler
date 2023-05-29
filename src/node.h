#pragma once
#include <iostream>
#include <vector>
#include <string>
#include "codegen.h"
class CodeGenContext;
class NStatement;
class NExpression;
class NVariableDeclaration;
using namespace llvm;
typedef std::vector<NStatement*> StatementList;
typedef std::vector<NExpression*> ExpressionList;
typedef std::vector<NVariableDeclaration*> VariableList;
extern llvm::IRBuilder<> CodeBuilder;
extern llvm::LLVMContext MyContext;

// basic data type
enum TypeID {
	_Error,
	_Bool,
	_Char,
	_Int,
	_Double,
  _Array,
  _Pointer,
	_Void
};

class Node {
public:
	virtual ~Node() {}
	virtual llvm::Value* codeGen(CodeGenContext& context) { return nullptr; }
};

class NExpression : public Node {
};

class NStatement : public Node {
};

class NLiteral : public NExpression {
public:
  int typeID;
  char character_value;
  int integer_value;
  double real_value;
  bool bool_value;
  NLiteral(char character) :
    typeID(_Char), character_value(character), integer_value(0), real_value(0.0), bool_value(false) {}
  NLiteral(int integer) :
    typeID(_Int), character_value('\0'), integer_value(integer), real_value(0.0), bool_value(false) {}
  NLiteral(double real) :
    typeID(_Double), character_value('\0'), integer_value(0), real_value(real), bool_value(false) {}
  NLiteral(bool bool_value) :
    typeID(_Bool), character_value('\0'), integer_value(0), real_value(0.0), bool_value(bool_value) {}  
  ~NLiteral(void) {}
  virtual llvm::Value* codeGen(CodeGenContext& context);
};


class NIdentifier : public NExpression {
public:
	std::string name;
	NIdentifier(const std::string& name) : name(name) { }
	virtual llvm::Value* codeGen(CodeGenContext& context);
};

class MyType {
public:
  MyType(){}
  virtual ~MyType(){}
  virtual Type* getLLVMType() = 0;
  virtual bool isPointerTy() = 0;
  virtual bool isArrayTy() = 0;
  virtual Type* getBaseType() = 0;
};

class BuiltInType : public MyType{
public:
  int typeID;
  BuiltInType(int typeID) : typeID(typeID) {}
  virtual Type* getLLVMType(){
    switch(typeID){
		case _Bool: return Type::getInt1Ty(MyContext);
		case _Char: return Type::getInt8Ty(MyContext);
		case _Int: return Type::getInt32Ty(MyContext);
		case _Double: return Type::getDoubleTy(MyContext);
		case _Void: return Type::getVoidTy(MyContext);
		default: return nullptr;
	}
  }
  virtual Type* getBaseType(){
    return this->getLLVMType();
  }

  virtual bool isPointerTy(){
    return false;
  }
  virtual bool isArrayTy(){
    return false;
  }
};

class NArrayType : public MyType {
public:
  int baseType; 
  size_t length;
  NArrayType( int baseType , int length ) : baseType(baseType), length(length) { }
  virtual Type* getLLVMType(){
    MyType* base_ty = new BuiltInType(baseType);
    Type* baseLLVMType = base_ty->getLLVMType();
    delete base_ty;
    return llvm::ArrayType::get( baseLLVMType , this->length );
  }

  virtual bool isPointerTy(){
    return false;
  }
  virtual bool isArrayTy(){
    return true;
  }
  virtual Type* getBaseType(){
    MyType* base_ty = new BuiltInType(baseType);
    Type* baseLLVMType = base_ty->getLLVMType();
    delete base_ty;
    return baseLLVMType;
  }

};

class NArrayIndex : public NExpression {
public:
  NIdentifier& array;
  NExpression& index;
  //NArrayType *arrType; 
  //从符号表中找到arrayType 得到baseType 做bitcast 指向数组的指针换成指向具体元素的指针，才能当左值
  NArrayIndex(NIdentifier& array, NExpression& index ) : 
    array(array), index(index) {}
  virtual llvm::Value* codeGen(CodeGenContext& context);
  

};


class NPointerType : public MyType {
public:
  int baseType;
  NPointerType( int baseType ) : baseType(baseType) {}
  virtual Type* getLLVMType(){
    MyType* base_ty = new BuiltInType(baseType);
    Type* baseLLVMType = base_ty->getLLVMType();
    delete base_ty;
    return llvm::PointerType::get( baseLLVMType , 0U );
  }

  virtual bool isPointerTy(){
    return true;
  }
  virtual bool isArrayTy(){
    return false;
  }
  virtual Type* getBaseType(){
    MyType* base_ty = new BuiltInType(baseType);//only support bulit in type
    Type* baseLLVMType = base_ty->getLLVMType();
    delete base_ty;
    return baseLLVMType;
  }

};


class NAddressOf : public NExpression {
public:
  NIdentifier& operand; // maybe a expr will do better
  NAddressOf( NIdentifier& expr) : operand(expr) {}
  virtual llvm::Value* codeGen(CodeGenContext& context);
};


class NDereference : public NExpression {
public:
  NIdentifier& ptr;
  NDereference( NIdentifier& ptr ) : ptr(ptr) {}
  virtual llvm::Value* codeGen(CodeGenContext& context);
};

class NMethodCall : public NExpression {
public:
	const NIdentifier& id;
	ExpressionList arguments;
	NMethodCall(const NIdentifier& id, ExpressionList& arguments) :
		id(id), arguments(arguments) { }
	NMethodCall(const NIdentifier& id) : id(id) { }
	virtual llvm::Value* codeGen(CodeGenContext& context);
};

class NUnaryOperator : public NExpression {
public:
	int op;
	NExpression& exp;
	NUnaryOperator(int op, NExpression& exp) :
		op(op), exp(exp) { }
	virtual llvm::Value* codeGen(CodeGenContext& context);
};

class NBinaryOperator : public NExpression {
public:
	int op;
	NExpression& lhs;
	NExpression& rhs;
	NBinaryOperator(NExpression& lhs, int op, NExpression& rhs) :
		lhs(lhs), rhs(rhs), op(op) { }
	virtual llvm::Value* codeGen(CodeGenContext& context);
};

class NAssignment : public NExpression {
public:
	NExpression& lhs;
	NExpression& rhs;
	NAssignment(NExpression& lhs, NExpression& rhs) : 
		lhs(lhs), rhs(rhs) { }
	virtual llvm::Value* codeGen(CodeGenContext& context);
};

class NBlock : public NStatement {
public:
	StatementList statements;
	NBlock() { }
	virtual llvm::Value* codeGen(CodeGenContext& context);
};

class NExpressionStatement : public NStatement {
public:
	NExpression& expression;
	NExpressionStatement(NExpression& expression) : 
		expression(expression) { }
	virtual llvm::Value* codeGen(CodeGenContext& context );
};

class NReturnStatement : public NStatement {
public:
	NExpression* expression; // can be nullptr
	NReturnStatement(NExpression* expression = nullptr) : 
		expression(expression) { }
	virtual llvm::Value* codeGen(CodeGenContext& context);
};

class NVariableDeclaration : public NStatement {
public:
	// int type;
	NIdentifier& id;
	NExpression* assignmentExpr;
  // NArrayType* array;
  // NPointerType* pointer;
  MyType* type;
	NVariableDeclaration(MyType* type, NIdentifier& id, NExpression *assignmentExpr = nullptr) :
		type(type), id(id), assignmentExpr(assignmentExpr) { }
  // NVariableDeclaration(NIdentifier& id, NArrayType* array) :
  //   type(_Array), id(id), array(array) { assignmentExpr = nullptr; pointer = nullptr; }
  // NVariableDeclaration(NIdentifier& id, NPointerType* pointer) :
  //   type(_Pointer), id(id), pointer(pointer) { assignmentExpr = nullptr; array = nullptr; }

  virtual llvm::Value* codeGen(CodeGenContext& context);
};

class NExternDeclaration : public NStatement {
public:
    MyType* type;
    const NIdentifier& id;
    VariableList arguments;
    NExternDeclaration(MyType* type, const NIdentifier& id, const VariableList& arguments) :
        type(type), id(id), arguments(arguments) {}
    virtual llvm::Value* codeGen(CodeGenContext& context);
};

class NFunctionDeclaration : public NStatement {
public:
	MyType* type;
	const NIdentifier& id;
	VariableList arguments;
	NBlock* block; // body
	NFunctionDeclaration(MyType* type, const NIdentifier& id, const VariableList& arguments, NBlock* block) :
		type(type), id(id), arguments(arguments), block(block) { }
	virtual llvm::Value* codeGen(CodeGenContext& context);
};

class NIfStatement : public NStatement {
public:
	NExpression& condition;
	NStatement* thenStmt;
	NStatement* elseStmt; // can be nullptr
	NIfStatement(NExpression& condition, NStatement* thenStmt, NStatement* elseStmt = nullptr) : 
		condition(condition), thenStmt(thenStmt), elseStmt(elseStmt) { }
	virtual llvm::Value* codeGen(CodeGenContext& context);
};

// for(initial ; condition ; tail) {loopbody}
class NForStatement : public NStatement {
public:
	NStatement* initStmt;
	NExpression* condition;
	NStatement* tailStmt;
	NStatement* loopBody;
	NForStatement(NStatement* initStmt, NExpression* condition, NStatement* tailStmt, NStatement* loopBody) :
		initStmt(initStmt), condition(condition), tailStmt(tailStmt), loopBody(loopBody) {}
	virtual llvm::Value* codeGen(CodeGenContext& context);
};

class NWhileStatement : public NStatement {
public:
	NExpression* condition;
	NStatement* loopBody;
	NWhileStatement(NExpression* condition, NStatement* loopBody) :
		condition(condition), loopBody(loopBody) {}
	virtual llvm::Value* codeGen(CodeGenContext& context);
};

class NContinueStatement : public NStatement {
public:
	NContinueStatement() {}
	virtual llvm::Value* codeGen(CodeGenContext& context);
};

class NBreakStatement : public NStatement {
public:
	NBreakStatement() {}
	virtual llvm::Value* codeGen(CodeGenContext& context);
};