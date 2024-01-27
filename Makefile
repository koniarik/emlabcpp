
# conditionally enables sanitizers
CXX_FLAGS = $(if $(SANITIZER), -fsanitize=$(SANITIZER) )  -O0 -fno-inline -g
LINKER_FLAGS = $(if $(SANITIZER), -fsanitize=$(SANITIZER) ) -O0 -fno-inline -g
GENERATOR:=$(shell if [ -x "$$(which ninja)" ]; then echo "Ninja"; else echo "Unix Makefiles"; fi)

EXTRAARGS=-DCMAKE_CXX_FLAGS="$(CXX_FLAGS)" -DCMAKE_EXE_LINKER_FLAGS="$(LINKER_FLAGS)" -DCMAKE_EXPORT_COMPILE_COMMANDS=1 -DEMLABCPP_TESTS_ENABLED=ON -G "$(GENERATOR)"


.PHONY: clean build_test exec_test test

test: build_test
	cd build/norm && ctest -T Test --output-on-failure

clean:
	rm -rf ./build

configure:
	cmake -Bbuild/norm $(EXTRAARGS)

build_test: configure
	cmake --build build/norm

build_coverage:
	cmake -Bbuild/cov $(EXTRAARGS) -DEMLABCPP_COVERAGE_ENABLED=ON
	cmake --build build/cov

run_coverage: build_coverage
	cd build/cov && ctest -T Test

coverage: run_coverage
	gcovr --decisions --calls -p --html-details -o build/cov/index.html -r .

clang-tidy: configure
	find src/ include/ \( -iname "*.hpp" -or -iname "*.cpp" \) -print0 | parallel -0 clang-tidy -p build/norm {}

clang-format:
	find ./ \( -iname "*.h" -o -iname "*.cpp" \) | xargs clang-format -i

cmake-format:
	find ./ -iname "*CMakeLists.txt" -o -iname "*.cmake" | xargs cmake-format -i

doc:
	doxygen
