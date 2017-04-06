#ifndef EE382V_ASSIGNMENT3_LIVE_H
#define EE382V_ASSIGNMENT3_LIVE_H

#include "llvm/Pass.h"
#include "llvm/IR/Instructions.h"

#include "util.h"
#include "meet.h"
#include "transfer.h"
#include "dataflow.h"

namespace ee382v
{

class  LiveAnalysis : public llvm::FunctionPass {
private:
	// Private field declaration here
	Dataflow* df;
public:
	static char ID;
	LiveAnalysis() : llvm::FunctionPass(ID) {
		df = new Dataflow(10);
	}

	bool runOnFunction(llvm::Function&);

	// We don't modify the program, so we preserve all analyses
	void getAnalysisUsage(llvm::AnalysisUsage &AU) const;
};

}

#endif
