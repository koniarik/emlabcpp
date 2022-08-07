
# conditionally enables sanitizers
CXX_FLAGS = $(if $(SANITIZER), -fsanitize=$(SANITIZER) )  -O0 -fno-inline -g  -DEMLABCPP_LOGGING_ENABLED=ON
LINKER_FLAGS = $(if $(SANITIZER), -fsanitize=$(SANITIZER) ) -O0 -fno-inline -g

EXTRAARGS=-DCMAKE_CXX_FLAGS="$(CXX_FLAGS)" -DCMAKE_EXE_LINKER_FLAGS="$(LINKER_FLAGS)" -DCMAKE_EXPORT_COMPILE_COMMANDS=1 -DEMLABCPP_TESTS_ENABLED=ON


.PHONY: clean build_test exec_test test

clean:
	rm -rf ./ctidy_build
	rm -rf ./build

build_test:
	cmake -Bbuild $(EXTRAARGS)
	cmake --build build

test: build_test
	cd build && ctest -T Test --output-on-failure

build_coverage:
	cmake -Bbuild $(EXTRAARGS) -DEMLABCPP_COVERAGE_ENABLED=ON
	cmake --build build

coverage: build_coverage
	cd build && ctest -T Test
	lcov -c -d build  --exclude "/usr/include/*" --exclude "*_test.cpp" -o build/emlabcpp.lcov.info
	genhtml build/emlabcpp.lcov.info -s -o coverage/ -k --legend --demangle-cpp


clang-tidy:
	cmake -Bctidy_build -DEMLABCPP_TESTS_ENABLED=ON -DCMAKE_EXPORT_COMPILE_COMMANDS=1 -DCMAKE_CXX_CLANG_TIDY=clang-tidy
	make -Cctidy_build -j

clang-format:
	find ./ -iname "*.h" -o -iname "*.cpp" | xargs clang-format -i

cmake-format:
	find ./ -iname "*CMakeLists.txt" -o -iname "*.cmake" | xargs cmake-format -i

doc:
	doxygen
