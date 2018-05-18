#include "llvm/Pass.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Instruction.h"
#include "llvm/Support/raw_ostream.h"
#include <map>

using namespace llvm;

namespace {
    struct CountStaticInst: public FunctionPass {
        static char ID;
        CountStaticInst(): FunctionPass(ID) {}

        bool runOnFunction(Function &F) override {
            std::map<const char*, unsigned int> countNameMap;
            for (BasicBlock &BB : F) {
                for (Instruction &I : BB) {
                    const char* opcodeName = I.getOpcodeName();
                    if(countNameMap.find(opcodeName) != countNameMap.end()) {
                        countNameMap[opcodeName] += 1;
                    }
                    else{
                        countNameMap[opcodeName] = 1;
                    }
                }
            }

            for (auto iter = countNameMap.begin(); iter != countNameMap.end(); iter++) {
                const char* opcodeName = iter->first;
                unsigned int number = iter->second;
                errs() << opcodeName << "\t" << number << "\n";
            }

            return false;
        }
    };
}

char CountStaticInst::ID = 0;
static RegisterPass<CountStaticInst> X("cse231-csi", "Count Static Instructions", false, false);
