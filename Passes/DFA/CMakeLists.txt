add_llvm_loadable_module( CSE231DFA
  ReachingDefinitionAnalysis.cpp
  LivenessAnalysis.cpp
  MayPointToAnalysis.cpp
  
  PLUGIN_TOOL
  opt
  )
