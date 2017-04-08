#ifndef EE382V_ASSIGNMENT3_MEET_H
#define EE382V_ASSIGNMENT3_MEET_H

#include "llvm/Analysis/Interval.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/CFG.h"
#include "llvm/IR/Instructions.h"

#include <set>
#include <map>

using namespace llvm;

namespace ee382v
{

class Meet {
public:
	using TrackedSet = std::set<std::string>;
	using DFMap = std::map<const llvm::BasicBlock*,TrackedSet>;

	Meet() {}
	virtual ~Meet() = default;

	virtual bool operator()(BasicBlock* BB, DFMap& IN, DFMap& OUT) = 0;
};

class LiveMeet : public Meet {
public:
	LiveMeet() : Meet() {}

	virtual bool operator()(BasicBlock* BB, DFMap& IN, DFMap& OUT) override
	{
		bool change = false;
		for(auto iter = succ_begin(BB); iter!=succ_end(BB); iter++)
		{
			//since I record all possible variable in successors' IN(see transfer)
			//but I only want corresponding variable on the connected edge when I merge
			//so I need to make sure other phi variable from other predecessor are not moerged
			TrackedSet notMyPhi;
			for(auto& inst: (*(*iter)))
			{
				if(isa<PHINode>(inst))
				{
					const PHINode* phi = &(cast<PHINode>(inst));
					for(unsigned int i=0;i<phi->getNumIncomingValues();i++)
					{
						if(phi->getIncomingBlock(i)->getName().str() != BB->getName().str())
							notMyPhi.insert(phi->getIncomingValue(i)->getName().str());
					}
				}
			}
			for(auto& reg: IN[*iter])
			{
				if(notMyPhi.count(reg)==0)
					change |= OUT[BB].insert(reg).second;
			}
		}
		return change;
	}
};

class RdefMeet : public Meet {
public:
	RdefMeet() : Meet() {}

	virtual bool operator()(BasicBlock* BB, DFMap& IN, DFMap& OUT) override
	{
		bool change = false;
		for(auto iter = pred_begin(BB); iter!=pred_end(BB); iter++)
		{
			for(auto& reg: OUT[*iter])
			{
				change |= IN[BB].insert(reg).second;
			}
		}
		return change;
	}
};

}

#endif
