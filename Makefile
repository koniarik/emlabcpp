# https://stackoverflow.com/questions/2483182/recursive-wildcards-in-gnu-make/18258352#18258352
rwildcard=$(foreach d,$(wildcard $(1:=/*)),$(call rwildcard,$d,$2) $(filter $(subst *,%,$2),$d))

# conditionally enables sanitizers
EXTRAARGS=$(if $(SANITIZER), -DCMAKE_CXX_FLAGS="-fsanitize=$(SANITIZER)" -DCMAKE_EXE_LINKER_FLAGS="-fsanitize=$(SANITIZER)", )

HEADERS=$(call rwildcard, include, *.h)

FORMAT_TARGETS = $(addsuffix .format, $(HEADERS))

.PHONY: clean build_test exec_test test format $(FORMAT_TARGETS)

clean:
	rm -rf ./build

build_test:
	cmake -Bbuild ./tests/ $(EXTRAARGS)
	make -Cbuild -j

exec_test: build_test
	cd build && ctest

test: exec_test

single_header: 
	quom include/main.h emlabcpp.h

format: $(FORMAT_TARGETS)
	
$(FORMAT_TARGETS):
	clang-format -i $(basename $@)
