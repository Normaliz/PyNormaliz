import PyNormaliz_cpp

ineq = [ [ 0,0,1 ], [ 0,1,0 ],[1,0,0] ];

cone = PyNormaliz_cpp.NmzCone(["cone", ineq ])

PyNormaliz_cpp.NmzResult( cone, "HilbertBasis" )

cone2 = PyNormaliz_cpp.NmzResult( cone, "IntegerHull" )

PyNormaliz_cpp.NmzResult( cone2, "HilbertBasis" )
