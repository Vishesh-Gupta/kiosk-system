from conan import ConanFile

class KioskRecipe(ConanFile):
    settings = "os", "compiler", "build_type", "arch"
    generators = "CMakeToolchain", "CMakeDeps"

    def requirements(self):
        self.requires("libpq/15.4")
        self.requires("libpqxx/7.8.1")
        self.requires("grpc/1.54.3")

    def build_requirements(self):
        self.tool_requires("cmake/3.22.6")

    def package_info(self):
        # Method for defining package information, like include paths
        self.cpp_info.includedirs = ['include']