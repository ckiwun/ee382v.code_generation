#ifndef EE382V_ASSIGNMENT3_MEET_H
#define EE382V_ASSIGNMENT3_MEET_H

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
		return false;
	}
};

class RdefMeet : public Meet {
public:
	RdefMeet() : Meet() {}

	virtual bool operator()(BasicBlock* BB, DFMap& IN, DFMap& OUT) override
	{
		return false;
	}
};

}

#endif
