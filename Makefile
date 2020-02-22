
build_tests:
	cmake -Bbuild tests/
	make -Cbuild -j

exec_tests: build_tests
	cd build && ctest

tests: exec_tests
