#ifndef EE382V_ASSIGNMENT2_EPP_H
#define EE382V_ASSIGNMENT2_EPP_H

#include "llvm/Analysis/LoopPass.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/IR/Dominators.h"

//added by me
#include "llvm/IR/BasicBlock.h"
#include <iostream>
#include <vector>
#include <map>
#include <queue>
#include <stack>
#include <unordered_set>
#include <unordered_map>

namespace ee382v
{
class BasicBlock;
class BL_Node;
class BL_Edge;
class BL_DAG;

typedef std::vector<BL_Node*> BLNodeVector;
typedef std::vector<BL_Node*>::iterator BLNode_iter;
typedef std::vector<BL_Node*>::reverse_iterator BLNode_riter;
typedef std::vector<BL_Edge*> BLEdgeVector;
typedef std::vector<BL_Edge*>::iterator BLEdge_iter;
typedef std::stack<BL_Node*> BLNodeStack;
typedef std::queue<BL_Node*> BLNodeQueue;
typedef llvm::Loop Loop;

class InstrumentPass: public llvm::LoopPass
{
public:
	static char ID;
	InstrumentPass() : llvm::LoopPass(ID) {}

	bool runOnLoop(Loop*, llvm::LPPassManager&);
	bool doFinalization();

	void getAnalysisUsage(llvm::AnalysisUsage &AU) const;

private:
	static int global_loop_id;
	std::map<int,bool> inner;

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
	BasicBlock* _basicBlock;
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
	BL_DAG(Loop* loop)
	: _entry(NULL), _exit(NULL), _loop(loop), _npath(-1) {}
	//remember to release all memory
	~BL_DAG();
	
	//High-Level API
	void build(Loop* loop);
	void topological_sort();
	void calculatePathNumbers();

	//Accessors
	BL_Node* getEntry() const {return _entry;}
	BL_Node* getExit() const {return _exit;}
	Loop* getLoop() const {return _loop;}
	unsigned getNumberOfPaths() const {return _npath;}

	//Iterators
	BLNode_iter begin() {return _nodes.begin();}
	BLNode_iter end() {return _nodes.end();}
	BLNode_riter rbegin() {return _nodes.rbegin();}
	BLNode_riter rend() {return _nodes.rend();}

private:
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
		_edges.push_back(edge);
	}

	BLNodeVector _nodes;
	BLEdgeVector _edges;
	BL_Node* _entry;
	BL_Node* _exit;
	Loop* _loop;
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
