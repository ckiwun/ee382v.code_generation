#ifndef EE382V_ASSIGNMENT3_TRANSFER_H
#define EE382V_ASSIGNMENT3_TRANSFER_H

#include <set>
#include <map>

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

	virtual void calculateGENKILL(llvm::Function&) = 0;
	virtual bool operator()(BasicBlock* BB, DFMap& IN, DFMap& OUT) = 0;
};

class LiveTransfer : public Transfer {
public:
	LiveTransfer() : Transfer() {}

	virtual void calculateGENKILL(llvm::Function& F)
	{
		for(auto& bb: F)
		{
			GEN[&bb]=TrackedSet();
			KILL[&bb]=TrackedSet();
		}
		//calculate gen/kill
	}

	virtual bool operator()(BasicBlock* BB, DFMap& IN, DFMap& OUT) override
	{
		return false;
	}
};

class RdefTransfer : public Transfer {
public:
	RdefTransfer() : Transfer() {}

	virtual void calculateGENKILL(llvm::Function& F)
	{
		for(auto& bb: F)
		{
			GEN[&bb]=TrackedSet();
			KILL[&bb]=TrackedSet();
		}
		//calculate gen/kill
	}

	virtual bool operator()(BasicBlock* BB, DFMap& IN, DFMap& OUT) override
	{
		return false;
	}
};

}

#endif
