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