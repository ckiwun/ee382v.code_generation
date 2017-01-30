#include "IRPrinter.h"

#include <llvm/IR/Module.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/DebugInfo.h>
#include <llvm/IR/GlobalValue.h>
#include <llvm/IR/AssemblyAnnotationWriter.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/Support/FormattedStream.h>

using namespace llvm;
using namespace ee382v;

bool IRPrinter::runOnModule(Module& M)
{
	// Remove the following line and before you write down your own codes
	//M.print(outs(), nullptr);
	int fc,bbc;
	for(Module::iterator F=M.begin(),FE=M.end();F!=FE;F++,fc++)
	{
		//F->print(outs(), nullptr);
		if(F==M.begin()) fc=0;
		if(F->isDeclaration()) outs() << "declare "; else outs() << "define ";
		outs() << *F->getReturnType() << " @" << F->getName() << " (";
		auto pref = "";
		for(auto& arg : F->getArgumentList()) { outs() << pref << arg; pref = ", ";}
		outs() << ") #" << fc << "\n";

		for(Function::iterator bb=F->begin(),bbe=F->end();bb!=bbe;bb++,bbc++)
		{
			outs() << bb->getName() << "\n";
			for(BasicBlock::iterator i=bb->begin(),ie=bb->end();i!=ie;i++)
			{
				if (DILocation *deb = i->getDebugLoc())
				{
					outs() << "  line " << deb->getLine() << ":" << *i<< "\n";
				}
				else
				{
					outs() << *i<< "\n";
				}
			}
		}
	}

	return false;
}

void IRPrinter::getAnalysisUsage(AnalysisUsage &AU) const
{
	AU.setPreservesAll();
}

// LLVM uses the address of this static member to identify the pass, so the
// initialization value is unimportant.
char IRPrinter::ID = 0;

// Register the pass name to allow it to be called with opt:
// See http://llvm.org/releases/3.9.1/docs/WritingAnLLVMPass.html#running-a-pass-with-opt for more info.
static RegisterPass<IRPrinter> X("printIR", "IR + Source line number printer");
