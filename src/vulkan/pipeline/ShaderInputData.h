#ifndef VULKAN_PIPELINE_PIPELINE_H
#define VULKAN_PIPELINE_PIPELINE_H

#include "shaderbuilder/ShaderInfo.h"
#include "shaderbuilder/ShaderIndex.h"
#include "shaderbuilder/SPIRVCache.h"
#include "shaderbuilder/LocationContext.h"
#include "shaderbuilder/VertexAttribute.h"
#include "shaderbuilder/PushConstant.h"
#include "shaderbuilder/PushConstantDeclarationContext.h"
#include "debug/DebugSetName.h"
#include "utils/Vector.h"
#include "utils/log2.h"
#include "utils/TemplateStringLiteral.h"
#include "utils/Badge.h"
#include <vector>
#include <set>
#include <tuple>
#include <memory>
#include <cxxabi.h>

// Forward declarations.

namespace task {
class SynchronousWindow;
} // namespace task

namespace vulkan::shaderbuilder {

class VertexShaderInputSetBase;
template<typename ENTRY> class VertexShaderInputSet;

struct VertexAttribute;

} // namespace vulkan::shaderbuilder

namespace vulkan::pipeline {

class ShaderInputData
{
 private:
  //---------------------------------------------------------------------------
  // Vertex attributes.
  utils::Vector<shaderbuilder::VertexShaderInputSetBase*> m_vertex_shader_input_sets;   // Existing vertex shader input sets (a 'binding' slot).
  std::set<shaderbuilder::VertexAttribute> m_vertex_attributes;                         // All existing vertex attributes of the above input sets (including declaration function).
  shaderbuilder::VertexAttributeDeclarationContext m_vertex_shader_location_context;    // Location context used for vertex attributes (VertexAttribute).
  using glsl_id_str_to_vertex_attribute_layout_container_t = std::map<std::string, vulkan::shaderbuilder::VertexAttributeLayout, std::less<>>;
  mutable glsl_id_str_to_vertex_attribute_layout_container_t m_glsl_id_str_to_vertex_attribute_layout;  // Map VertexAttributeLayout::m_glsl_id_str to the VertexAttributeLayout
  //---------------------------------------------------------------------------                         // object that contains it.

  //---------------------------------------------------------------------------
  // Push constants.
  using glsl_id_str_to_push_constant_container_t = std::map<std::string, vulkan::shaderbuilder::PushConstant, std::less<>>;
  glsl_id_str_to_push_constant_container_t m_glsl_id_str_to_push_constant;              // Map VertexAttributeLayout::m_glsl_id_str to the PushConstant object that contains it.
  using glsl_id_str_to_declaration_context_container_t = std::map<std::string, std::unique_ptr<shaderbuilder::DeclarationContext>>;
  glsl_id_str_to_declaration_context_container_t m_glsl_id_str_to_declaration_context;  // Map the prefix of VertexAttributeLayout::m_glsl_id_str to its DeclarationContext object.

  std::set<vk::PushConstantRange, PushConstantRangeCompare> m_push_constant_ranges;
  //---------------------------------------------------------------------------

  std::vector<shaderbuilder::ShaderVariable const*> m_shader_variables;         // A list of all ShaderVariable's (elements of m_vertex_attributes, m_glsl_id_str_to_push_constant, ...).
  std::vector<vk::PipelineShaderStageCreateInfo> m_shader_stage_create_infos;
  std::vector<vk::UniqueShaderModule> m_shader_modules;

 private:
  //---------------------------------------------------------------------------
  // Vertex attributes.

  // Add shader variable (VertexAttribute) to m_vertex_attributes (and a pointer to that to m_shader_variables)
  // and a VertexAttributeLayout to m_glsl_id_str_to_vertex_attribute_layout, for a non-array vertex attribute.
  template<typename ContainingClass, glsl::Standard Standard, glsl::ScalarIndex ScalarIndex, int Rows, int Cols, size_t Alignment, size_t Size, size_t ArrayStride,
      int MemberIndex, size_t MaxAlignment, size_t Offset, utils::TemplateStringLiteral GlslIdStr>
  void add_vertex_attribute(shaderbuilder::BindingIndex binding, shaderbuilder::MemberLayout<ContainingClass,
      shaderbuilder::BasicTypeLayout<Standard, ScalarIndex, Rows, Cols, Alignment, Size, ArrayStride>,
      MemberIndex, MaxAlignment, Offset, GlslIdStr> const& member_layout);

