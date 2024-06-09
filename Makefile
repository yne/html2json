TESTS_REF := $(wildcard tests/*.json)
TESTS_RES := $(patsubst %.json, %.res, $(TESTS_REF))
all:html2json
tests/%.res: tests/% tests/%.json html2json
	./html2json < $< | jq | diff - $<.json
test:$(TESTS_RES)
.PHONY: all test