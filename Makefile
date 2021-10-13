
# conditionally enables sanitizers
EXTRAARGS=$(if $(SANITIZER), -DCMAKE_CXX_FLAGS="-fsanitize=$(SANITIZER)" -DCMAKE_EXE_LINKER_FLAGS="-fsanitize=$(SANITIZER)", )

.PHONY: clean build_test exec_test test

clean:
	rm -rf ./build

build_test:
	cmake -Bbuild $(EXTRAARGS)
	make -Cbuild -j

exec_test: build_test
	cd build && ctest

test: exec_test
