#ifndef EE382V_ASSIGNMENT3_REACH_H
#define EE382V_ASSIGNMENT3_REACH_H

#include "llvm/Pass.h"
#include "llvm/IR/Instructions.h"

#include "meet.h"
#include "transfer.h"
#include "dataflow.h"

namespace ee382v
{

class RdefAnalysis : public llvm::FunctionPass {
private:
	// Private field declaration here
	Dataflow* df;
public:
	using TrackedSet = std::set<std::string>;
	static char ID;
	RdefAnalysis() : llvm::FunctionPass(ID)
	{
		df = new Dataflow(true,new RdefMeet(),new RdefTransfer());
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
