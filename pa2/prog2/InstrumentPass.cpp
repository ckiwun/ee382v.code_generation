#define DEBUG_TYPE "InstrumentPass"

#include "InstrumentPass.h"

#include "llvm/ADT/Statistic.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/IR/Dominators.h"
#include "llvm/IR/Instructions.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/Debug.h"

using namespace ee382v;
using namespace llvm;

bool BL_Node::removePredEdge(BL_Edge* edge) {
	for(auto eit = _predEdges.begin(); eit!= _predEdges.end(); eit++) {
		if((*(*eit))==(*edge)) {
			_predEdges.erase(eit);
			return true;
		}
	}
	return false;
}

bool BL_Node::removeSuccEdge(BL_Edge* edge) {
	for(auto eit = _succEdges.begin(); eit!= _succEdges.end(); eit++) {
		if((*(*eit))==(*edge)) {
			_succEdges.erase(eit);
			return true;
		}
	}
	return false;
}

void BL_DAG::build(Loop* loop) {

}

void BL_DAG::topological_sort() {

}

void BL_DAG::calculatePathNumbers() {

}

bool InstrumentPass::runOnLoop(llvm::Loop* loop, llvm::LPPassManager& lpm)
{
	//1. mark inner[loopid] false if not inner most loop
	if(!loop->getSubLoops().empty()) {
		inner[global_loop_id++] = false;
		return false;
	}
	else {
		inner[global_loop_id++] = true;
	}

	//2. build dag with all BB within this loop
	BL_DAG* dag;
	dag->build(loop);

	//3. topological_sort
	dag->topological_sort();

	//4. calculate and assign edge value
	dag->calculatePathNumbers();

	//optimization (optional)

	//5. instruement [r+=weight(e)] at CHORD edge
	//6. instrument [count[loopid,r]++] at latch

	//LoopInfo& loopInfo = getAnalysis<LoopInfoWrapperPass>().getLoopInfo();
	//DominatorTree& domTree = getAnalysis<DominatorTreeWrapperPass>().getDomTree();
	//DomTreeNode *node = domTree.getNode(loop->getHeader());
	//FunctionType *FunTy = FunctionType::get( Type::getVoidTy( MP->getContext() ), ... );
	//Function *Function = dyn_cast<Function> ( MP->getOrInsertFunction(...) );
	//APInt LoopId(...);
	//Value *init_arg_values[] = { Constant::getIntegerValue(...), ... };
	//CallInst *call = CallInst::Create(...);
	//call->insertBefore(???->getFirstNonPHI());
	//call->insertBefore(latch->getTerminator());
	return false;
}

bool InstrumentPass::doFinalization() {
	for(int i=0;i<global_loop_id;i++)
		outs() << inner[i] << "\n";
	outs() << "Finish Instruementation!\n";
	return false;
}

void InstrumentPass::getAnalysisUsage(AnalysisUsage &AU) const
{
	AU.addRequired<LoopInfoWrapperPass>();
	AU.addRequired<DominatorTreeWrapperPass>();
	AU.addRequiredTransitive<DominatorTreeWrapperPass>();
	AU.setPreservesCFG();
}

char InstrumentPass::ID = 0;
int InstrumentPass::global_loop_id = 0;
static RegisterPass<InstrumentPass> X("epp", "Efficient Path Profiling Instrument Pass.");
