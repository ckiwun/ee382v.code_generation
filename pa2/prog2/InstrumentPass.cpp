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
#include "llvm/IR/InstrTypes.h"

#include <string>

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
			//if(!_loop->contains(succBB)) continue;
			buildEdge(inDag, dfsStack, currentNode, succBB);
		}
	}
}

void BL_DAG::buildEdge(BLBlockNodeMap& inDag, BLNodeStack& dfsStack, BL_Node* currentNode, BasicBlock* succBB) {
	if(!_loop->contains(currentNode->getBlock())) return;
	BL_Node* succNode = inDag[succBB];
	
	if(succNode && succNode->getColor() == BL_Node::BLACK) {
		// visited node and forward edge
		addEdge(createEdge(currentNode, succNode));
	}
	else if(succNode && succNode->getColor() == BL_Node::GRAY) {
		// visited node and back edge
		// outs() << "Backedge detected.\n";
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

void BL_DAG::insert_pseudoexit() {
	BasicBlock* pseudoexit;
	std::string ebname = "ps.exit";
	unsigned number = 0;
	//auto pref = "";
	//Insert pseudo exit block first
	for(auto block : _blocks) {
		if(!_loop->isLoopExiting(block)) continue;
		auto term = block->getTerminator();
		for(unsigned idx = 0; idx<term->getNumSuccessors(); idx++) {
			auto succ = term->getSuccessor(idx);
			if(_loop->contains(succ)) continue;
			pseudoexit = BasicBlock::Create(_module->getContext(),ebname+std::to_string(number),_function);
			pseudoexit->moveAfter(block);

			term->setSuccessor(idx,pseudoexit);
			//outs() << "Old Block: " << block->getName() << ", Sucessor: ";
			//pref = "";
			//for(unsigned i = 0; i< term->getNumSuccessors(); i++) { outs() << pref << term->getSuccessor(i)->getName(); pref = ", "; }

			auto br_inst = BranchInst::Create(succ,pseudoexit);
			assert(br_inst->getNumSuccessors()==1);
			//outs() << "\nNew Block: " << pseudoexit->getName() << ", Sucessor: " << br_inst->getSuccessor(0)->getName() << "\n";
			number++;
		}
	}
	//assume one loop has only one latch
	//can't insert now, loop header phi entries don't match predecessors
	//auto latch = _loop->getLoopLatch();
	//if(latch) {
	//	pseudoexit = BasicBlock::Create(_module->getContext(),ebname+std::to_string(number),_function);
	//	pseudoexit->moveAfter(latch);
	//	_loop->addBlockEntry(pseudoexit);
	//	auto term = latch->getTerminator();
	//	assert(term->getNumSuccessors()==1);
	//	term->setSuccessor(0,pseudoexit);
	//	outs() << "Old Block: " << latch->getName() << ", Sucessor: " << term->getSuccessor(0)->getName() << "\n";
	//	auto br_inst = BranchInst::Create(_loop->getHeader(),pseudoexit);
	//	assert(br_inst->getNumSuccessors()==1);
	//	outs() << "New Block: " << br_inst->getParent()->getName() << ", Sucessor: " << br_inst->getSuccessor(0)->getName() << "\n";
	//}
}

void BL_DAG::build() {
	BLNodeStack dfsStack;

	BL_Node* node = createNode(_loop->getHeader());
	_entry = node;
	_exit = NULL;

	dfsStack.push(node);

	while(!dfsStack.empty())
		buildNode(_indag, dfsStack);

}

void BL_DAG::printInfo(bool is_inner) {
	outs() << "[Loop #" << getLoopID();
	if(is_inner) {
		outs() << ", Innermost Loop]";
	}
	else {
		outs() << ", Outer Loop]";
		return;
	}

	auto pref = "";
	outs() << "\n[Info] Original BB List: ";
	for(auto block : _blocks) {
		outs() << pref << block->getName();
		pref = ", ";
	}
	pref = "";
	outs() << "\n[Info] Node List: ";
	for(auto node : _nodes) {
		outs() << pref << node->getBlock()->getName();
		pref = ", ";
	}

	outs() << "\n[Info] Edge List:\n";
	for(auto edge : _edges) {
		outs() << edge->getSource()->getBlock()->getName() << " => ";
		outs() << edge->getTarget()->getBlock()->getName() << "\n";
	}

	pref = "";
	//Where finalize_reg should be instrumented
	//Insert pseudo exit block first
	outs() << "[Info] ExitingBlocks: ";
	for(auto block : _blocks) {
		if(!_loop->isLoopExiting(block)) continue;
		outs() << pref << block->getName();
		pref = ", ";
	}
	outs() << "\n[Info] Latch: " << _loop->getLoopLatch()->getName();

	//Where init_reg should be instrumented
	outs() << "\n[Info] FirstNonPhi: " << _loop->getHeader()->getFirstNonPHI()->getName();

	//Where inc_reg should be instrumented
	outs() << "\n[Info] NonZeroEdge: ";
	for(auto edge : _edges) {
		if(!edge->getWeight()) continue;
		outs() << edge->getSource()->getBlock()->getName() << " => ";
		outs() << edge->getTarget()->getBlock()->getName() << ", ";
	}
	outs() << "\n";
}

BL_Edge* BL_DAG::existEdge(BL_Node* source, BL_Node* target) {
	for(auto edge:_edges) {
		if(source==edge->getSource()&&target==edge->getTarget())
			return edge;
	}
	return NULL;
}

void BL_DAG::topologicalSortUtil(BL_Node* node, std::map<BL_Node*,bool>& visited, BLNodeStack &Stack)
{
	visited[node] = true;
	
	for (auto adj = node->succBegin(); adj!=node->succEnd(); ++adj)
	    if (!visited[(*adj)->getTarget()])
	        topologicalSortUtil((*adj)->getTarget(), visited, Stack);
	
	Stack.push(node);
}

void BL_DAG::topological_sort() {
	BLNodeStack Stack;
	std::map<BL_Node*,bool> visited;
	outs() << "[Debug] Topological Sorting: ";
	for(auto node:_nodes) {
		outs() << node->getBlock()->getName() << " ";
		visited[node] = false;
	}
	outs() << "\n";
	for (auto node:_nodes) {
		if (visited[node] == false)
			topologicalSortUtil(node, visited, Stack);
	}
	_nodes.clear();
	outs() << "[Debug] Sorted NodeList: ";
	while(!Stack.empty())
	{
		outs() << Stack.top()->getBlock()->getName() << " ";
		_nodes.push_back(Stack.top());
		Stack.pop();
	}
	outs() << "\n";
}

void BL_DAG::calculatePathNumbers() {
	outs() << "[Debug] Test Reverse Iter: ";
	for(auto rit = rbegin(); rit!=rend(); rit++) {
		outs() << (*rit)->getBlock()->getName() << " ";
	}
	outs() << "\n";
}

bool InstrumentPass::runOnLoop(llvm::Loop* loop, llvm::LPPassManager& lpm)
{
	BL_DAG* dag = new BL_DAG(loop,global_loop_id);
	//1. mark inner[loopid] false if not inner most loop
	if(!loop->getSubLoops().empty()) {
		inner[global_loop_id++] = false;
		dag->printInfo(false);
		return false;
	}
	inner[global_loop_id++] = true;

	//2. build dag with all BB within this loop
	dag->insert_pseudoexit();
	dag->build();

	//3. topological_sort
	dag->topological_sort();

	//4. calculate and assign edge value
	dag->calculatePathNumbers();
	dag->printInfo(true);

	//optimization (optional)

	//5. instruement [r+=weight(e)] at CHORD edge
	//6. instrument [count[loopid,r]++] at latch

	return true;

	//Module *M = L->getPreheader()->getParent()->getParent() ;
	//Constant *PrintFunc;
	//PrintFunc = M->getOrInsertFunction("Print", Type::getVoidTy(M->getContext()), (Type*)0);
	//FPrint= cast<Function>(PrintFunc);//Fprint is Function *Fprint defined in Mypass;
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
	//auto pref = "";
	//for(int i=0;i<global_loop_id;i++) {
	//	outs() << pref << inner[i];
	//	pref = ", ";
	//}
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
