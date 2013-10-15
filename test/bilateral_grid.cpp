#include "Halide.h"
#include <stdio.h>
#include "../DumpCallGraph.h"

using namespace Halide;

int main(int argc, char **argv) {
    ImageParam input(Float(32), 2);
    Param<float> r_sigma;
    int s_sigma = 8; //atoi(argv[1]);
    Var x("x"), y("y"), z("z"), c("c");

    // Add a boundary condition
    Func clamped("clamped");
    clamped(x, y) = input(clamp(x, 0, input.width()-1),
                          clamp(y, 0, input.height()-1));

    // Construct the bilateral grid
    RDom r(0, s_sigma, 0, s_sigma);
    Expr val = clamped(x * s_sigma + r.x - s_sigma/2, y * s_sigma + r.y - s_sigma/2);
    val = clamp(val, 0.0f, 1.0f);
    Expr zi = cast<int>(val * (1.0f/r_sigma) + 0.5f);
    Func grid("grid"), histogram("histogram");
    histogram(x, y, zi, c) += select(c == 0, val, 1.0f);

    // Introduce a dummy function, so we can schedule the histogram within it
    grid(x, y, z, c) = histogram(x, y, z, c);

    // Blur the grid using a five-tap filter
    Func blurx("blurx"), blury("blury"), blurz("blurz");
    blurx(x, y, z, c) = (grid(x-2, y, z, c) +
                         grid(x-1, y, z, c)*4 +
                         grid(x  , y, z, c)*6 +
                         grid(x+1, y, z, c)*4 +
                         grid(x+2, y, z, c));
    blury(x, y, z, c) = (blurx(x, y-2, z, c) +
                         blurx(x, y-1, z, c)*4 +
                         blurx(x, y  , z, c)*6 +
                         blurx(x, y+1, z, c)*4 +
                         blurx(x, y+2, z, c));
    blurz(x, y, z, c) = (blury(x, y, z-2, c) +
                         blury(x, y, z-1, c)*4 +
                         blury(x, y, z  , c)*6 +
                         blury(x, y, z+1, c)*4 +
                         blury(x, y, z+2, c));

    // Take trilinear samples to compute the output
    val = clamp(clamped(x, y), 0.0f, 1.0f);
    Expr zv = val * (1.0f/r_sigma);
    zi = cast<int>(zv);
    Expr zf = zv - zi;
    Expr xf = cast<float>(x % s_sigma) / s_sigma;
    Expr yf = cast<float>(y % s_sigma) / s_sigma;
    Expr xi = x/s_sigma;
    Expr yi = y/s_sigma;
    Func interpolated("interpolated");
    interpolated(x, y, c) =
        lerp(lerp(lerp(blurz(xi, yi, zi, c), blurz(xi+1, yi, zi, c), xf),
                  lerp(blurz(xi, yi+1, zi, c), blurz(xi+1, yi+1, zi, c), xf), yf),
             lerp(lerp(blurz(xi, yi, zi+1, c), blurz(xi+1, yi, zi+1, c), xf),
                  lerp(blurz(xi, yi+1, zi+1, c), blurz(xi+1, yi+1, zi+1, c), xf), yf), zf);

    // Normalize
    Func bilateral_grid("bilateral_grid");
    bilateral_grid(x, y) = interpolated(x, y, 0)/interpolated(x, y, 1);

    dump_call_graph("bilateral_grid.calls.json", bilateral_grid);

    return 0;
}



