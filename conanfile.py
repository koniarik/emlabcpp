from conan import ConanFile
from conan.tools.cmake import CMakeToolchain, CMake, cmake_layout
from conan.tools.build import check_min_cppstd


class emlabcppRecipe(ConanFile):
    name = "emlabcpp"
    version = "1.7"

    license = "MIT"
    author = "Veverak squirrelcze@gmail.com"
    url = "https://github.com/koniarik/emlabcpp"
    topics = ("embedded","utilities")

    settings = "os", "compiler", "build_type", "arch"

    exports_sources = "CMakeLists.txt", "src/*", "include/*", "cmake/*", "tests/*", "examples/*"

    def validate(self):
        check_min_cppstd(self, "20")

    def requirements(self):
        self.test_requires("gtest/1.14.0")

    def layout(self):
        cmake_layout(self)

    def generate(self):
        tc = CMakeToolchain(self)
        tc.generate()

    def build(self):
        cmake = CMake(self)
        cmake.configure()
        cmake.build()
        if not self.conf.get("tools.build:skip_test", default=False):
            cmake.test()

    def package(self):
        cmake = CMake(self)
        cmake.install()

    def package_info(self):
        self.cpp_info.libs = ["emlabcpp"]
