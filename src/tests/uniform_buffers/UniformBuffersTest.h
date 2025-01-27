#pragma once

#include "vulkan/Application.h"

class UniformBuffersTest : public vulkan::Application
{
  using vulkan::Application::Application;

 private:
  int thread_pool_number_of_worker_threads() const override
  {
    // Lets use 4 worker threads in the thread pool.
    return 4;
  }

 public:
  std::u8string application_name() const override
  {
    return u8"UniformBuffersTest";
  }
};
