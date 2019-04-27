PYTHON ?= python
PIP = $(PYTHON) -m pip -v

install: configure
	$(PIP) install --no-index --ignore-installed --no-deps .

clean:
	rm -rf build/
	rm -f PyNormaliz*.so
	rm -f *.pyc
	rm -f configure
	rm -f config.log
	rm -f config.py

configure: configure.ac
	autoconf

sdist: configure
	$(PYTHON) setup.py sdist
