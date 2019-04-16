import PyNormaliz_cpp
C = PyNormaliz_cpp.NmzCone(number_field=[ "a2-2", "a", "1.4+/-0.1" ] ,cone=[[[1],[0,1]],[[1],[-1]]])
PyNormaliz_cpp.NmzCompute( C, [ "SupportHyperplanes" ] )
PyNormaliz_cpp.NmzResult( C, "ExtremeRays" )

def rat_handler(list):
    return list[0]/list[1]

PyNormaliz_cpp.NmzResult( C, "ExtremeRays", RationalHandler=rat_handler, NumberfieldElementHandler=tuple, VectorHandler=tuple, MatrixHandler=tuple )

C = PyNormaliz_cpp.NmzCone(number_field=[ "a2-2", "a", "1.4+/-0.1" ] ,cone=[["1","a"],["1","-1"]])
PyNormaliz_cpp.NmzResult( C, "SupportHyperplanes" )
