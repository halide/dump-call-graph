// TODO: refactor in terms of find_direct_calls/find_transitive_calls
//       Challenge: this dump requires distinguishing calls in each update from those in the pure definition (I think?)

// TODO: docs
// TODO: calls in reduction index terms/bounds

// NOTE: this should be built with -fno-rtti to be sure it links successfully
// with the corresponding types (IRVisitor, etc.) in libHalide.a as it is
// usually compiled.

#include "DumpCallGraph.h"

#include <map>
#include <cstdio>

using std::map;
using std::string;
using namespace Halide;
using namespace Halide::Internal;

/* Find all the internal halide calls in an expr */
class FindAllCalls : public IRVisitor {
private:
    bool recursive;
public:
    FindAllCalls(bool recurse = false) : recursive(recurse) {}

    map<string, Function> calls;

    typedef map<string, Function>::iterator iterator;

    using IRVisitor::visit;

    void include_function(Function f) {
        iterator iter = calls.find(f.name());
        if (iter == calls.end()) {
            calls[f.name()] = f;
            if (recursive) {
                // recursively add everything called in the definition of f
                for (size_t i = 0; i < f.values().size(); i++) {
                    f.values()[i].accept(this);
                }
                // recursively add everything called in the definition of f's update step

                for(size_t i = 0; i < f.updates().size(); i++) {
                    for (size_t j = 0; j < f.updates()[i].values.size(); j++) {
                        f.updates()[i].values[j].accept(this);
                    }
                }
            }
        } else {
            assert(iter->second.same_as(f) &&
                   "Can't compile a pipeline using multiple functions with same name");
        }
    }

    void visit(const Call *call) {
        IRVisitor::visit(call);
        if (call->call_type == Call::Halide) {
            include_function(call->func);
        }
    }

    void dump_calls(FILE *of) {
        iterator it = calls.begin();
        while (it != calls.end()) {
            fprintf(of, "\"%s\"", it->first.c_str());
            ++it;
            if (it != calls.end()) {
                fprintf(of, ", ");
            }
        }
    }
};

void dump_function(FILE *of, const std::string name, const Function &f) {
    fprintf(of, " {\"name\": \"%s\", ", name.c_str());
    fprintf(of, "\"vars\": [");
    for (size_t i = 0; i < f.args().size(); i++) {
        fprintf(of, "\"%s\"", f.args()[i].c_str());
        if (i < f.args().size()-1) {
            fprintf(of, ", ");
        }
    }
    fprintf(of, "], ");

    fprintf(of, "\"calls\": [");
    FindAllCalls local_calls(false);
    for (size_t i = 0; i < f.values().size(); i++) {
        f.values()[i].accept(&local_calls);
    }
    local_calls.dump_calls(of);
    fprintf(of, "], ");

    // don't log reduction_value calls - these can't be meaningfully scheduled wrt. this function
    fprintf(of, "\"update_calls\": [");
    FindAllCalls update_calls(false);

    for(size_t i = 0; i < f.updates().size(); i++) {
        for (size_t j = 0; j < f.updates()[i].values.size(); j++) {
            f.updates()[i].values[j].accept(&update_calls);
        }
    }
    update_calls.dump_calls(of);
    fprintf(of, "]}");
}

void dump_call_graph(const std::string &outfilename, Func root) {
    FILE *of = fopen(outfilename.c_str(), "w");

    const Function &f = root.function();

    FindAllCalls all_calls(true);
    for (size_t i = 0; i < f.values().size(); i++) {
        f.values()[i].accept(&all_calls);
    }

    fprintf(of, "[\n");

    FindAllCalls::iterator it = all_calls.calls.begin();
    while (it != all_calls.calls.end()) {
        dump_function(of, it->first, it->second);
        fprintf(of, ",\n");

        ++it;
    }
    // dump the root function, too:
    dump_function(of, root.name(), f);

    fprintf(of, "\n]\n");
    fclose(of);
}
