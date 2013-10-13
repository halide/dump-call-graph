#ifndef DUMPCALLGRAPH_H
#define DUMPCALLGRAPH_H

#include <Halide.h>

void dump_call_graph(const std::string& outfilename, Halide::Func root);

#endif /* end of include guard: DUMPCALLGRAPH_H */
