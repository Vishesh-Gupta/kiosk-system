from conan import ConanFile


class KioskRecipe(ConanFile):
    name = "kiosk"
    version = "1.0.0"
    description = "Kiosk service with gRPC and PostgreSQL"
    settings = "os", "compiler", "build_type", "arch"
    generators = "CMakeToolchain", "CMakeDeps"
    exports_sources = "src/*", "proto/*", "tests/*", "CMakeLists.txt"

    def requirements(self):
        # grpc pulls in a matching protobuf (and abseil); libpqxx pulls in libpq.
        self.requires("grpc/1.69.0")
        self.requires("libpqxx/7.10.3")
        self.requires("nlohmann_json/3.11.3")
        self.requires("ftxui/7.0.3")
        self.requires("gtest/1.15.0")

    def build_requirements(self):
        # protoc and grpc_cpp_plugin must run on the build machine; they come
        # from the same protobuf/grpc versions as the host libraries.
        self.tool_requires("protobuf/<host_version>")
        self.tool_requires("grpc/<host_version>")
