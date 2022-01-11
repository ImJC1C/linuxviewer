#pragma once

#include "ShaderCompiler.h"
#include <vulkan/vulkan.hpp>
#include <shaderc/shaderc.h>    // shaderc_shader_kind, shaderc_glsl_infer_from_source
#include <cstdint>
#include <string>
#include <filesystem>
#include <vector>
#include <concepts>
#include "debug.h"
#ifdef CWDEBUG
#include <iosfwd>
#endif

namespace task {
class SynchronousWindow;
} // namespace task

namespace vulkan {

class Pipeline;

namespace shaderbuilder {

class ShaderModule
{
 private:
  vk::ShaderStageFlagBits const m_stage;        // The stage that this shader will be used in.
  std::string m_name;                           // Shader name, used for diagnostics.
  std::string m_glsl_source_code;               // GLSL source code; loaded with load().
  std::vector<uint32_t> m_spirv_code;           // Cached, compiled SPIR-V code.

 public:
  // Construct an empty ShaderModule object to be used for the specified stage.
  // A name can be specified at construction or later with set_name. Note that a call to reset() does NOT reset the name.
  ShaderModule(vk::ShaderStageFlagBits stage, std::string name = "uninitialized shader") : m_stage(stage), m_name(std::move(name)) { }

  // Set name of this object (for diagnostics).
  ShaderModule& set_name(std::string name)
  {
    m_name = std::move(name);
    return *this;
  }

  // Open file filename and read its text contents into m_glsl_source_code.
  // Only use this overload when explicitly using a std::filesystem::path.
  ShaderModule& load(std::same_as<std::filesystem::path> auto const& filename);

  // Load source code from a string.
  ShaderModule& load(std::string_view source)
  {
    m_glsl_source_code = source;
    return *this;
  }

  // Compile the code in m_glsl_source_code and cache it in m_spirv_code.
  void compile(ShaderCompiler const& compiler, ShaderCompilerOptions const& options);

  // Compile and forget.
  vk::UniqueShaderModule create(task::SynchronousWindow const* owning_window, ShaderCompiler const& compiler, ShaderCompilerOptions const& options) const;

  // Create handle from cached SPIR-V code.
  // Use vulkan::Pipeline::add(shader_module) instead of this function.
  vk::UniqueShaderModule create(utils::Badge<vulkan::Pipeline>, task::SynchronousWindow const* owning_window) const;

  // Free resources.
  void reset();

  // Accessors.
  std::string_view glsl_source() const { return m_glsl_source_code; }
  vk::ShaderStageFlagBits stage() const { return m_stage; }
  std::string const& name() const { return m_name; }

  shaderc_shader_kind get_shader_kind() const;

#ifdef CWDEBUG
  void print_on(std::ostream& os) const;
#endif
};

} // namespace shaderbuilder
} // namespace vulkan