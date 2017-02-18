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
			if(!_loop->contains(succBB)) continue;
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
	BL_Node* node;
	BL_Edge* edge;
	std::string ebname = "ps.exit";
	unsigned number = 0;
	//Insert pseudo exit block first
	for(auto block : _blocks) {
		if(!_loop->isLoopExiting(block)) continue;
		pseudoexit = BasicBlock::Create(_module->getContext(),ebname+std::to_string(number),_function);
		pseudoexit->moveAfter(block);
		node = createNode(pseudoexit);
		edge = createEdge(_indag[block],node);
		addNode(node);
		addEdge(edge);
		number++;
	}
	pseudoexit = BasicBlock::Create(_module->getContext(),ebname+std::to_string(number),_function);
	pseudoexit->moveAfter(_loop->getLoopLatch());
	node = createNode(pseudoexit);
	edge = createEdge(_indag[_loop->getLoopLatch()],node);
	addNode(node);
	addEdge(edge);
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

void BL_DAG::topological_sort() {
	//Stack storing "no incoming edge" node
	BLNodeQueue S;
	BLNodeVector shadow(_nodes);
	_nodes.clear();
	std::map<BL_Node*,bool> selected;

	for(auto node:shadow)
		selected[node] = false;

	do {
		for(auto node:shadow){
			if(node->getNumberPredEdges()==0 && !selected[node]){
				selected[node] = true;
				S.push(node);
			}
		}
		BL_Node* current_node = S.front();
		_nodes.push_back(current_node);
		S.pop();
		BL_Edge* edge;
		for(auto node : shadow){
			if(node==current_node) continue;
			outs() << "remove edge!!!\n";
			edge = existEdge(current_node,node);
			if(edge){
				outs() << "edge detected current to node\n";
				current_node->removeSuccEdge(edge);
				node->removePredEdge(edge);
				for(auto it=shadow.begin();it!=shadow.end();it++)
					if((*it)==current_node) shadow.erase(it);
			}
			edge = existEdge(node,current_node);
			if(edge){
				outs() << "edge detected node to current\n";
				current_node->removePredEdge(edge);
				node->removeSuccEdge(edge);
				for(auto it=shadow.begin();it!=shadow.end();it++)
					if((*it)==current_node) shadow.erase(it);
			}
		}		
	} while(!S.empty());
}

void BL_DAG::calculatePathNumbers() {

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
	dag->build();
	//dag->insert_pseudoexit();

	//3. topological_sort
	//dag->topological_sort();

	//4. calculate and assign edge value
	dag->calculatePathNumbers();
	dag->printInfo(true);

	//optimization (optional)

	//5. instruement [r+=weight(e)] at CHORD edge
	//6. instrument [count[loopid,r]++] at latch

	return false;

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
