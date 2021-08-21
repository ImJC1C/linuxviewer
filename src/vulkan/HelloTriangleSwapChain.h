#pragma once

#include "Device.h"
#include "Queue.h"

#include <vulkan/vulkan.h>

#include <string>
#include <vector>

namespace vulkan {

class HelloTriangleSwapChain
{
 //public:
  static constexpr int MAX_FRAMES_IN_FLIGHT = 2;

 private:
  // Constructor
  Device const& device;

  // setup
  vk::Extent2D windowExtent;
  vk::Queue m_graphics_queue;
  vk::Queue m_present_queue;

  // createSyncObjects
  std::vector<vk::Semaphore> imageAvailableSemaphores;
  std::vector<vk::Semaphore> renderFinishedSemaphores;
  std::vector<vk::Fence> inFlightFences;
  std::vector<vk::Fence> imagesInFlight;

  vk::Format swapChainImageFormat;
  vk::Extent2D swapChainExtent;

  std::vector<vk::Framebuffer> swapChainFramebuffers;
  vk::RenderPass renderPass;

  std::vector<vk::Image> depthImages;
  std::vector<vk::DeviceMemory> depthImageMemorys;
  std::vector<vk::ImageView> depthImageViews;
  std::vector<vk::Image> swapChainImages;
  std::vector<vk::ImageView> swapChainImageViews;

  vk::SwapchainKHR swapChain;

  size_t currentFrame = 0;

 public:
  HelloTriangleSwapChain(Device const& deviceRef);
  ~HelloTriangleSwapChain();

  HelloTriangleSwapChain(HelloTriangleSwapChain const&) = delete;
  void operator=(HelloTriangleSwapChain const&) = delete;

  void setup(vk::Extent2D windowExtent, Queue graphics_queue, Queue present_queue, vk::SurfaceKHR surface);

  vk::Framebuffer getFrameBuffer(int index) const { return swapChainFramebuffers[index]; }
  vk::RenderPass getRenderPass() const { return renderPass; }
  vk::ImageView getImageView(int index) { return swapChainImageViews[index]; }
  size_t imageCount() const { return swapChainImages.size(); }
  vk::Format getSwapChainImageFormat() const { return swapChainImageFormat; }
  vk::Extent2D getSwapChainExtent() const { return swapChainExtent; }
  uint32_t width() const { return swapChainExtent.width; }
  uint32_t height() const { return swapChainExtent.height; }

  float extentAspectRatio() {
    return static_cast<float>(swapChainExtent.width) / static_cast<float>(swapChainExtent.height);
  }

  uint32_t acquireNextImage();
  void submitCommandBuffers(vk::CommandBuffer const& buffers, uint32_t const imageIndex);

 private:
  void createSwapChain(vk::SurfaceKHR surface, Queue graphics_queue, Queue present_queue);
  void createImageViews();
  void createDepthResources();
  void createRenderPass();
  void createFramebuffers();
  void createSyncObjects();

  // Helper function.
  vk::Format findDepthFormat();
};

} // namespace vulkan
