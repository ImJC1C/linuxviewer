#pragma once

#include "DeclarationContext.h"
#include <vector>
#include <cstddef>
#include <map>

namespace vulkan::shaderbuilder {

class PushConstantDeclarationContext final : public DeclarationContext
{
 private:
  std::map<vk::ShaderStageFlagBits, uint32_t> m_minimum_offset;         // The minimum offset in the push constant struct of all push constants used in the shader of the key.
  std::map<vk::ShaderStageFlagBits, uint32_t> m_maximum_offset;         // The maximum offset in the push constant struct of all push constants used in the shader of the key.
  std::string m_header;
  std::vector<std::string> m_elements;
  std::string m_footer;

 public:
  PushConstantDeclarationContext(std::string prefix, std::size_t hash);

  void glsl_id_str_is_used_in(char const* glsl_id_str, vk::ShaderStageFlagBits shader_stage, ShaderVariable const* shader_variable, pipeline::ShaderInputData* shader_input_data) override;
  std::string generate_declaration(vk::ShaderStageFlagBits shader_stage) const override;
};

} // namespace vulkan::shaderbuilder
