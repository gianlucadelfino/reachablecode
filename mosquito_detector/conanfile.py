from conans import ConanFile
from conan.tools.cmake import CMakeToolchain, CMake
from conan.tools.layout import cmake_layout


class MosquitoDetectorConan(ConanFile):
    name = "mosquito_detector"
    version = "0.1.0"

    # Optional metadata
    license = "MIT"
    author = "gianluca.delfino@gmail.com"
    url = "<Package recipe repository url here, for issues about the package>"
    description = "<Description of MosquitoDetector here>"
    topics = ("<Put some tag here>", "<here>", "<and here>")

    # Binary configuration
    settings = "os", "compiler", "build_type", "arch"

    # Sources are located in the same place as this recipe, copy them to the recipe
    exports_sources = "CMakeLists.txt", "src/*"

    # These 2 lines were Added manually!
    requires = ("opencv/4.5.3",)
    generators = "cmake_find_package"

    def layout(self):
        cmake_layout(self)

    def generate(self):
        tc = CMakeToolchain(self)
        tc.generate()

    def build(self):
        cmake = CMake(self)
        cmake.configure()
        cmake.build()

    def package(self):
        cmake = CMake(self)
        cmake.install()
