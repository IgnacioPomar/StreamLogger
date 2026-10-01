import os
import re

from conan import ConanFile
from conan.errors import ConanException
from conan.tools.build import check_min_cppstd
from conan.tools.cmake import CMake, CMakeDeps, CMakeToolchain, cmake_layout
from conan.tools.files import load


class StreamLoggerConan(ConanFile):
    name = "streamlogger"
    license = "Unlicense"
    description = "Modern C++ logger library, with event retrieval and color support"

    settings = "os", "compiler", "build_type", "arch"
    options = {
        "shared": [True, False],
        "fPIC": [True, False],
        "build_tests": [True, False],
    }
    default_options = {
        "shared": True,
        "fPIC": True,
        "build_tests": False,
    }

    exports_sources = "CMakeLists.txt", "include/*", "src/*", "test/*", "examples/*"

    def set_version(self):
        # Single source of truth: project(StreamLogger VERSION x.y.z) in CMakeLists.txt
        cmakelists = load(self, os.path.join(self.recipe_folder, "CMakeLists.txt"))
        match = re.search(r"project\s*\(\s*StreamLogger\s+VERSION\s+(\d+\.\d+\.\d+)", cmakelists)
        if not match:
            raise ConanException("Can not read the version from CMakeLists.txt")
        if self.version and self.version != match.group(1):
            raise ConanException(f"--version={self.version} does not match the CMakeLists.txt version {match.group(1)}")
        self.version = match.group(1)

    def config_options(self):
        if self.settings.os == "Windows":
            del self.options.fPIC

    def configure(self):
        if self.options.shared:
            self.options.rm_safe("fPIC")

    def package_id(self):
        # Building the tests does not change the packaged library
        del self.info.options.build_tests

    def validate(self):
        check_min_cppstd(self, 20)

    def build_requirements(self):
        if self.options.build_tests:
            self.test_requires("catch2/3.16.0")
            self.test_requires("trompeloeil/49")

    def layout(self):
        cmake_layout(self)
        # Editable mode: consumers use the headers straight from the source tree
        self.cpp.source.includedirs = ["include"]
        self.cpp.build.includedirs = ["include"]    # Generated export header
        self.cpp.build.libdirs = ["."]

    def generate(self):
        CMakeDeps(self).generate()
        tc = CMakeToolchain(self)
        tc.cache_variables["STREAMLOGGER_BUILD_TESTS"] = bool(self.options.build_tests)
        tc.cache_variables["STREAMLOGGER_BUILD_EXAMPLES"] = bool(self.options.build_tests)
        tc.generate()

    def build(self):
        cmake = CMake(self)
        cmake.configure()
        cmake.build()
        if self.options.build_tests:
            cmake.test()

    def package(self):
        CMake(self).install()

    def package_info(self):
        self.cpp_info.libs = ["StreamLogger"]
        self.cpp_info.set_property("cmake_file_name", "StreamLogger")
        self.cpp_info.set_property("cmake_target_name", "StreamLogger::StreamLogger")
        if self.settings.os in ["Linux", "FreeBSD"]:
            self.cpp_info.system_libs = ["pthread"]
