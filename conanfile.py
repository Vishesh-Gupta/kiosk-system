from conan import ConanFile

class KioskRecipe(ConanFile):
    settings = "os", "compiler", "build_type", "arch"
    generators = "CMakeToolchain", "CMakeDeps"

    def requirements(self):
        self.requires("libpq/15.4")

    def build_requirements(self):
        self.tool_requires("cmake/3.22.6")