all: module

module: NormalizModule.cpp setup.py
	python setup.py build_ext --inplace
