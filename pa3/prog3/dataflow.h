#ifndef EE382V_ASSIGNMENT3_DATAFLOW_H
#define EE382V_ASSIGNMENT3_DATAFLOW_H

#include "llvm/Pass.h"
#include "llvm/IR/Instructions.h"

namespace ee382v {

class Dataflow {
private:
	int _id;

public:
	Dataflow() {}
	Dataflow(int id) : _id(id) {}
	virtual ~Dataflow() = default;

	virtual int getID() {return _id;}

	

};

}

#endif
