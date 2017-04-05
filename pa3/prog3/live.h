#ifndef EE382V_ASSIGNMENT3_LIVE_H
#define EE382V_ASSIGNMENT3_LIVE_H

#include "llvm/Pass.h"
#include "llvm/IR/Instructions.h"

namespace ee382v
{

class  LiveAnalysis : public llvm::FunctionPass {
private:
	// Private field declaration here

public:
	static char ID;
	LiveAnalysis() : llvm::FunctionPass(ID) {}

	bool doInitialization() {return false;}
	bool doFinalization() {return false;}
	bool runOnFunction(llvm::Function&);

	// We don't modify the program, so we preserve all analyses
	void getAnalysisUsage(llvm::AnalysisUsage &AU) const;
};

}

#endif
