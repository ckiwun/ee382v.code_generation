#ifndef EE382V_ASSIGNMENT3_DATAFLOW_H
#define EE382V_ASSIGNMENT3_DATAFLOW_H

#include "llvm/Pass.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Function.h"

#include <set>
#include <map>
#include <deque>

#include "meet.h"
#include "transfer.h"

namespace ee382v {

class Dataflow {
public:
	using TrackedSet = std::set<std::string>;
	using DFMap = std::map<const llvm::BasicBlock*,TrackedSet>;

	bool _forward;
	Meet* _meet;
	Transfer* _transfer;
	DFMap _imap, _omap;
	TrackedSet _initial;
	
	Dataflow() {}
	Dataflow(bool forward, Meet* meet, Transfer* transfer)
		: _forward(forward), _meet(meet), _transfer(transfer) {}

	void compute(llvm::Function& F)
	{
		std::deque<llvm::BasicBlock*> BBlist;
		if(_forward)
		{
			for(auto &bb : F)
				BBlist.push_back(&bb);
		}
		else
		{
			for(auto &bb : F)
				BBlist.push_front(&bb);
		}

		bool change = true;
		while(change)
		{
			change = false;
			for(auto currBB : BBlist)
			{
				change |= (*_meet)(currBB, _imap, _omap);
				change |= (*_transfer)(currBB, _imap, _omap);
			}
		}
	}

	const TrackedSet& getInState(const llvm::BasicBlock* bb) const
	{
		return _imap.at(bb);
	}
	const TrackedSet& getOutState(const llvm::BasicBlock* bb) const
	{
		return _omap.at(bb);
	}
};

}

#endif
