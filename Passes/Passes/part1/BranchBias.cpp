#include "llvm/Pass.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Instruction.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Value.h"
#include "llvm/Support/raw_ostream.h"
#include <string.h>

using namespace llvm;
using namespace std;

namespace {
  struct BranchBias: public FunctionPass {
    static char ID;

    BranchBias() : FunctionPass(ID) {}
    bool runOnFunction(Function &F) override {
      Module* M = F.getParent();
      LLVMContext& ctx = M->getContext();

      // Functions in lib231.cpp
      Function* updateFunc = dyn_cast<Function>(M->getOrInsertFunction("updateBranchInfo",\
        Type::getVoidTy(ctx),\
        Type::getInt1Ty(ctx)));
      Function* printFunc = dyn_cast<Function>(M->getOrInsertFunction("printOutBranchInfo",\
      Type::getVoidTy(ctx)));

      for(auto itBlock = F.begin(), itBlockEnd = F.end(); itBlock != itBlockEnd; itBlock++){
        IRBuilder<> builder(&*itBlock);
        for(auto itInst = itBlock->begin(), itInstEnd = itBlock->end(); itInst != itInstEnd; itInst++){
          if(isa<BranchInst>(*itInst)){
            BranchInst *brInst = dyn_cast<BranchInst>(itInst);
            if(brInst->isConditional()){
              // errs()<<brInst->getOpcodeName()<<'\n';
              builder.SetInsertPoint(brInst);
              Value* isTaken = brInst->getCondition();
              // isTaken->getType()->print(errs());
              builder.CreateCall(updateFunc, isTaken);
            }
          }

          if((string)itInst->getOpcodeName() == "ret"){
            builder.SetInsertPoint(&*itInst);
            builder.CreateCall(printFunc);
          }
        }
      }

      return false;
    }
  };
}

char BranchBias::ID = 0;
static RegisterPass<BranchBias> X("cse231-bb", "Profiling Branch Bias", false, false);
