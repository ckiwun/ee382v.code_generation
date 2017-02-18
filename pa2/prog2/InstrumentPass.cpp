#define DEBUG_TYPE "InstrumentPass"

#include "InstrumentPass.h"

#include "llvm/ADT/Statistic.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Analysis/Interval.h"
#include "llvm/IR/Dominators.h"
#include "llvm/IR/Instructions.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/Debug.h"
#include "llvm/IR/BasicBlock.h"

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

void BL_DAG::buildNode(BLBlockNodeMap& inDag, BLNodeStack& dfsStack) {
	BL_Node* currentNode = dfsStack.top();
	BasicBlock* currentBlock = currentNode->getBlock();
	
	if(currentNode->getColor() != BL_Node::WHITE) {
		// we have already visited this node
		dfsStack.pop();
		currentNode->setColor(BL_Node::BLACK);
	}
	else {
		addNode(currentNode);
		currentNode->setColor(BL_Node::GRAY);
		inDag[currentBlock] = currentNode;
		
		// iterate through this node's successors
		for(succ_iterator succ = succ_begin(currentBlock); succ != succ_end(currentBlock); ++succ ) {
			BasicBlock* succBB = *succ;
			buildEdge(inDag, dfsStack, currentNode, succBB);
		}
	}
}

void BL_DAG::buildEdge(BLBlockNodeMap& inDag, BLNodeStack& dfsStack, BL_Node* currentNode, BasicBlock* succBB) {
	BL_Node* succNode = inDag[succBB];
	
	if(succNode && succNode->getColor() == BL_Node::BLACK) {
		// visited node and forward edge
		addEdge(createEdge(currentNode, succNode));
	}
	else if(succNode && succNode->getColor() == BL_Node::GRAY) {
		// visited node and back edge
		outs() << "Backedge detected.\n";
	}
	else {
		BL_Node* childNode;
		// not visited node and forward edge
		if(succNode) // an unvisited node that is child of a gray node
			childNode = succNode;
		else { // an unvisited node that is a child of a an unvisted node
			childNode = createNode(succBB);
			inDag[succBB] = childNode;
		}
		addEdge(createEdge(currentNode, childNode));
		dfsStack.push(childNode);
	}
}

void BL_DAG::build() {
	BLBlockNodeMap inDag;
	BLNodeStack dfsStack;

	BL_Node* node = createNode(_loop->getHeader());
	_entry = node;
	this->_exit = NULL;

	addNode(node);
	dfsStack.push(node);

	while(!dfsStack.empty())
		buildNode(inDag, dfsStack);
}

void BL_DAG::print() {
	outs() << "Loop #" << getLoopID() << "\n";
	auto pref = "";
	outs() << "Original BB List: \n";
	std::vector<BasicBlock*> blocks = _loop->getBlocks();
	for(auto block : blocks) {
		outs() << pref << block->getName();
		pref = ", ";
	}

	pref = "";
	outs() << "\nNode List:";
	for(auto node : _nodes) {
		outs() << pref << node->getBlock()->getName();
		pref = ", ";
	}

	outs() << "\nEdge List:";
	for(auto edge : _edges) {
		outs() << edge->getSource()->getBlock()->getName() << " => ";
		outs() << edge->getTarget()->getBlock()->getName() << "\n";
	}
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

	//2. build dag with all BB within this loop
	BL_DAG* dag = new BL_DAG(loop,global_loop_id);
	dag->build();
	if(global_loop_id==0) dag->print();

	//3. topological_sort
	dag->topological_sort();

	//4. calculate and assign edge value
	dag->calculatePathNumbers();

	//optimization (optional)

	//5. instruement [r+=weight(e)] at CHORD edge
	//6. instrument [count[loopid,r]++] at latch

	inner[global_loop_id++] = true;
	return false;

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
}

bool InstrumentPass::doFinalization() {
	auto pref = "";
	for(int i=0;i<global_loop_id;i++) {
		outs() << pref << inner[i];
		pref = ", ";
	}
	outs() << "\nFinish Instruementation!\n";
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
