#ifndef EE382V_ASSIGNMENT3_LIVE_H
#define EE382V_ASSIGNMENT3_LIVE_H

#include "llvm/Pass.h"
#include "llvm/IR/Instructions.h"

#include "meet.h"
#include "transfer.h"
#include "dataflow.h"

#include <set>

namespace ee382v
{

class  LiveAnalysis : public llvm::FunctionPass {
private:
	// Private field declaration here
public:
	using TrackedSet = std::set<std::string>;
	Dataflow<TrackedSet>* df;
	static char ID;
	LiveAnalysis() : llvm::FunctionPass(ID)
	{
		df = new Dataflow<TrackedSet>(false,new LiveMeet(),new LiveTransfer());
	}

	bool runOnFunction(llvm::Function&);

	// We don't modify the program, so we preserve all analyses
	void getAnalysisUsage(llvm::AnalysisUsage &AU) const;

	const TrackedSet& getInState(const llvm::BasicBlock* bb) const
	{
		return df->getInState(bb);
	}
	const TrackedSet& getOutState(const llvm::BasicBlock* bb) const
	{
		return df->getOutState(bb);
	}
};

}

#endif
