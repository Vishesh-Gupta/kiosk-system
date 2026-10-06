#include <atomic>
#include <chrono>
#include <cstdlib>
#include <cstring>
#include <ftxui/component/event.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <iostream>
#include <string>
#include <thread>

#include "kiosk_client.h"
#include "kiosk_tui.h"

namespace {

void printUsage(const char* program) {
  std::cout << "Usage: " << program << " [--address host:port]\n\n"
            << "Interactive terminal UI for the kiosk gRPC server.\n\n"
            << "  -a, --address   server address (default: $KIOSK_SERVER or localhost:50051)\n"
            << "  -h, --help      show this help\n";
}

}  // namespace

int main(int argc, char** argv) {
  std::string address = "localhost:50051";
  if (const char* env = std::getenv("KIOSK_SERVER"); env != nullptr && *env != '\0') {
    address = env;
  }

  for (int i = 1; i < argc; ++i) {
    const std::string arg = argv[i];
    if (arg == "-h" || arg == "--help") {
      printUsage(argv[0]);
      return EXIT_SUCCESS;
    }
    if ((arg == "-a" || arg == "--address") && i + 1 < argc) {
      address = argv[++i];
    } else if (arg.rfind("--address=", 0) == 0) {
      address = arg.substr(std::strlen("--address="));
    } else {
      std::cerr << "Unknown argument: " << arg << "\n\n";
      printUsage(argv[0]);
      return EXIT_FAILURE;
    }
  }

  auto client = KioskClient::Connect(address);
  auto screen = ftxui::ScreenInteractive::Fullscreen();

  WorkerDispatcher dispatcher([&screen](std::function<void()> task) {
    screen.Post(std::move(task));
    screen.PostEvent(ftxui::Event::Custom);  // redraw with the new state
  });
  KioskApp app(*client, dispatcher, screen.ExitLoopClosure());
  app.start();

  // Re-check server health periodically so the status badge stays current.
  std::atomic<bool> running{true};
  std::thread healthTicker([&] {
    using namespace std::chrono_literals;
    while (running) {
      for (int i = 0; i < 50 && running; ++i) {
        std::this_thread::sleep_for(100ms);
      }
      if (running) {
        screen.Post([&app] { app.refreshHealth(); });
      }
    }
  });

  screen.Loop(app.component());

  running = false;
  healthTicker.join();
  dispatcher.stop();  // background tasks reference `app`; finish them first
  return EXIT_SUCCESS;
}
