/* MAIN_CXX */
#include "sNotebook.hpp"

int main(void) {

  std::string filename;

  try {
    do {
      fmt::print("> ");
      filename = s::RuntimeManager::input();
    } while(filename.empty());
    s::RuntimeManager::run(filename);

  } catch (const std::exception &e) {
    fmt::print("Error: {}\n", e.what());
  }

  return 0;
}
