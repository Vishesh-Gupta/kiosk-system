from conan import ConanFile

class KioskRecipe(ConanFile):
    name = "kiosk"
    version = "1.0.0"
    description = "Kiosk service with gRPC and PostgreSQL"
    settings = "os", "compiler", "build_type", "arch"
    generators = "CMakeToolchain", "CMakeDeps"
    exports_sources = "src/*", "proto/*", "CMakeLists.txt"

    def requirements(self):
        self.requires("libpq/15.4")
        self.requires("libpqxx/7.10.3") 
        self.requires("grpc/1.69.0")
        self.requires("nlohmann_json/3.11.2")
        self.requires("boost/1.80.0")
        self.requires("gtest/1.13.0")

    def build_requirements(self):
        self.tool_requires("cmake/4.1.3")
        self.tool_requires("protobuf/3.21.9")

    def package_info(self):
        self.cpp_info.includedirs = ["include"]
        self.cpp_info.libdirs = ["lib"]
        self.cpp_info.bindirs = ["bin"]