  // Add shader variable (VertexAttribute) to m_vertex_attributes (and a pointer to that to m_shader_variables)
  // and a VertexAttributeLayout to m_glsl_id_str_to_vertex_attribute_layout, for a vertex attribute array.
  template<typename ContainingClass, glsl::Standard Standard, glsl::ScalarIndex ScalarIndex, int Rows, int Cols, size_t Alignment, size_t Size, size_t ArrayStride,
      int MemberIndex, size_t MaxAlignment, size_t Offset, utils::TemplateStringLiteral GlslIdStr, size_t Elements>
  void add_vertex_attribute(shaderbuilder::BindingIndex binding, shaderbuilder::MemberLayout<ContainingClass,
      shaderbuilder::ArrayLayout<shaderbuilder::BasicTypeLayout<Standard, ScalarIndex, Rows, Cols, Alignment, Size, ArrayStride>, Elements>,
      MemberIndex, MaxAlignment, Offset, GlslIdStr> const& member_layout);

  // End vertex attribute.
  //---------------------------------------------------------------------------

  //---------------------------------------------------------------------------
  // Push constants.

  // Add shader variable (PushConstant) to m_glsl_id_str_to_push_constant (and a pointer to that to m_shader_variables),
  // and a declaration context for its prefix) if that doesn't already exist, for a non-array push constant.
  template<typename ContainingClass, glsl::Standard Standard, glsl::ScalarIndex ScalarIndex, int Rows, int Cols, size_t Alignment, size_t Size, size_t ArrayStride,
      int MemberIndex, size_t MaxAlignment, size_t Offset, utils::TemplateStringLiteral GlslIdStr>
  void add_push_constant_member(shaderbuilder::MemberLayout<ContainingClass,
      shaderbuilder::BasicTypeLayout<Standard, ScalarIndex, Rows, Cols, Alignment, Size, ArrayStride>,
      MemberIndex, MaxAlignment, Offset, GlslIdStr> const& member_layout);

  // Add shader variable (PushConstant) to m_glsl_id_str_to_push_constant (and a pointer to that to m_shader_variables),
  // and a declaration context for its prefix) if that doesn't already exist, for a push constant array.
  template<typename ContainingClass, glsl::Standard Standard, glsl::ScalarIndex ScalarIndex, int Rows, int Cols, size_t Alignment, size_t Size, size_t ArrayStride,
      int MemberIndex, size_t MaxAlignment, size_t Offset, utils::TemplateStringLiteral GlslIdStr, size_t Elements>
  void add_push_constant_member(shaderbuilder::MemberLayout<ContainingClass,
      shaderbuilder::ArrayLayout<shaderbuilder::BasicTypeLayout<Standard, ScalarIndex, Rows, Cols, Alignment, Size, ArrayStride>, Elements>,
      MemberIndex, MaxAlignment, Offset, GlslIdStr> const& member_layout);

  // End push constants.
  //---------------------------------------------------------------------------

 public:
  template<typename ENTRY>
  requires (std::same_as<typename shaderbuilder::ShaderVariableLayouts<ENTRY>::tag_type, glsl::per_vertex_data> ||
            std::same_as<typename shaderbuilder::ShaderVariableLayouts<ENTRY>::tag_type, glsl::per_instance_data>)
  void add_vertex_input_binding(shaderbuilder::VertexShaderInputSet<ENTRY>& vertex_shader_input_set);

  template<typename ENTRY>
  requires (std::same_as<typename shaderbuilder::ShaderVariableLayouts<ENTRY>::tag_type, glsl::push_constant_std430>)
  void add_push_constant();

