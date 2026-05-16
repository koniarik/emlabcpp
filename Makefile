
PRESET ?= debug

.PHONY: all test clean clang-tidy clang-format cmake-format conan doc

all:
	cmake --workflow --preset $(PRESET)

test:
	cmake --workflow --preset $(PRESET)

clean:
	rm -rf ./build

configure:
	cmake --preset $(PRESET)

build:
	cmake --build --preset $(PRESET)

clang-tidy: configure
	find src/ include/ \( -iname "*.hpp" -or -iname "*.cpp" \) -print0 | parallel -0 clang-tidy -p build {}

clang-format:
	find ./ \( -iname "*.h" -o -iname "*.cpp" \) | xargs clang-format -i

cmake-format:
	find ./ -iname "*CMakeLists.txt" -o -iname "*.cmake" | xargs cmake-format -i

conan:
	conan build . --build=missing

doc:
	doxygen
