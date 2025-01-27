#pragma once

#include <vulkan/vulkan.hpp>
#include "debug.h"

namespace vulkan::pipeline {

// Compare functor for m_push_constant_ranges.
struct PushConstantRangeCompare
{
  bool operator()(vk::PushConstantRange const& pcr1, vk::PushConstantRange const& pcr2) const
  {
    // If the shader stage that this push constant range is used for is different then the
    // ranges can co-exist; just order them by stageFlags.
    if (!(pcr1.stageFlags & pcr2.stageFlags))
      return pcr1.stageFlags < pcr2.stageFlags;
    // If two ranges have at least one shader stage (bit) in common; then they can NOT co-exist.
    // If we return "equal" here, then the old range will be replaced, and the way that the ranges are
    // generated by PushConstantDeclarationContext::glsl_id_str_is_used_in that should be the smaller one.
    // For example:
    //       |<-----old range already in the set----->|
    //       |<-----new range being added----------------->|  (the maximum offset just became larger).
    //       or
    //  |<-----new range being added----------------->|       (the minimum offset just became smaller).
    //  Hence, this is something we expect. However, if the ranges do not have one end in common then
    //  something unexpected is going and we should assert.
    ASSERT(pcr1.offset == pcr2.offset || pcr1.offset + pcr1.size == pcr2.offset + pcr2.size);
    return pcr1.offset < pcr2.offset && pcr1.offset + pcr1.size < pcr2.offset + pcr2.size;
  }
};

} // namespace vulkan
