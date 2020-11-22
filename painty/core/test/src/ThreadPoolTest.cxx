/**
 * @file ThreadPoolTest.cxx
 * @author thomas lindemeier
 *
 * @brief
 *
 * @date 2020-10-20
 *
 */

#include "gtest/gtest.h"
#include "painty/core/ThreadPool.hxx"

TEST(ThreadPoolTest, Construct) {
  {
    painty::ThreadPool singleThread(1U);

    std::cout << "Hello World from main thread: " << std::this_thread::get_id()
              << std::endl;

    std::vector<std::future<void>> futures;
    for (auto i = 0U; i < 10U; i++) {
      futures.push_back(singleThread.add_back([]() {
        std::cout << "Hello World from thread: " << std::this_thread::get_id()
                  << std::endl;
      }));
    }

    for (const auto& f : futures) {
      f.wait();
    }
  }

  {
    painty::ThreadPool multiThread(10U);

    std::cout << "Hello World from main thread: " << std::this_thread::get_id()
              << std::endl;

    std::vector<std::future<void>> futures;
    for (auto i = 0U; i < 10U; i++) {
      futures.push_back(multiThread.add_back([]() {
        std::cout << "Hello World from thread: " << std::this_thread::get_id()
                  << std::endl;
      }));
    }

    for (const auto& f : futures) {
      f.wait();
    }
  }
}
