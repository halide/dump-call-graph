# Building
`DumpCallGraph.[cpp,h]` can be directly imported into an outside project to provide their functionality. `DumpCallGraph.cpp` should usually be compiled with `-fno-rtti` to be sure it links successfully with the corresponding types (`IRVisitor`, etc.) in `libHalide.a` as Halide is usually compiled by its default Mac/Linux Makefile.
