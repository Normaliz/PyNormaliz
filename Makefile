PYTHON ?= python
PIP = $(PYTHON) -m pip -v

install: configure
	$(PIP) install --no-index --ignore-installed --no-deps .

clean:
	rm PyNormaliz*.so

configure: configure.ac
	autoconf

dist: configure
	$(PYTHON) setup.py sdist
