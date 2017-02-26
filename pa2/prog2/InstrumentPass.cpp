#define DEBUG_TYPE "InstrumentPass"

#include "InstrumentPass.h"
#include "epp_runtime.h"

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
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Type.h"

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
	std::string ebname = "exit";
	static unsigned number = 0;
	//Insert pseudo exit block first
	for(auto block : _blocks) {
		if(!_loop->isLoopExiting(block)) continue;
		auto term = block->getTerminator();
		for(unsigned idx = 0; idx<term->getNumSuccessors(); idx++) {
			auto succ = term->getSuccessor(idx);
			if(_loop->contains(succ)) continue;
			pseudoexit = BasicBlock::Create(_module->getContext(),ebname+std::to_string(number),_function);
			auto ploop = _loop->getParentLoop();
			while(ploop) {
				ploop->addBlockEntry(pseudoexit);
				ploop = ploop->getParentLoop();
			}
			pseudoexit->moveAfter(block);

			term->setSuccessor(idx,pseudoexit);
			auto br_inst = BranchInst::Create(succ,pseudoexit);
			assert(br_inst->getNumSuccessors()==1);
			number++;
		}
	}
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
		outs() << ", Outer Loop]\n";
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
	//outs() << "[Info] ExitingBlocks: ";
	//for(auto block : _blocks) {
	//	if(!_loop->isLoopExiting(block)) continue;
	//	outs() << pref << block->getName();
	//	pref = ", ";
	//}
	//outs() << "\n[Info] Latch: " << _loop->getLoopLatch()->getName();

	//Where init_reg should be instrumented
	//outs() << "\n[Info] FirstNonPhi: " << _loop->getHeader()->getFirstNonPHI()->getName();

	//Where inc_reg should be instrumented
	outs() << "[Info] NonZeroEdge: ";
	pref = "";
	for(auto edge : _edges) {
		if(!edge->getWeight()) continue;
		outs() << pref << edge->getSource()->getBlock()->getName() << " => ";
		outs() << edge->getTarget()->getBlock()->getName();
		outs() << "(" << edge->getWeight() << ")";
		pref = ", ";
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
	for (auto node:_nodes) {
		if (visited[node] == false)
			topologicalSortUtil(node, visited, Stack);
	}
	_nodes.clear();
	while(!Stack.empty())
	{
		_nodes.push_back(Stack.top());
		Stack.pop();
	}
}

void BL_DAG::calculatePathNumbers() {
	for(auto rit = rbegin(); rit!=rend(); rit++) {
		if((*rit)->getNumberSuccEdges()==0) {
			(*rit)->setNumberPaths(1);
			continue;
		}
		(*rit)->setNumberPaths(0);
		for(auto iter=(*rit)->succBegin();iter!=(*rit)->succEnd();iter++) {
			(*iter)->setWeight((*rit)->getNumberPaths());
			(*rit)->setNumberPaths((*rit)->getNumberPaths()+(*iter)->getTarget()->getNumberPaths());
		}
	}
}
void BL_DAG::calculateRouteInfo() {
	SmallVector<BasicBlock*,8> latches;
	_loop->getLoopLatches(latches);
	for(auto latch:latches){
		printAllPaths(_indag[_loop->getHeader()],_indag[latch]);
	}
	for(auto node : _nodes) {
		auto block = node->getBlock();
		std::string blockname = block->getName().substr(0,4);
		if(blockname!="exit") continue;
		printAllPaths(_indag[_loop->getHeader()],_indag[block]);
	}
}

void BL_DAG::printAllPaths(BL_Node* s, BL_Node* d) {
	std::map<BL_Node*,bool> visited;
	std::vector<std::string> path;

	for (auto node:_nodes)
		visited[node] = false;

	printAllPathsUtil(s, d, visited, path, 0);
}

void BL_DAG::printAllPathsUtil(BL_Node* u, BL_Node* d, std::map<BL_Node*,bool> visited, std::vector<std::string> path, unsigned path_val) {
	visited[u] = true;
	path.push_back(u->getBlock()->getName());
	
	if (u == d)
	{
		std::string fpath;
		auto pref="";
		for(auto str:path) {
			fpath+=pref;
			fpath+=str;
			pref = "<=";
		}
		_parent->route[_loopid][path_val] = fpath;
		//outs() << fpath;
		//outs() << "(" << path_val << ")";
		//outs() << "\n";
	}
	else
	{
		for (auto adj = u->succBegin(); adj!=u->succEnd(); ++adj) {
			if (!visited[(*adj)->getTarget()])
				printAllPathsUtil((*adj)->getTarget(), d, visited, path, path_val+(*adj)->getWeight());
		}
	}
	
	path.pop_back();
	visited[u] = false;
}

