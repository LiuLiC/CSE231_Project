#include "231DFA.h"
#include "llvm/Pass.h"
#include "llvm/Support/raw_ostream.h"
#include <string.h>

using namespace std;
using namespace llvm;

class MayPointToInfo: public Info {
prviate:
  map<pair<char, unsigned>, set<unsigned>> infoMap;

public:
  MayPointToInfo() {}
  MayPointToInfo(map<pair<char, unsigned>, set<unsigned>> m) { infoMap = m; }

  map<pair<char, unsigned>, set<unsigned>> getInfoMap(){
    return infoMap;
  }

  void setInfoMap(map<pair<char, unsigned>, set<unsigned>> m){
    infoMap = m;
  }

  void print() {
    for (auto ri = infoMap.begin(); ri != infoMap.end(); ++ri) {
      if (ri->second.size() == 0)
        continue;

      errs() << (char) toupper(ri->first.first) << ri->first.second << "->(";

      for (auto mi = ri->second.begin(); mi != ri->second.end(); ++mi) {
        errs() << "M" << *mi << "/";
      }
      errs() << ")|";
    }
    errs() << "\n";
  }

  static bool equals(MayPointToInfo* info1, MayPointToInfo* info2){
    return info1->getInfoMap() == info2->getInfoMap();
  }

  static MayPointToInfo* join(MayPointToInfo * info1, MayPointToInfo * info2, MayPointToInfo * result) {
	  map<pair<char, unsigned>, set<unsigned>> info1Map = info1->getInfoMap();
		map<pair<char, unsigned>, set<unsigned>> info2Map = info2->getInfoMap();

		for (auto it = info2Map.begin(); it != info2Map.end(); ++it) {
			info1Map[make_pair(it->first.first, it->first.second)].insert(it->second.begin(), it->second.end());
		}

		result->setInfoMap(info1Map);

		return result;
	}

};

