#pragma once

#include <llvm/IR/Function.h>
#include <llvm/Support/raw_ostream.h>

// A skeleton for printing your analysis results.

// Instantiate AnalysisType with the type of your dataflow analysis, which should provide two const public methods getInState() and getOutState(). Both methods should accept a const BasicBlock* as an argument and returns a const reference to a container that represents the IN/OUT set of the given basic block.

// Iteration order doesn't matter.

// Print each function with something like this:
//      DataFlowAnnotator<YourAnalysisType> annotator(errs());
//      annotator.print(function);

// Do not add any additional information during this print; I will be parsing it. Also in general comment out your debug output. Ideally the only output of your pass would be the annotated functions as above.

// Of course, if you can write your own annotator that behaves exactly like this one and is eaiser for you to use, feel free to do that.

namespace example
{

template <typename AnalysisType>
class DataFlowAnnotator
{
private:
	AnalysisType& analysis;
	llvm::raw_ostream& os;

	template <typename StateType>
	void printState(const StateType& state) const
	{
		os << "{ ";
		for (auto v: state)
			os << v->getName() << " ";
		os << "}";
	}

	void emitBasicBlockStartAnnot(const llvm::BasicBlock& bb)
	{
		printState(analysis.getInState(&bb));
	}

	void emitBasicBlockEndAnnot(const llvm::BasicBlock& bb)
	{
		printState(analysis.getOutState(&bb));
		os << "\n---\n";
	}
public:
	DataFlowAnnotator(AnalysisType& a, llvm::raw_ostream& o): analysis(a), os(o) {}

	void print(const llvm::Function& f)
	{
		os << "\n******  DataFlow Result for function " << f.getName() << "  ******\n";
		for (auto const& bb: f)
		{
			emitBasicBlockStartAnnot(bb);
			os << bb;
			emitBasicBlockEndAnnot(bb);
		}
	}
};

}
