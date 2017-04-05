#ifndef EE382V_ASSIGNMENT3_DATAFLOW_H
#define EE382V_ASSIGNMENT3_DATAFLOW_H

#include "llvm/Pass.h"
#include "llvm/IR/Instructions.h"

namespace ee382v
{

class Dataflow {
private:

public:
	Dataflow(int id) : _id(id) {}
	virtual ~Dataflow();

	int _id;
	virtual int getID();

};

class Live : public Dataflow {
private:

public:
	Live(int id) : Dataflow(id) {}

};

}

#endif
