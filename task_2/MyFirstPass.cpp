#include "llvm/IR/Function.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/Pass.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/IR/PassManager.h"
#include "llvm/Transforms/IPO/PassManagerBuilder.h"
using namespace llvm;

// Note: Make sure this is NOT inside any namespace
struct MyFirstPass : public FunctionPass {
  static char ID;
  MyFirstPass() : FunctionPass(ID) {}

  bool runOnFunction(Function &F) override {
    errs() << "MyFirstPass: Processing function: " << F.getName() << "\n";
    
    unsigned numBB = 0;
    unsigned numInst = 0;
    
    for (auto &BB : F) {
      numBB++;
      for (auto &I : BB) {
        numInst++;
      }
    }
    
    errs() << "  Number of basic blocks: " << numBB << "\n";
    errs() << "  Number of instructions: " << numInst << "\n";
    
    return false;
  }
};

// Initialize the pass ID (outside any namespace)
char MyFirstPass::ID = 0;

// Register the pass - this MUST be outside any namespace
static RegisterPass<MyFirstPass> X("myfirstpass", "My First Pass");

// Register for opt automatic inclusion
static RegisterStandardPasses Y(
    PassManagerBuilder::EP_EarlyAsPossible,
    [](const PassManagerBuilder &Builder, legacy::PassManagerBase &PM) {
      PM.add(new MyFirstPass());
    });
