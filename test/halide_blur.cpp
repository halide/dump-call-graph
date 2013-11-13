#include <Halide.h>
#include "../DumpCallGraph.h"
using namespace Halide;

int main(int argc, char **argv) {

    ImageParam input(UInt(16), 2);
    Func blur_x("blur_x"), blur_y("blur_y");
    Var x("x"), y("y"), xi("xi"), yi("yi");
    
    // The algorithm
    blur_x(x, y) = (input(x, y) + input(x+1, y) + input(x+2, y))/3;
    blur_y(x, y) = (blur_x(x, y) + blur_x(x, y+1) + blur_x(x, y+2))/3;
    
    dump_call_graph("halide_blur.calls.json", blur_y);
    
    return 0;
}
