#include "llvm/Pass.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Instruction.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/Value.h"
#include <map>
#include <vector>
#include <string.h>

using namespace llvm;
using namespace std;

namespace {
  struct CountDynamicInst : public FunctionPass {
    static char ID;

    CountDynamicInst() : FunctionPass(ID) {}
    bool runOnFunction(Function &F) override {
      Module* M = F.getParent();
      LLVMContext &ctx = M->getContext();

      // Functions in lib231.cpp
      Function* updateFunc = dyn_cast<Function>(M->getOrInsertFunction("updateInstrInfo",\
      Type::getVoidTy(ctx),\
      Type::getInt32Ty(ctx),  Type::getInt32PtrTy(ctx), Type::getInt32PtrTy(ctx)));
      Function* printFunc = dyn_cast<Function>(M->getOrInsertFunction("printOutInstrInfo",\
      Type::getVoidTy(ctx)));

      for(auto itBlock = F.begin(), itBlockEnd = F.end(); itBlock != itBlockEnd; itBlock++){
        map<int, int> opCounter;
        for(auto itInst = itBlock->begin(), itInstEnd = itBlock->end(); itInst != itInstEnd; itInst++){
          opCounter[itInst->getOpcode()]++;
        }

        IRBuilder<> builder(&*itBlock);
        builder.SetInsertPoint(itBlock->getTerminator());

        int num = opCounter.size();
        vector<Value*> args;
        vector<Constant*> keys;
        vector<Constant*> values;

        for(auto it = opCounter.begin(), end = opCounter.end(); it != end; it++){
          keys.push_back(ConstantInt::get(Type::getInt32Ty(ctx), it->first));
          values.push_back(ConstantInt::get(Type::getInt32Ty(ctx), it->second));
        }

        ArrayType* arrayType = ArrayType::get(Type::getInt32Ty(ctx), num);
        // GEP require an array
        // detail in http://llvm.org/docs/GetElementPtr.html
        Value* gepIdx[2] = {ConstantInt::get(Type::getInt32Ty(ctx), 0), ConstantInt::get(Type::getInt32Ty(ctx), 0)};

        GlobalVariable* keysG = new GlobalVariable(*M, arrayType, true, GlobalVariable::InternalLinkage, ConstantArray::get(arrayType, keys), "keysGlobal");
        GlobalVariable* valuesG = new GlobalVariable(*M, arrayType, true, GlobalVariable::InternalLinkage, ConstantArray::get(arrayType, values), "valuesGlobal");

        args.push_back(ConstantInt::get(Type::getInt32Ty(ctx), num));
        args.push_back(builder.CreateInBoundsGEP(keysG, gepIdx));
        args.push_back(builder.CreateInBoundsGEP(valuesG, gepIdx));

        builder.CreateCall(updateFunc, args);

        for(auto itInst = itBlock->begin(), itEnd = itBlock->end(); itInst != itEnd; itInst++){
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

char CountDynamicInst::ID = 0;
static RegisterPass<CountDynamicInst> X("cse231-cdi", "Count Dynamic Instructions", false, false);