  void build_shader(task::SynchronousWindow const* owning_window, shaderbuilder::ShaderIndex const& shader_index, shaderbuilder::ShaderCompiler const& compiler,
      shaderbuilder::SPIRVCache& spirv_cache
      COMMA_CWDEBUG_ONLY(AmbifixOwner const& ambifix));

  void build_shader(task::SynchronousWindow const* owning_window, shaderbuilder::ShaderIndex const& shader_index, shaderbuilder::ShaderCompiler const& compiler
      COMMA_CWDEBUG_ONLY(AmbifixOwner const& ambifix))
  {
    shaderbuilder::SPIRVCache tmp_spirv_cache;
    build_shader(owning_window, shader_index, compiler, tmp_spirv_cache COMMA_CWDEBUG_ONLY(ambifix));
  }

  // Create glsl code from template source code.
  //
  // glsl_source_code_buffer is only used when the code from shader_info needs preprocessing,
  // otherwise this function returns a string_view directly into the shader_info's source code.
  //
  // Hence, both shader_info and the string passed as glsl_source_code_buffer need to have a life time beyond the call to compile.
  std::string_view preprocess(shaderbuilder::ShaderInfo const& shader_info, std::string& glsl_source_code_buffer);

  // Called from PushConstantDeclarationContext::glsl_id_str_is_used_in.
  void insert(vk::PushConstantRange const& push_constant_range)
  {
    auto range = m_push_constant_ranges.equal_range(push_constant_range);
    m_push_constant_ranges.erase(range.first, range.second);
    m_push_constant_ranges.insert(push_constant_range);
  }

  std::vector<vk::PushConstantRange> push_constant_ranges() const
  {
    return { m_push_constant_ranges.begin(), m_push_constant_ranges.end() };
  }

  //---------------------------------------------------------------------------
  // Accessors.

  // Used by VertexAttribute::is_used_in to access the VertexAttributeDeclarationContext.
  shaderbuilder::VertexAttributeDeclarationContext& vertex_shader_location_context(utils::Badge<shaderbuilder::VertexAttribute>) { return m_vertex_shader_location_context; }
  auto const& vertex_shader_input_sets() const { return m_vertex_shader_input_sets; }

  // Used by PushConstant::is_used_in to look up the declaration context.
  glsl_id_str_to_declaration_context_container_t const& glsl_id_str_to_declaration_context(utils::Badge<shaderbuilder::PushConstant>) const { return m_glsl_id_str_to_declaration_context; }
  glsl_id_str_to_push_constant_container_t const& glsl_id_str_to_push_constant() const { return m_glsl_id_str_to_push_constant; }

  // Returns information on what was added with add_vertex_input_binding.
  std::vector<vk::VertexInputBindingDescription> vertex_binding_descriptions() const;

  // Returns information on what was added with add_vertex_input_binding.
  std::vector<vk::VertexInputAttributeDescription> vertex_input_attribute_descriptions() const;

  // Returns information on what was added with build_shader.
  std::vector<vk::PipelineShaderStageCreateInfo> const& shader_stage_create_infos() const { return m_shader_stage_create_infos; }
};

} // namespace vulkan::pipeline

#endif // VULKAN_PIPELINE_PIPELINE_H

#ifndef VULKAN_SHADERBUILDER_VERTEX_SHADER_INPUT_SET_H
#include "shaderbuilder/VertexShaderInputSet.h"
#endif
#ifndef VULKAN_SHADERBUILDER_VERTEX_ATTRIBUTE_ENTRY_H
#include "shaderbuilder/VertexAttribute.h"
#endif
#ifndef VULKAN_APPLICATION_H
#include "Application.h"
#endif

#ifndef VULKAN_PIPELINE_PIPELINE_H_definitions
#define VULKAN_PIPELINE_PIPELINE_H_definitions