void BL_DAG::instrumentation() {
	//init_path_reg
	Function* init_path = cast<Function>(getModule()->getOrInsertFunction("init_path_reg", Type::getVoidTy(getModule()->getContext()), IntegerType::get(getModule()->getContext(),32) ,NULL));
	Value* init_path_arg= Constant::getIntegerValue(IntegerType::get(getModule()->getContext(),32),APInt(32,getLoopID()));
	CallInst* init_path_call = CallInst::Create(init_path,init_path_arg);
	BasicBlock* loopheader = getLoop()->getHeader();
	init_path_call->insertBefore(loopheader->getFirstNonPHI());

	//inc_path_reg
	std::string edgename("edge");
	unsigned number = 0;
	//insert on edge between exiting block and pseudo exit, need to create a new basic block between
	Function* inc_path = cast<Function>(getModule()->getOrInsertFunction("inc_path_reg", Type::getVoidTy(getModule()->getContext()), IntegerType::get(getModule()->getContext(),32), IntegerType::get(getModule()->getContext(),32) ,NULL));
	for(auto edge : _edges) {
		if(!edge->getWeight()) continue;
		//insert new basic block along the edge
		BasicBlock* source = edge->getSource()->getBlock();
		BasicBlock* target = edge->getTarget()->getBlock();
		auto source_term = source->getTerminator();
		for(unsigned idx = 0; idx<source_term->getNumSuccessors(); idx++) {
			auto succ = source_term->getSuccessor(idx);
			if(_loop->contains(succ)) continue;
			if(target!=succ) continue;
			BasicBlock* edgeblock = BasicBlock::Create(_module->getContext(),edgename+std::to_string(number++),_function,succ);
			auto ploop = _loop->getParentLoop();
			while(ploop) {
				ploop->addBlockEntry(edgeblock);
				ploop = ploop->getParentLoop();
			}
			source_term->setSuccessor(idx,edgeblock);
			BranchInst::Create(succ,edgeblock);
			Value* inc_path_arg[]= {Constant::getIntegerValue(IntegerType::get(getModule()->getContext(),32),APInt(32,getLoopID())),Constant::getIntegerValue(IntegerType::get(getModule()->getContext(),32),APInt(32,edge->getWeight()))};
			CallInst* inc_path_call = CallInst::Create(inc_path,inc_path_arg);
			inc_path_call->insertBefore(edgeblock->getTerminator());
		}
	}
	//insert on in-loop edge
	for(auto edge : _edges) {
		if(!edge->getWeight()) continue;
		//insert new basic block along the edge
		BasicBlock* source = edge->getSource()->getBlock();
		BasicBlock* target = edge->getTarget()->getBlock();
		auto source_term = source->getTerminator();
		for(unsigned idx = 0; idx<source_term->getNumSuccessors(); idx++) {
			auto succ = source_term->getSuccessor(idx);
			if(target!=succ) continue;
			BasicBlock* edgeblock = BasicBlock::Create(_module->getContext(),edgename+std::to_string(number++),_function,target);
			_loop->addBlockEntry(edgeblock);
			auto ploop = _loop->getParentLoop();
			while(ploop) {
				ploop->addBlockEntry(edgeblock);
				ploop = ploop->getParentLoop();
			}
			source_term->setSuccessor(idx,edgeblock);
			BranchInst::Create(succ,edgeblock);
			Value* inc_path_arg[]= {Constant::getIntegerValue(IntegerType::get(getModule()->getContext(),32),APInt(32,getLoopID())),Constant::getIntegerValue(IntegerType::get(getModule()->getContext(),32),APInt(32,edge->getWeight()))};
			CallInst* inc_path_call = CallInst::Create(inc_path,inc_path_arg);
			inc_path_call->insertBefore(edgeblock->getTerminator());
		}
	}
	//finalize_path_reg
	Function* finalize_path = cast<Function>(getModule()->getOrInsertFunction("finalize_path_reg", Type::getVoidTy(getModule()->getContext()), IntegerType::get(getModule()->getContext(),32) ,NULL));
	Value* finalize_path_arg= Constant::getIntegerValue(IntegerType::get(getModule()->getContext(),32),APInt(32,getLoopID()));
	CallInst* finalize_path_call = CallInst::Create(finalize_path,finalize_path_arg);
	SmallVector<BasicBlock*,8> latches;
	_loop->getLoopLatches(latches);
	for(auto latch:latches){
		finalize_path_call->insertBefore(latch->getTerminator());
	}
	for(auto node : _nodes) {
		auto block = node->getBlock();
		std::string blockname = block->getName().substr(0,4);
		if(blockname!="exit") continue;
		CallInst* finalize_path_call = CallInst::Create(finalize_path,finalize_path_arg);
		finalize_path_call->insertBefore(block->getTerminator());
	}
}

bool InstrumentPass::runOnLoop(llvm::Loop* loop, llvm::LPPassManager& lpm)
{
	BL_DAG* dag = new BL_DAG(loop,global_loop_id++,this);
	if(!loop->getSubLoops().empty()) {
		//dag->printInfo(false);
		return false;
	}

	dag->insert_pseudoexit();
	dag->build();

	dag->topological_sort();
	dag->calculatePathNumbers();
	dag->calculateRouteInfo();

	//dag->printInfo(true);

	dag->instrumentation();

	return true;
}

bool InstrumentPass::doFinalization() {
	outs() << "LoopID  PathID  PathInfo\n";
	for(auto loop:route)
	for(auto path:loop.second)
		outs() << loop.first << "       " << path.first << "       " << path.second << "\n";

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
