#ifndef EE382V_ASSIGNMENT1_H
#define EE382V_ASSIGNMENT1_H

#include "llvm/Pass.h"
#include "llvm/IR/Instructions.h"

namespace ee382v
{

class IRPrinter: public llvm::ModulePass {
private:
	// Private field declaration here

public:
	static char ID;
	IRPrinter() : llvm::ModulePass(ID) {}

	bool runOnModule(llvm::Module&);

	// We don't modify the program, so we preserve all analyses
	void getAnalysisUsage(llvm::AnalysisUsage &AU) const;
};

}

#endif
