#include "llvm/IR/Function.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/Pass.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/IR/PassManager.h"
#include "llvm/Transforms/IPO/PassManagerBuilder.h"
using namespace llvm;

// Note: Make sure this is NOT inside any namespace
struct InstCnt : public FunctionPass {
  static char ID;
  InstCnt() : FunctionPass(ID) {}

  bool runOnFunction(Function &F) override {
    for (auto &BB : F) {
      unsigned numInst = 0;
      for (auto &I : BB) {
        numInst++;
      }
      errs() << numInst << "\n";
    }
    return false;
  }
};

char InstCnt::ID = 0;

static RegisterPass<InstCnt> X("instcnt", "Per block instruction count");

static RegisterStandardPasses Y(
    PassManagerBuilder::EP_EarlyAsPossible,
    [](const PassManagerBuilder &Builder, legacy::PassManagerBase &PM) {
      PM.add(new InstCnt());
    });
