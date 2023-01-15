from conans import ConanFile, CMake


class ConanHelloEmscripten(ConanFile):
    name = "video-chat"
    version = "1.0"
    description = "vide-chat with emscripten"
    topics = ("video", "chat", "emscripten", "js", "javascript")
    license = "MIT"
    # url = "https://github.com/conan-io/examples"
    # homepage = "https://github.com/conan-io/examples"
    settings = {"os": ["Emscripten"]}
    requires = "zlib/1.2.11"
    exports_sources = ["CMakeLists.txt", "main.cpp"]
    generators = ["cmake"]

    def _configure_cmake(self):
        cmake = CMake(self)
        cmake.configure()
        return cmake

    def build(self):
        cmake = self._configure_cmake()
        cmake.build()

    def package(self):
        cmake = self._configure_cmake()
        cmake.install()
