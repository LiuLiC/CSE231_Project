#include "llvm/Pass.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Instruction.h"
#include "llvm/Support/raw_ostream.h"

using namespace llvm;

namespace {
    struct CountDynamicInst : public FunctionPass {
        static char ID;

        CountDynamicInst() : FunctionPass(ID) {}
        bool runOnFunction(Function &F) override {
          
        }
    };
}

char CountDynamicInst::ID = 0;
static RegisterPass<CountDynamicInst> X("cse231-cdi", "Count Dynamic Instructions", false, false);