namespace vulkan::pipeline {

template<typename ContainingClass, glsl::Standard Standard, glsl::ScalarIndex ScalarIndex, int Rows, int Cols, size_t Alignment, size_t Size, size_t ArrayStride,
    int MemberIndex, size_t MaxAlignment, size_t Offset, utils::TemplateStringLiteral GlslIdStr>
void ShaderInputData::add_vertex_attribute(shaderbuilder::BindingIndex binding, shaderbuilder::MemberLayout<ContainingClass,
    shaderbuilder::BasicTypeLayout<Standard, ScalarIndex, Rows, Cols, Alignment, Size, ArrayStride>,
    MemberIndex, MaxAlignment, Offset, GlslIdStr> const& member_layout)
{
  std::string_view glsl_id_sv = static_cast<std::string_view>(member_layout.glsl_id_str);
  // These strings are made with std::to_array("stringliteral"), which includes the terminating zero,
  // but the trailing '\0' was already removed by the conversion to string_view.
  ASSERT(!glsl_id_sv.empty() && glsl_id_sv.back() != '\0');
  shaderbuilder::VertexAttributeLayout vertex_attribute_layout_tmp{
    .m_base_type = {
      .m_standard = Standard,
      .m_rows = Rows,
      .m_cols = Cols,
      .m_scalar_type = ScalarIndex,
      .m_log2_alignment = utils::log2(Alignment),
      .m_size = Size,
      .m_array_stride = ArrayStride },
    .m_glsl_id_str = glsl_id_sv.data(),
    .m_offset = Offset,
    .m_array_size = 0
  };
  Dout(dc::vulkan, "Registering \"" << glsl_id_sv << "\" with layout " << vertex_attribute_layout_tmp);
  auto res1 = m_glsl_id_str_to_vertex_attribute_layout.insert(std::pair{glsl_id_sv, vertex_attribute_layout_tmp});
  // The m_glsl_id_str of each ENTRY must be unique. And of course, don't register the same attribute twice.
  ASSERT(res1.second);

  shaderbuilder::VertexAttributeLayout const* vertex_attribute_layout = &res1.first->second;
  auto res2 = m_vertex_attributes.emplace(vertex_attribute_layout, binding);
  // All used names must be unique.
  if (!res2.second)
    THROW_ALERT("Duplicated shader variable layout id \"[ID_STR]\". All used ids must be unique.", AIArgs("[ID_STR]", vertex_attribute_layout->m_glsl_id_str));
  // Add a pointer to the VertexAttribute that was just added to m_vertex_attributes to m_shader_variables.
  m_shader_variables.push_back(&*res2.first);
}

template<typename ContainingClass, glsl::Standard Standard, glsl::ScalarIndex ScalarIndex, int Rows, int Cols, size_t Alignment, size_t Size, size_t ArrayStride,
    int MemberIndex, size_t MaxAlignment, size_t Offset, utils::TemplateStringLiteral GlslIdStr, size_t Elements>
void ShaderInputData::add_vertex_attribute(shaderbuilder::BindingIndex binding, shaderbuilder::MemberLayout<ContainingClass,
    shaderbuilder::ArrayLayout<shaderbuilder::BasicTypeLayout<Standard, ScalarIndex, Rows, Cols, Alignment, Size, ArrayStride>, Elements>,
    MemberIndex, MaxAlignment, Offset, GlslIdStr> const& member_layout)
{
  std::string_view glsl_id_sv = static_cast<std::string_view>(member_layout.glsl_id_str);
  // These strings are made with std::to_array("stringliteral"), which includes the terminating zero,
  // but the trailing '\0' was already removed by the conversion to string_view.
  ASSERT(!glsl_id_sv.empty() && glsl_id_sv.back() != '\0');
  shaderbuilder::VertexAttributeLayout vertex_attribute_layout_tmp{
    .m_base_type = {
      .m_standard = Standard,
      .m_rows = Rows,
      .m_cols = Cols,
      .m_scalar_type = ScalarIndex,
      .m_log2_alignment = utils::log2(Alignment),
      .m_size = Size,
      .m_array_stride = ArrayStride },
    .m_glsl_id_str = glsl_id_sv.data(),
    .m_offset = Offset,
    .m_array_size = Elements
  };
  Dout(dc::vulkan, "Registering \"" << glsl_id_sv << "\" with layout " << vertex_attribute_layout_tmp);
  auto res1 = m_glsl_id_str_to_vertex_attribute_layout.insert(std::pair{glsl_id_sv, vertex_attribute_layout_tmp});
  // The m_glsl_id_str of each ENTRY must be unique. And of course, don't register the same attribute twice.
  ASSERT(res1.second);

  shaderbuilder::VertexAttributeLayout const* vertex_attribute_layout = &res1.first->second;
  // m_array_size should have been set during the call to Application::register_vertex_attributes.
  ASSERT(vertex_attribute_layout->m_array_size == Elements);
  auto res2 = m_vertex_attributes.emplace(vertex_attribute_layout, binding);
  // All used names must be unique.
  if (!res2.second)
    THROW_ALERT("Duplicated shader variable layout id \"[ID_STR]\". All used ids must be unique.", AIArgs("[ID_STR]", vertex_attribute_layout->m_glsl_id_str));
  // Add a pointer to the VertexAttribute that was just added to m_vertex_attributes to m_shader_variables.
  m_shader_variables.push_back(&*res2.first);
}

template<typename ENTRY>
requires (std::same_as<typename shaderbuilder::ShaderVariableLayouts<ENTRY>::tag_type, glsl::per_vertex_data> ||
          std::same_as<typename shaderbuilder::ShaderVariableLayouts<ENTRY>::tag_type, glsl::per_instance_data>)
void ShaderInputData::add_vertex_input_binding(shaderbuilder::VertexShaderInputSet<ENTRY>& vertex_shader_input_set)
{
  DoutEntering(dc::vulkan, "vulkan::pipeline::add_vertex_input_binding<" << libcwd::type_info_of<ENTRY>().demangled_name() << ">(...)");
  using namespace shaderbuilder;

  BindingIndex binding = m_vertex_shader_input_sets.iend();

  // Use the specialization of ShaderVariableLayouts to get the layout of ENTRY
  // in the form of a tuple, of the vertex attributes, `layouts`.
  // Then for each member layout call insert_vertex_attribute.
  [&]<typename... MemberLayout>(std::tuple<MemberLayout...> const& layouts)
  {
    ([&, this]
    {
      {
#if 0 // Print the type MemberLayout.
        int rc;
        char const* const demangled_type = abi::__cxa_demangle(typeid(MemberLayout).name(), 0, 0, &rc);
        Dout(dc::vulkan, "We get here for type " << demangled_type);
#endif
        static constexpr int member_index = MemberLayout::member_index;
        add_vertex_attribute(binding, std::get<member_index>(layouts));
      }
    }(), ...);
  }(typename decltype(ShaderVariableLayouts<ENTRY>::struct_layout)::members_tuple{});

  // Keep track of all VertexShaderInputSetBase objects.
  m_vertex_shader_input_sets.push_back(&vertex_shader_input_set);
}

template<typename ContainingClass, glsl::Standard Standard, glsl::ScalarIndex ScalarIndex, int Rows, int Cols, size_t Alignment, size_t Size, size_t ArrayStride,
    int MemberIndex, size_t MaxAlignment, size_t Offset, utils::TemplateStringLiteral GlslIdStr>
void ShaderInputData::add_push_constant_member(shaderbuilder::MemberLayout<ContainingClass,
    shaderbuilder::BasicTypeLayout<Standard, ScalarIndex, Rows, Cols, Alignment, Size, ArrayStride>,
    MemberIndex, MaxAlignment, Offset, GlslIdStr> const& member_layout)
{
  std::string_view glsl_id_sv = static_cast<std::string_view>(member_layout.glsl_id_str);
  shaderbuilder::BasicType const basic_type = { .m_standard = Standard, .m_rows = Rows, .m_cols = Cols, .m_scalar_type = ScalarIndex,
    .m_log2_alignment = utils::log2(Alignment), .m_size = Size, .m_array_stride = ArrayStride };
  shaderbuilder::PushConstant push_constant(basic_type, member_layout.glsl_id_str.chars.data(), Offset);
  auto res1 = m_glsl_id_str_to_push_constant.insert(std::pair{glsl_id_sv, push_constant});
  // The m_glsl_id_str of each ENTRY must be unique. And of course, don't register the same push constant twice.
  ASSERT(res1.second);
  m_shader_variables.push_back(&res1.first->second);

  // Add a PushConstantDeclarationContext with key push_constant.prefix(), if that doesn't already exists.
  if (m_glsl_id_str_to_declaration_context.find(push_constant.prefix()) == m_glsl_id_str_to_declaration_context.end())
  {
    std::size_t const hash = std::hash<std::string>{}(push_constant.prefix());
    DEBUG_ONLY(auto res2 =)
      m_glsl_id_str_to_declaration_context.try_emplace(push_constant.prefix(), std::make_unique<shaderbuilder::PushConstantDeclarationContext>(push_constant.prefix(), hash));
    // We just used find() and it wasn't there?!
    ASSERT(res2.second);
  }
}

template<typename ContainingClass, glsl::Standard Standard, glsl::ScalarIndex ScalarIndex, int Rows, int Cols, size_t Alignment, size_t Size, size_t ArrayStride,
    int MemberIndex, size_t MaxAlignment, size_t Offset, utils::TemplateStringLiteral GlslIdStr, size_t Elements>
void ShaderInputData::add_push_constant_member(shaderbuilder::MemberLayout<ContainingClass,
    shaderbuilder::ArrayLayout<shaderbuilder::BasicTypeLayout<Standard, ScalarIndex, Rows, Cols, Alignment, Size, ArrayStride>, Elements>,
    MemberIndex, MaxAlignment, Offset, GlslIdStr> const& member_layout)
{
  std::string_view glsl_id_sv = static_cast<std::string_view>(member_layout.glsl_id_str);
  shaderbuilder::BasicType const basic_type = { .m_standard = Standard, .m_rows = Rows, .m_cols = Cols, .m_scalar_type = ScalarIndex,
    .m_log2_alignment = utils::log2(Alignment), .m_size = Size, .m_array_stride = ArrayStride };
  shaderbuilder::PushConstant push_constant(basic_type, member_layout.glsl_id_str.chars.data(), Offset, Elements);
  auto res1 = m_glsl_id_str_to_push_constant.insert(std::pair{glsl_id_sv, push_constant});
  // The m_glsl_id_str of each ENTRY must be unique. And of course, don't register the same push constant twice.
  ASSERT(res1.second);
  m_shader_variables.push_back(&res1.first->second);

  // Add a PushConstantDeclarationContext with key push_constant.prefix(), if that doesn't already exists.
  if (m_glsl_id_str_to_declaration_context.find(push_constant.prefix()) == m_glsl_id_str_to_declaration_context.end())
  {
    std::size_t const hash = std::hash<std::string>{}(push_constant.prefix());
    DEBUG_ONLY(auto res2 =)
      m_glsl_id_str_to_declaration_context.try_emplace(push_constant.prefix(), std::make_unique<shaderbuilder::DeclarationContext>(push_constant.prefix(), hash));
    // We just used find() and it wasn't there?!
    ASSERT(res2.second);
  }
}

template<typename ENTRY>
requires (std::same_as<typename shaderbuilder::ShaderVariableLayouts<ENTRY>::tag_type, glsl::push_constant_std430>)
void ShaderInputData::add_push_constant()
{
  DoutEntering(dc::vulkan, "vulkan::pipeline::add_push_constant<" << libcwd::type_info_of<ENTRY>().demangled_name() << ">(...)");
  using namespace shaderbuilder;

  [&]<typename... MemberLayout>(std::tuple<MemberLayout...> const& layouts)
  {
    ([&, this]
    {
      {
#if 1 // Print the type MemberLayout.
        int rc;
        char const* const demangled_type = abi::__cxa_demangle(typeid(MemberLayout).name(), 0, 0, &rc);
        Dout(dc::vulkan, "We get here for type " << demangled_type);
#endif
        static constexpr int member_index = MemberLayout::member_index;
        add_push_constant_member(std::get<member_index>(layouts));
      }
    }(), ...);
  }(typename decltype(ShaderVariableLayouts<ENTRY>::struct_layout)::members_tuple{});
}

} // namespace vulkan::pipeline

#endif // VULKAN_PIPELINE_PIPELINE_H_definitions
