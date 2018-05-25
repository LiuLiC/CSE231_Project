#include "231DFA.h"
#include "llvm/Pass.h"
#include "llvm/Support/raw_ostream.h"
#include <string.h>

using namespace llvm;
using namespace std;

class ReachingInfo: public Info {
private:
  set<unsigned> infoSet;

public:
  ReachingInfo() {}
  ReachingInfo(set<unsigned> s) {
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

  static bool equals(ReachingInfo* info1, ReachingInfo* info2){
    return info1->getInfoSet() == info2->getInfoSet();
  }

  static ReachingInfo* join(ReachingInfo* info1, ReachingInfo* info2, ReachingInfo* result){
    set<unsigned> info1Set = info1->getInfoSet(), info2Set = info2->getInfoSet();

    for(auto it = info2Set.begin(), end = info2Set.end(); it != end; it++){
      info1Set.insert(*it);
    }

    result->setInfoSet(info1Set);

    return result;
  }

};

class ReachingDefinitionAnalysis: public DataFlowAnalysis<ReachingInfo, true> {
private:
  typedef pair<unsigned, unsigned> Edge;
  map<Edge, ReachingInfo*> EdgeToInfo;
  map<string, int> opTocategory = {{"br", 2}, {"switch", 2}, {"alloca", 1}, {"load", 1},\
                                   {"store", 2}, {"getelementptr", 1}, {"icmp", 1}, {"fcmp", 1},\
                                   {"phi", 3}, {"select", 1}
                                  };

public:
  ReachingDefinitionAnalysis(ReachingInfo & bottom, ReachingInfo & initialState):\
    DataFlowAnalysis(bottom, initialState) {}

    void flowfunction(Instruction * I,
                      vector<unsigned> & IncomingEdges,
                      vector<unsigned> & OutgoingEdges,
                      vector<ReachingInfo *> & Infos){

      string opName = I->getOpcodeName();
      int category = opTocategory.count(opName) ? opTocategory[opName] : 2;
      category = I->isBinaryOp() ? 1 : category;

      unsigned instrIdx = getInstrToIndex()[I];
      map<Edge, ReachingInfo*> edgeToInfo = getEdgeToInfo();
      ReachingInfo* info = new ReachingInfo();

      for (size_t i = 0; i < IncomingEdges.size(); i++) {
        Edge inEdge = make_pair(IncomingEdges[i], instrIdx);
        ReachingInfo::join(info, edgeToInfo[inEdge], info);
      }

      if(category == 1){
        set<unsigned> current = {instrIdx};
        ReachingInfo::join(info, new ReachingInfo(current), info);
      }
      else if(category == 3){
        Instruction* firstNonPhi = I->getParent()->getFirstNonPHI();
        unsigned firstNonPhiIdx = getInstrToIndex()[firstNonPhi];
        set<unsigned> current;

        for (size_t i = instrIdx; i < firstNonPhiIdx; i++) {
          current.insert(i);
        }

        ReachingInfo::join(info, new ReachingInfo(current), info);
      }

      for (size_t i = 0; i < OutgoingEdges.size(); i++) {
        Infos.push_back(info);
      }
    }
};

namespace {
  struct ReachingDefinitionAnalysisPass: public FunctionPass {
    static char ID;

    ReachingDefinitionAnalysisPass() : FunctionPass(ID) {}
    bool runOnFunction(Function &F) override {
      ReachingInfo bottom, initialState;
      ReachingDefinitionAnalysis* RDA = new ReachingDefinitionAnalysis(bottom, initialState);

      RDA->runWorklistAlgorithm(&F);
      RDA->print();

      return false;
    }
  };
}

char ReachingDefinitionAnalysisPass::ID = 0;
static RegisterPass<ReachingDefinitionAnalysisPass> X("cse231-reaching", "ReachingDefinitionAnalysis", false, false);
