.PHONY: all
all: tests examples

.PHONY: tests
tests:
	$(MAKE) -C tests

.PHONY: examples
examples:
	$(MAKE) -C examples

.PHONY: test
test: tests
	python ./runtest.py

.PHONY: clean
clean: 
	$(MAKE) -C tests clean
	$(MAKE) -C examples clean
