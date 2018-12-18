#include "231DFA.h"
#include "llvm/Pass.h"
#include "llvm/Support/raw_ostream.h"
#include <string.h>

using namespace std;
using namespace llvm;

class LivenessInfo: public Info {
private:
  set<unsigned> infoSet;

public:
  LivenessInfo() {}
  LivenessInfo(set<unsigned> s) {
    infoSet = s;
  }

  set<unsigned> getInfoSet(){
    return infoSet;
  }

  void setInfoSet(set<unsigned> s){
    infoSet = s;
  }

  void print(){
    for(auto it = infoSet.begin(), end = infoSet.end(); it != end; it++){
      errs() << *it << "|";
    }
    errs() << '\n';
  }

  static bool equals(LivenessInfo* info1, LivenessInfo* info2){
    return info1->getInfoSet() == info2->getInfoSet();
  }

  static LivenessInfo* join(LivenessInfo* info1, LivenessInfo* info2, LivenessInfo* result){
    set<unsigned> info1Set = info1->getInfoSet(), info2Set = info2->getInfoSet();

    for(auto it = info2Set.begin(), end = info2Set.end(); it != end; it++){
      info1Set.insert(*it);
    }

    result->setInfoSet(info1Set);

    return result;
  }

  static LivenessInfo* remove(LivenessInfo* info1, LivenessInfo* info2, LivenessInfo* result) {
		set<unsigned> info1Set = info1->getInfoSet(), info2Set = info2->getInfoSet();

		for (auto it = info2Set.begin(), end = info2Set.end(); it != end; it++) {
			info1Set.erase(*it);
		}

		result->setInfoSet(info1Set);

		return result;
	}

};

class LivenessAnalysis: public DataFlowAnalysis<LivenessInfo, false> {
private:
  typedef pair<unsigned, unsigned> Edge;
  map<string, int> opTocategory = {{"br", 2}, {"switch", 2}, {"alloca", 1}, {"load", 1},\
                                   {"store", 2}, {"getelementptr", 1}, {"icmp", 1}, {"fcmp", 1},\
                                   {"phi", 3}, {"select", 1}
                                  };

public:
  LivenessAnalysis(LivenessInfo & bottom, LivenessInfo & initialState):
    DataFlowAnalysis(bottom, initialState) {}
  ~LivenessAnalysis() {}

  void flowfunction(Instruction * I,
                      vector<unsigned> & IncomingEdges,
                      vector<unsigned> & OutgoingEdges,
                      vector<LivenessInfo *> & Infos){

    string opName = I->getOpcodeName();
    int category = opTocategory.count(opName) ? opTocategory[opName] : 2;
    category = I->isBinaryOp() ? 1 : category;

    unsigned instrIdx = getInstrToIndex()[I];
    map<Edge, LivenessInfo*> edgeToInfo = getEdgeToInfo();
    map<Instruction *, unsigned> instrToIndex = getInstrToIndex();
		map<unsigned, Instruction *> indexToInstr = getIndexToInstr();
    LivenessInfo* outInfo = new LivenessInfo();
    set<unsigned> operands;

    for(unsigned i = 0; i < I->getNumOperands(); i++){
      Instruction* instr = (Instruction*)I->getOperand(i);

      if(instrToIndex.count(instr) != 0)
        operands.insert(instrToIndex[instr]);
    }

    for (size_t i = 0; i < IncomingEdges.size(); i++) {
        Edge inEdge = make_pair(IncomingEdges[i], instrIdx);
        LivenessInfo::join(outInfo, edgeToInfo[inEdge], outInfo);
    }

    if(category == 1){
      set<unsigned> index = {instrIdx};

      LivenessInfo::join(outInfo, new LivenessInfo(operands), outInfo);
      LivenessInfo::remove(outInfo, new LivenessInfo(index), outInfo);

      for (size_t i = 0; i < OutgoingEdges.size(); i++) {
        Infos.push_back(outInfo);
      }
    }
    else if(category == 2){
      LivenessInfo::join(outInfo, new LivenessInfo(operands), outInfo);

      for (size_t i = 0; i < OutgoingEdges.size(); i++) {
        Infos.push_back(outInfo);
      }
    }
    else{
      set<unsigned> index;
      Instruction* firstNonPhi = I->getParent()->getFirstNonPHI();
      unsigned firstNonPhiIdx = instrToIndex[firstNonPhi];

      for(unsigned i = instrIdx; i < firstNonPhiIdx; i++)
        index.insert(i);
      
      LivenessInfo::remove(outInfo, new LivenessInfo(index), outInfo);

      for(unsigned k = 0; k < OutgoingEdges.size(); k++){
        set<unsigned> operand;
        LivenessInfo* out_k = new LivenessInfo();

        for(unsigned i = instrIdx; i < firstNonPhiIdx; i++){
          Instruction* instr = (Instruction*)indexToInstr[i];

          for(unsigned j = 0; j < instr->getNumOperands(); j++){
            Instruction* opInstr = (Instruction*)I->getOperand(j);

            if(instrToIndex.count(opInstr) == 0)
              continue;
            
            if(opInstr->getParent() == indexToInstr[OutgoingEdges[k]]->getParent())
              operand.insert(instrToIndex[opInstr]);
          }

          LivenessInfo::join(outInfo, new LivenessInfo(operand), out_k);
          Infos.push_back(out_k);
        } 
      }
    }
  }
};

namespace {
  struct LivenessAnalysisPass : public FunctionPass {
    static char ID;
    LivenessAnalysisPass() : FunctionPass(ID) {}

    bool runOnFunction(Function &F) override {
      LivenessInfo bottom;
      LivenessInfo initialState;
      LivenessAnalysis * rda = new LivenessAnalysis(bottom, initialState);

      rda->runWorklistAlgorithm(&F);
      rda->print();

      return false;
    }
  }; 
} 

char LivenessAnalysisPass::ID = 0;
static RegisterPass<LivenessAnalysisPass> X("cse231-liveness", "Do liveness analysis on DFA CFG", false, false);
