
PRESET ?= debug

.PHONY: all test clean clang-tidy clang-format cmake-format conan doc

all:
	cmake --workflow --preset $(PRESET)

test:
	cmake --workflow --preset $(PRESET)

clean:
	rm -rf ./_build

configure:
	cmake --preset $(PRESET)

build:
	cmake --build --preset $(PRESET)

clang-tidy: configure
	run-clang-tidy -p _build/$(PRESET) -quiet 'emlabcpp_verify_interface_header_sets'

clang-format:
	find ./ \( -iname "*.hpp" -o -iname "*.cpp" \) | xargs clang-format -i

cmake-format:
	find ./ -iname "*CMakeLists.txt" -o -iname "*.cmake" | xargs cmake-format -i

conan:
	conan build . --build=missing

doc:
	doxygen
