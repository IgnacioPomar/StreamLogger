from conan import ConanFile
from conan.tools.build import check_min_cppstd
from conan.tools.cmake import CMake, CMakeDeps, CMakeToolchain, cmake_layout


class StreamLoggerConan(ConanFile):
    name = "streamlogger"
    version = "0.1.0"
    license = "Unlicense"
    description = "Modern C++ logger library, with event retrieval and color support"

    settings = "os", "compiler", "build_type", "arch"
    options = {
        "shared": [True, False],
        "fPIC": [True, False],
        "build_tests": [True, False],
    }
    default_options = {
        "shared": False,
        "fPIC": True,
        "build_tests": False,
    }

    exports_sources = "CMakeLists.txt", "include/*", "src/*", "test/*", "examples/*"

    def config_options(self):
        if self.settings.os == "Windows":
            del self.options.fPIC

    def configure(self):
        if self.options.shared:
            self.options.rm_safe("fPIC")

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
        if self.options.shared:
            self.cpp_info.defines = ["LGGR_DLL"]
        if self.settings.os in ["Linux", "FreeBSD"]:
            self.cpp_info.system_libs = ["pthread"]
