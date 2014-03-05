import sys
import json
import pygraphviz as gv

def main(args):
    assert(len(args) == 1)
    cgfile = args[0]
    print >>sys.stderr, "WARNING: does not yet process update calls, etc."
    g = gv.AGraph()
    cg = json.load(open(cgfile))
    for f in cg:
        for f2 in f['calls']:
            g.add_edge(f['name'], f2)
    print g

if __name__ == '__main__':
    main(sys.argv[1:])
