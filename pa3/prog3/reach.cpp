#include "reach.h"
#include "DataFlowAnnotator.h"

#include <llvm/IR/Module.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/DebugInfo.h>
#include <llvm/IR/GlobalValue.h>
#include <llvm/IR/AssemblyAnnotationWriter.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/Support/FormattedStream.h>

using namespace llvm;
using namespace ee382v;

bool RdefAnalysis::runOnFunction(Function& F)
{
	TrackedSet initial;
	for (auto iter = F.arg_begin(); iter!=F.arg_end(); iter++)
	{
		initial.insert(iter->getName().str());
	}
	df->init(F, initial);
	df->compute(F);
	DataFlowAnnotator<RdefAnalysis> annotator(*this);
	annotator.print(F);
	return false;
}

void RdefAnalysis::getAnalysisUsage(AnalysisUsage &AU) const
{
	AU.setPreservesAll();
}

// LLVM uses the address of this static member to identify the pass, so the
// initialization value is unimportant.
char RdefAnalysis::ID = 0;

// Register the pass name to allow it to be called with opt:
// See http://llvm.org/releases/3.9.1/docs/WritingAnLLVMPass.html#running-a-pass-with-opt for more info.
static RegisterPass<RdefAnalysis> X("reach", "Apply reaching definition dataflow analysis");
