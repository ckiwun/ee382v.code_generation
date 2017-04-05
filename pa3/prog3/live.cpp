#include "live.h"
#include "dataflow.h"

#include <llvm/IR/Module.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/DebugInfo.h>
#include <llvm/IR/GlobalValue.h>
#include <llvm/IR/AssemblyAnnotationWriter.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/Support/FormattedStream.h>

using namespace llvm;
using namespace ee382v;
class Dataflow;

bool LiveAnalysis::runOnFunction(Function& F)
{
	Dataflow* a = new Dataflow(10);
	outs() << a->getID();

	for(Function::iterator bb=F.begin();bb!=F.end();bb++)
	{
		outs() << bb->getName() << "\n";
	}

	return false;
}

void LiveAnalysis::getAnalysisUsage(AnalysisUsage &AU) const
{
	AU.setPreservesAll();
}

// LLVM uses the address of this static member to identify the pass, so the
// initialization value is unimportant.
char LiveAnalysis::ID = 0;

// Register the pass name to allow it to be called with opt:
// See http://llvm.org/releases/3.9.1/docs/WritingAnLLVMPass.html#running-a-pass-with-opt for more info.
static RegisterPass<LiveAnalysis> X("live", "Apply liveness dataflow analysis");
