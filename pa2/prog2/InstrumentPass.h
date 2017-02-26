#ifndef EE382V_ASSIGNMENT2_EPP_H
#define EE382V_ASSIGNMENT2_EPP_H

#include "llvm/Analysis/LoopPass.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/IR/Dominators.h"

//added by me
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Module.h"
#include <iostream>
#include <vector>
#include <map>
#include <queue>
#include <stack>
#include <unordered_set>
#include <unordered_map>
#include <string>

namespace ee382v
{
using llvm::BasicBlock;
using llvm::Function;
using llvm::Module;
using llvm::Loop;

class BasicBlock;
class Function;
class Module;
class BL_Node;
class BL_Edge;
class BL_DAG;

typedef std::vector<BL_Node*> BLNodeVector;
typedef std::vector<BL_Node*>::iterator BLNode_iter;
typedef std::vector<BL_Node*>::reverse_iterator BLNode_riter;
typedef std::vector<BL_Edge*> BLEdgeVector;
typedef std::vector<BL_Edge*>::iterator BLEdge_iter;
typedef std::map<BasicBlock*, BL_Node*> BLBlockNodeMap;
typedef std::stack<BL_Node*> BLNodeStack;
typedef std::queue<BL_Node*> BLNodeQueue;

class InstrumentPass: public llvm::LoopPass
{
public:
	static char ID;
	InstrumentPass() : llvm::LoopPass(ID) {}

	bool runOnLoop(Loop*, llvm::LPPassManager&);
	bool doFinalization();
	int getLoopID() {return global_loop_id;}

	void getAnalysisUsage(llvm::AnalysisUsage &AU) const;

private:
	static int global_loop_id;
	std::map<unsigned,std::map<unsigned,std::string>> route;

	friend class BL_DAG;

};

class BL_Node {
public:
	enum NodeColor { WHITE, GRAY, BLACK };
	
	BL_Node(BasicBlock* BB)
	: _basicBlock(BB), _numberPaths(0), _color(WHITE) {
		static unsigned nextUID = 0;
		_uid = nextUID++;
	}

	//Accessors
	BasicBlock* getBlock() const {return _basicBlock;}
	unsigned getNumberPaths() const {return _numberPaths;}
	NodeColor getColor() const {return _color;}
	unsigned getNumberPredEdges() const {return _predEdges.size();}
	unsigned getNumberSuccEdges() const {return _succEdges.size();}
	unsigned getUID() const {return _uid;}

	//Modifiers
	void setNumberPaths(unsigned numberPaths) {_numberPaths = numberPaths;}
	
	void setColor(NodeColor color) {_color = color;}
	void addPredEdge(BL_Edge* edge) {_predEdges.push_back(edge);}
	bool removePredEdge(BL_Edge* edge);
	void addSuccEdge(BL_Edge* edge) {_succEdges.push_back(edge);}
	bool removeSuccEdge(BL_Edge* edge);

	//Iterators
	BLEdge_iter predBegin() {return _predEdges.begin();}
	BLEdge_iter predEnd() {return _predEdges.end();}
	BLEdge_iter succBegin() {return _succEdges.begin();}
	BLEdge_iter succEnd() {return _succEdges.end();}
	
private:
	llvm::BasicBlock* _basicBlock;
	BLEdgeVector _predEdges;
	BLEdgeVector _succEdges;
	unsigned _numberPaths;
	NodeColor _color;
	unsigned _uid;
};

class BL_Edge {
public:
	enum EdgeType { INIT, BACKEDGE, SPTREE_EDGE, CHORD };
	
	BL_Edge(BL_Node* source, BL_Node* target)
	: _source(source), _target(target), _weight(0), _edgeType(INIT) {}
	
	//Accessors
	BL_Node* getSource() const {return _source;}
	BL_Node* getTarget() const {return _target;}
	EdgeType getType() const {return _edgeType;}
	unsigned getWeight() const {return _weight;}

	//Modifiers
	void setType(EdgeType type) {_edgeType=type;}
	void setWeight(unsigned weight) {_weight=weight;}

private:
	BL_Node* _source;
	BL_Node* _target;
	unsigned _weight;
	EdgeType _edgeType;
};

class BL_DAG {
public:
	BL_DAG(Loop* loop, unsigned loopid, InstrumentPass* parent)
	: _entry(NULL), _exit(NULL), _loop(loop), _loopid(loopid), _npath(-1) {
		_function = _loop->getHeader()->getParent();
		_module = _function->getParent();
		_blocks = _loop->getBlocks();
		_parent = parent;
	}
	//remember to release all memory
	~BL_DAG();
	
	//High-Level API
	void build();
	void insert_pseudoexit();
	void topological_sort();
	void calculatePathNumbers();
	void calculateRouteInfo();
	void instrumentation();

	//Accessors
	BL_Node* getEntry() const {return _entry;}
	BL_Node* getExit() const {return _exit;}
	Loop* getLoop() const {return _loop;}
	Module* getModule() const {return _module;}
	unsigned getLoopID() const {return _loopid;}
	int getNumberOfPaths() const {return _npath;}
	void printInfo(bool);

	//Iterators (valid only after topological sort)
	BLNode_iter begin() {return _nodes.begin();}
	BLNode_iter end() {return _nodes.end();}
	BLNode_riter rbegin() {return _nodes.rbegin();}
	BLNode_riter rend() {return _nodes.rend();}

private:
	void topologicalSortUtil(BL_Node* node, std::map<BL_Node*,bool>& visited, BLNodeStack &Stack);
	void printAllPaths(BL_Node* s, BL_Node* d);
	void printAllPathsUtil(BL_Node* u, BL_Node* d, std::map<BL_Node*,bool> visited, std::vector<std::string> path, unsigned);
	void traverseRouteUtil(BL_Node* node, std::map<BL_Node*,bool>& visited, unsigned path_val);
	BL_Edge* existEdge(BL_Node*, BL_Node*);
	BL_Node* createNode(BasicBlock* BB) {
		BL_Node* node = new BL_Node(BB);
		return node;
	}
	BL_Edge* createEdge(BL_Node* source, BL_Node* target) {
		BL_Edge* edge = new BL_Edge(source,target);
		return edge;
	}
	void addNode(BL_Node* node) {
		_nodes.push_back(node);
	}
	void addEdge(BL_Edge* edge) {
		BL_Node* source = edge->getSource();
		BL_Node* target = edge->getTarget();
		source->addSuccEdge(edge);
		target->addPredEdge(edge);
		_edges.push_back(edge);
	}
	void buildNode(BLBlockNodeMap& inDag, BLNodeStack& dfsStack);
	void buildEdge(BLBlockNodeMap& inDag, BLNodeStack& dfsStack, BL_Node* currentNode, BasicBlock* succBB);
	BLNodeVector _nodes;
	BLEdgeVector _edges;
	BLBlockNodeMap _indag;
	std::vector<BasicBlock*> _blocks;
	BL_Node* _entry;
	BL_Node* _exit;
	Loop* _loop;
	Function* _function;
	Module* _module;
	InstrumentPass* _parent;
	unsigned _loopid;
	int _npath;
};

inline bool operator==(const BL_Edge& lhs, const BL_Edge& rhs) {
	return (lhs.getSource()->getUID()==rhs.getSource()->getUID()&&
		lhs.getTarget()->getUID()==rhs.getTarget()->getUID());
}
inline bool operator!=(const BL_Edge& lhs, const BL_Edge& rhs) {
	return !(rhs==rhs);
}

}

#endif
