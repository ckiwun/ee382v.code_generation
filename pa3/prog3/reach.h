#ifndef EE382V_ASSIGNMENT3_REACH_H
#define EE382V_ASSIGNMENT3_REACH_H

#include "llvm/Pass.h"
#include "llvm/IR/Instructions.h"

#include "util.h"
#include "meet.h"
#include "transfer.h"
#include "dataflow.h"

namespace ee382v
{

class RdefAnalysis : public llvm::FunctionPass {
private:
	// Private field declaration here

public:
	static char ID;
	RdefAnalysis() : llvm::FunctionPass(ID) {}

	bool runOnFunction(llvm::Function&);

	// We don't modify the program, so we preserve all analyses
	void getAnalysisUsage(llvm::AnalysisUsage &AU) const;
};

}

#endif
