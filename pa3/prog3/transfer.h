#ifndef EE382V_ASSIGNMENT3_TRANSFER_H
#define EE382V_ASSIGNMENT3_TRANSFER_H

#include <set>
#include <map>

#include "llvm/Support/raw_ostream.h"
#include "llvm/IR/Dominators.h"

using namespace llvm;

namespace ee382v
{

class Transfer {
public:
	using TrackedSet = std::set<std::string>;
	using DFMap = std::map<const llvm::BasicBlock*,TrackedSet>;

	DFMap GEN, KILL;

	Transfer() {}
	virtual ~Transfer() = default;

	virtual void calculateGENKILL(Function&, raw_ostream& os) = 0;
	virtual bool operator()(BasicBlock* BB, DFMap& IN, DFMap& OUT) = 0;
};

class LiveTransfer : public Transfer {
public:
	LiveTransfer() : Transfer() {}

	virtual void calculateGENKILL(Function& F, raw_ostream& os) override
	{
		for(auto& bb: F)
		{
			for(auto& inst: bb)
			{
				if(isa<PHINode>(inst))//not sure what to do, just put all op to gen list in conservative manner now
				{
					KILL[&bb].insert(inst.getName().str());
					for(auto opIter = inst.op_begin(); opIter!=inst.op_end();opIter++)
					{
						int count = KILL[&bb].count((*opIter)->getName().str());
						if(!(isa<Constant>(*opIter))&&count==0)
						{
							GEN[&bb].insert((*opIter)->getName().str());
						}
					}
				}
				else if (!isa<TerminatorInst>(inst))
				{
					KILL[&bb].insert(inst.getName().str());
					for(auto opIter = inst.op_begin(); opIter!=inst.op_end();opIter++)
					{
						int count = KILL[&bb].count((*opIter)->getName().str());
						if(!(isa<Constant>(*opIter))&&count==0)
						{
							GEN[&bb].insert((*opIter)->getName().str());
						}
					}
				}
				else
				{
					if(!inst.getName().str().empty())
					{
						KILL[&bb].insert(inst.getName().str());
					}
					for(auto opIter = inst.op_begin(); opIter!=inst.op_end();opIter++)
					{
						int count = KILL[&bb].count((*opIter)->getName().str());
						if(!(isa<Constant>(*opIter))&&!(isa<BasicBlock>(*opIter))&&count==0)
						{
							GEN[&bb].insert((*opIter)->getName().str());
						}
					}
				}
			}
		}
	}

	virtual bool operator()(BasicBlock* BB, DFMap& IN, DFMap& OUT) override
	{
		bool change = false;
		for(auto op: OUT[BB])
		{
			if(KILL[BB].count(op)==0)//if not killed, pass to IN set
				change |= IN[BB].insert(op).second;
		}
		for(auto op: GEN[BB])
		{
			change |= IN[BB].insert(op).second;
		}
		return change;
	}
};

class RdefTransfer : public Transfer {
public:
	RdefTransfer() : Transfer() {}

	virtual void calculateGENKILL(Function& F, raw_ostream& os) override
	{
		DominatorTree DomTree(F);
		for(auto& bb: F)
		{
			for(auto& inst: bb)
			{
				if(isa<PHINode>(inst))
				{
					GEN[&bb].insert(inst.getName().str());
					const PHINode* phi = &(cast<PHINode>(inst));
					//assume i is either 0 or 1
					BasicBlock* leftBlock = phi->getIncomingBlock(0);
					BasicBlock* rightBlock = phi->getIncomingBlock(1);
					bool left_dom = DomTree.dominates(leftBlock,rightBlock);
					bool right_dom = DomTree.dominates(rightBlock,leftBlock);
					if(left_dom)
						KILL[&bb].insert(phi->getIncomingValue(1)->getName().str());
					else if(right_dom)
						KILL[&bb].insert(phi->getIncomingValue(0)->getName().str());

				}
				else if(!isa<TerminatorInst>(inst))
				{
					GEN[&bb].insert(inst.getName().str());
				}
			}
		}
	}

	virtual bool operator()(BasicBlock* BB, DFMap& IN, DFMap& OUT) override
	{
		bool change = false;
		for(auto op: IN[BB])
		{
			if(KILL[BB].count(op)==0)//if not killed, pass to IN set
				change |= OUT[BB].insert(op).second;
		}
		for(auto op: GEN[BB])
		{
			change |= OUT[BB].insert(op).second;
		}
		return change;
	}
};

}

#endif
