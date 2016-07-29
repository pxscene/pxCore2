#include <iostream>
#include <chrono>
#include <unistd.h>

int main(int argc, char* argv[])
{
  auto delay = std::chrono::system_clock::now() + std::chrono::milliseconds(500);

//  sleep(2);
  auto current_delay = delay - std::chrono::system_clock::now();

  std::cout << "current_delay:" << current_delay.count() << std::endl;

  return 0;
}
