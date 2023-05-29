#include <iostream>
#include "codegen.h"
#include "node.h"

extern int yyparse();
extern NBlock* programBlock;


void createCoreFunctions(CodeGenContext& context);

int main(int argc, char **argv){
	std::string IRFile = "a.ll";
	std::string ObjectFile = "a.o";

	if (argc <= 1) {
		std::cout << "no input file" << std::endl;
		return 0;
	}
	else {
		freopen(argv[1], "r", stdin); //标准输入重定向
		for (int i=2; i<argc; i++){
			std::string tmp = argv[i];
			if(tmp == "-o"){
				ObjectFile = std::string(argv[++i]);
				if(ObjectFile.length()<=2 || ObjectFile.substr(ObjectFile.length()-2) != ".o"){
					throw std::runtime_error("object file is not *.o");
					return -1;
				}
			}
			else if(tmp == "-l"){
				IRFile = std::string(argv[++i]);
				if(IRFile.length()<=3 || IRFile.substr(IRFile.length()-3) != ".ll"){
					throw std::runtime_error("ir file is not *.ll");
					return -1;
				}
			}
			else{
				throw std::runtime_error("invalid input argument");
				return -1;
			}
		}
	}

	CodeGenContext context;

	// parse input file
	if(yyparse()){
		throw std::runtime_error("Syntax error.");
		return -1;
	}
	// build AST and generate IR
	context.generateIRCode(*programBlock);
	// print and save IR
	context.dumpIR(IRFile);
	// generate object file
	context.generateObjectCode(ObjectFile);

	std::cout << "IR file: " << IRFile << std::endl;
	std::cout << "object file: " << ObjectFile << std::endl;
	std::cout << "compile OK" << std::endl;
	
#ifdef DEBUG
	// run input file
	context.runCode();
#endif
	
	return 0;
}

