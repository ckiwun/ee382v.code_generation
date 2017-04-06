#ifndef EE382V_ASSIGNMENT3_UTIL_H
#define EE382V_ASSIGNMENT3_UTIL_H

using namespace llvm;

namespace ee382v
{

class MyHash {
public:
	size_t operator() (const StringRef& element) const
	{
		std::hash<std::string> stringHash;
		return stringHash(element.str());
	}
};

class MyEqual {
public:
	size_t operator() (const StringRef& lhs, const StringRef& rhs) const
	{
		return lhs.str().compare(rhs.str());
	}
};

class WorkList {
public:

};

}

#endif
