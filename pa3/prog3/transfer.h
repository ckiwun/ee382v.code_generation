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

	Transfer() {}
	virtual ~Transfer() = default;

	virtual bool operator()(BasicBlock* BB, DFMap& IN, DFMap& OUT) = 0;
};

class LiveTransfer : public Transfer {
public:
	LiveTransfer() : Transfer() {}

	virtual bool operator()(BasicBlock* BB, DFMap& IN, DFMap& OUT) override
	{
		return false;
	}
};

class RdefTransfer : public Transfer {
public:
	RdefTransfer() : Transfer() {}

	virtual bool operator()(BasicBlock* BB, DFMap& IN, DFMap& OUT) override
	{
		return false;
	}
};

}

#endif
