/*
 * RedEye Engine - A 3D Game Engine written in C++.
 * Copyright (C) 2018-2024 Julia Mauri and Ruben Sardon
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 */

module;

#include <SDL2/SDL.h>
#include <SDL_vulkan.h>
#include <vulkan/vulkan.h>

#include <iostream>
#include <vector>
#include <set>

export module Vulkan;

std::vector<VkLayerProperties> availableLayers{};

// TODO: Setup Allocation Callbacks for memory management
const VkAllocationCallbacks* allocation_callbacks = nullptr;
VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                                             VkDebugUtilsMessageTypeFlagsEXT messageType,
                                             const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData)
{
    std::cerr << "Validation layer: " << pCallbackData->pMessage << std::endl;
    return VK_FALSE;
}

enum DeviceFeature : uint8_t
{
    GEOMETRY_SHADER = 0,
    TESSELLATION_SHADER = 1,
    SAMPLER_ANISOTROPY = 2
};

namespace Criteria
{
    namespace Prefered
    {
        const std::vector<const char*> layers = {
            "VK_LAYER_KHRONOS_validation", // requieres vulkan-validationlayers vcpkg
            "VK_LAYER_NV_optimus" // Ensures discrete NVIDIA GPU usage instead of default integrated GPU
                                  // Improves Nvidia performance on laptops
        };
    } // namespace Prefered

    namespace Required
    {
        // Device Settings
        const uint8_t features =
            DeviceFeature::GEOMETRY_SHADER | DeviceFeature::TESSELLATION_SHADER | DeviceFeature::SAMPLER_ANISOTROPY;
        const std::vector<const char*> extensions = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};

        // Device->Surface Settings
        VkSurfaceFormatKHR surface_format = {VK_FORMAT_B8G8R8A8_SRGB, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR};
        VkPresentModeKHR present_mode = VK_PRESENT_MODE_FIFO_KHR;
    } // namespace Required

    namespace Swapchain
    {
        VkExtent2D Extent(const VkSurfaceCapabilitiesKHR& capabilities, const VkExtent2D& window_size)
        {
            // Fixed size surface
            if (capabilities.currentExtent.width != UINT32_MAX)
                return capabilities.currentExtent;

            // Resizable surface
            return {std::max(capabilities.minImageExtent.width,
                             std::min(capabilities.maxImageExtent.width, window_size.width)),
                    std::max(capabilities.minImageExtent.height,
                             std::min(capabilities.maxImageExtent.height, window_size.height))};
        }
        uint32_t MinImageCount(const VkSurfaceCapabilitiesKHR& capabilities)
        {
            return capabilities.maxImageCount > 0 && capabilities.minImageCount + 1 > capabilities.maxImageCount
                       ? capabilities.maxImageCount
                       : capabilities.minImageCount + 1;
        }
    } // namespace Swapchain
} // namespace Criteria

namespace Shaders
{
    namespace Default
    {
        const char* Vertex = R"(
            #version 450
            layout(location = 0) in vec3 inPosition;
            layout(location = 1) in vec3 inColor;
            
            layout(location = 0) out vec3 fragColor;
            
            layout(set = 0, binding = 0) uniform TransfomMatrices {
                mat4 model;
                mat4 view;
                mat4 proj;
            } ubo;
            
            void main() {
                gl_Position = ubo.proj * ubo.view * ubo.model * vec4(inPosition, 1.0);
                fragColor = inColor;
            }
        )";

        const char* Fragment = R"(
            #version 450
            layout(location = 0) in vec3 fragColor;
            layout(location = 0) out vec4 outColor;

            void main() {
                outColor = vec4(fragColor, 1.0);
            }
        )";
    } // namespace Default

    bool CreateModule(VkDevice logical_device, const char* code, VkShaderModule& shaderModule)
    {
        size_t codeSize = strlen(code);
        std::vector<uint32_t> codeBytes((codeSize + 3) / 4);
        memcpy(codeBytes.data(), code, codeSize);

        VkShaderModuleCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        createInfo.codeSize = codeBytes.size() * sizeof(uint32_t);
        createInfo.pCode = codeBytes.data();

        if (vkCreateShaderModule(logical_device, &createInfo, allocation_callbacks, &shaderModule) != VK_SUCCESS)
        {
            std::cerr << "Failed to create shader module!" << std::endl;
            return false;
        }

        return true;
    }

    // Vertex Input

    struct Vertex
    {
        float pos[3];
        float color[3];

        static inline VkVertexInputBindingDescription BindingDescription()
        {
            return {0, sizeof(float) * 6, VK_VERTEX_INPUT_RATE_VERTEX};
        }
    };

    void PopulateVertexInputState(VkPipelineVertexInputStateCreateInfo& vertexInputInfo)
    {
        static std::vector<VkVertexInputBindingDescription> binding_descriptions = {Vertex::BindingDescription()};
        static std::vector<VkVertexInputAttributeDescription> attribute_descriptions = {
            {0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, pos)},
            {1, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, color)}};

        vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
        vertexInputInfo.vertexBindingDescriptionCount = static_cast<uint32_t>(binding_descriptions.size());
        vertexInputInfo.pVertexBindingDescriptions = binding_descriptions.data();
        vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attribute_descriptions.size());
        vertexInputInfo.pVertexAttributeDescriptions = attribute_descriptions.data();
    }

    // Uniforms

    struct TransfomMatrices
    {
        float model[16] = {1.f, 0.f, 0.f, 0.f, 0.f, 1.f, 0.f, 0.f, 0.f, 0.f, 1.f, 0.f, 0.f, 0.f, 0.f, 1.f};
        float view[16] = {1.f, 0.f, 0.f, 0.f, 0.f, 1.f, 0.f, 0.f, 0.f, 0.f, 1.f, 0.f, 0.f, 0.f, 0.f, 1.f};
        float proj[16] = {1.f, 0.f, 0.f, 0.f, 0.f, 1.f, 0.f, 0.f, 0.f, 0.f, 1.f, 0.f, 0.f, 0.f, 0.f, 1.f};

        static inline VkDescriptorSetLayoutBinding LayoutBinding()
        {
            return {0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT, nullptr};
        }
    };

    std::vector<VkDescriptorSetLayoutBinding> layout_bindings = {TransfomMatrices::LayoutBinding()};
    std::vector<VkDescriptorPoolSize> pool_sizes = {{VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1}};
}

namespace ToString
{
    const char* PhysicalDeviceVendor(uint32_t vendorID)
    {
        switch (vendorID)
        {
            case 0x8086:
                return "Intel";
            case 0x10DE:
                return "NVIDIA";
            case 0x1002:
                return "AMD";
            default:
                return "Unknown Vendor";
        }
    }

    const char* PhysicalDeviceType(uint32_t deviceType)
    {
        switch (deviceType)
        {
            case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU:
                return "Integrated GPU";
            case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU:
                return "Discrete GPU";
            case VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU:
                return "Virtual GPU";
            case VK_PHYSICAL_DEVICE_TYPE_CPU:
                return "CPU";
            case VK_PHYSICAL_DEVICE_TYPE_OTHER:
            default:
                return "Other";
        }
    }
} // namespace ToString

namespace Log
{
    namespace Window
    {
        void SizeProperties(SDL_Window* window)
        {
            int logicalWidth, logicalHeight;
            SDL_GetWindowSize(window, &logicalWidth, &logicalHeight);

            int drawableWidth, drawableHeight;
            SDL_Vulkan_GetDrawableSize(window, &drawableWidth, &drawableHeight);

            float dpiScaleX = static_cast<float>(drawableWidth) / logicalWidth;
            float dpiScaleY = static_cast<float>(drawableHeight) / logicalHeight;

            std::cout << "Window Properties: " << std::endl;
            std::cout << "\t  Logical size: " << logicalWidth << "x" << logicalHeight << "\n";
            std::cout << "\t  Drawable size: " << drawableWidth << "x" << drawableHeight << "\n";
            std::cout << "\t  DPI Scale: " << dpiScaleX << ", " << dpiScaleY << "\n";
        }
    } // namespace Window

    namespace PhysicalDevice
    {
        void SparseProperties(const VkPhysicalDeviceSparseProperties& sparseProperties)
        {
            std::cout << "\t  Sparse Properties:" << std::endl;
            std::cout << "\t\t- Residency Standard 2D Block Shape: " << sparseProperties.residencyStandard2DBlockShape
                      << std::endl;
            std::cout << "\t\t- Residency Standard 2D Multisample Block Shape: "
                      << sparseProperties.residencyStandard2DMultisampleBlockShape << std::endl;
            std::cout << "\t\t- Residency Standard 3D Block Shape: " << sparseProperties.residencyStandard3DBlockShape
                      << std::endl;
            std::cout << "\t\t- Residency Aligned Mip Size: " << sparseProperties.residencyAlignedMipSize << std::endl;
            std::cout << "\t\t- Residency Non-Resident Strict: " << sparseProperties.residencyNonResidentStrict
                      << std::endl;
        }

        void DeviceLimits(const VkPhysicalDeviceLimits& limits)
        {
            std::cout << "\t  Device Limits:" << std::endl;
            std::cout << "\t\t- Max Image Dimension 1D: " << limits.maxImageDimension1D << std::endl;
            std::cout << "\t\t- Max Image Dimension 2D: " << limits.maxImageDimension2D << std::endl;
            std::cout << "\t\t- Max Image Dimension 3D: " << limits.maxImageDimension3D << std::endl;
            std::cout << "\t\t- Max Image Dimension Cube: " << limits.maxImageDimensionCube << std::endl;
            std::cout << "\t\t- Max Image Array Layers: " << limits.maxImageArrayLayers << std::endl;
            std::cout << "\t\t- Max Texel Buffer Elements: " << limits.maxTexelBufferElements << std::endl;
            std::cout << "\t\t- Max Uniform Buffer Range: " << limits.maxUniformBufferRange << std::endl;
            std::cout << "\t\t- Max Storage Buffer Range: " << limits.maxStorageBufferRange << std::endl;
            std::cout << "\t\t- Max Push Constants Size: " << limits.maxPushConstantsSize << std::endl;
            std::cout << "\t\t- Max Memory Allocation Count: " << limits.maxMemoryAllocationCount << std::endl;
            std::cout << "\t\t- Max Sampler Allocation Count: " << limits.maxSamplerAllocationCount << std::endl;
            std::cout << "\t\t- Buffer Image Granularity: " << limits.bufferImageGranularity << std::endl;
            std::cout << "\t\t- Sparse Address Space Size: " << limits.sparseAddressSpaceSize << std::endl;
            std::cout << "\t\t- Max Bound Descriptor Sets: " << limits.maxBoundDescriptorSets << std::endl;
            std::cout << "\t\t- Max Per Stage Descriptor Samplers: " << limits.maxPerStageDescriptorSamplers << std::endl;
            std::cout << "\t\t- Max Per Stage Descriptor Uniform Buffers: " << limits.maxPerStageDescriptorUniformBuffers << std::endl;
            std::cout << "\t\t- Max Per Stage Descriptor Storage Buffers: " << limits.maxPerStageDescriptorStorageBuffers << std::endl;
            std::cout << "\t\t- Max Per Stage Descriptor Sampled Images: " << limits.maxPerStageDescriptorSampledImages << std::endl;
            std::cout << "\t\t- Max Per Stage Descriptor Storage Images: " << limits.maxPerStageDescriptorStorageImages << std::endl;
            std::cout << "\t\t- Max Per Stage Descriptor Input Attachments: " << limits.maxPerStageDescriptorInputAttachments << std::endl;
            std::cout << "\t\t- Max Per Stage Resources: " << limits.maxPerStageResources << std::endl;
            std::cout << "\t\t- Max Descriptor Set Samplers: " << limits.maxDescriptorSetSamplers << std::endl;
            std::cout << "\t\t- Max Descriptor Set Uniform Buffers: " << limits.maxDescriptorSetUniformBuffers << std::endl;
            std::cout << "\t\t- Max Descriptor Set Uniform Buffers Dynamic: " << limits.maxDescriptorSetUniformBuffersDynamic << std::endl;
            std::cout << "\t\t- Max Descriptor Set Storage Buffers: " << limits.maxDescriptorSetStorageBuffers << std::endl;
            std::cout << "\t\t- Max Descriptor Set Storage Buffers Dynamic: " << limits.maxDescriptorSetStorageBuffersDynamic << std::endl;
            std::cout << "\t\t- Max Descriptor Set Sampled Images: " << limits.maxDescriptorSetSampledImages << std::endl;
            std::cout << "\t\t- Max Descriptor Set Storage Images: " << limits.maxDescriptorSetStorageImages << std::endl;
            std::cout << "\t\t- Max Descriptor Set Input Attachments: " << limits.maxDescriptorSetInputAttachments << std::endl;
            std::cout << "\t\t- Max Vertex Input Attributes: " << limits.maxVertexInputAttributes << std::endl;
            std::cout << "\t\t- Max Vertex Input Bindings: " << limits.maxVertexInputBindings << std::endl;
            std::cout << "\t\t- Max Vertex Input Attribute Offset: " << limits.maxVertexInputAttributeOffset << std::endl;
            std::cout << "\t\t- Max Vertex Input Binding Stride: " << limits.maxVertexInputBindingStride << std::endl;
            std::cout << "\t\t- Max Vertex Output Components: " << limits.maxVertexOutputComponents << std::endl;
            std::cout << "\t\t- Max Tessellation Generation Level: " << limits.maxTessellationGenerationLevel << std::endl;
            std::cout << "\t\t- Max Tessellation Patch Size: " << limits.maxTessellationPatchSize << std::endl;
            std::cout << "\t\t- Max Tessellation Control Per Vertex Input Components: " << limits.maxTessellationControlPerVertexInputComponents << std::endl;
            std::cout << "\t\t- Max Tessellation Control Per Vertex Output Components: " << limits.maxTessellationControlPerVertexOutputComponents << std::endl;
            std::cout << "\t\t- Max Tessellation Control Per Patch Output Components: " << limits.maxTessellationControlPerPatchOutputComponents << std::endl;
            std::cout << "\t\t- Max Tessellation Control Total Output Components: " << limits.maxTessellationControlTotalOutputComponents << std::endl;
            std::cout << "\t\t- Max Tessellation Evaluation Input Components: " << limits.maxTessellationEvaluationInputComponents << std::endl;
            std::cout << "\t\t- Max Tessellation Evaluation Output Components: " << limits.maxTessellationEvaluationOutputComponents << std::endl;
            std::cout << "\t\t- Max Geometry Shader Invocations: " << limits.maxGeometryShaderInvocations << std::endl;
            std::cout << "\t\t- Max Geometry Input Components: " << limits.maxGeometryInputComponents << std::endl;
            std::cout << "\t\t- Max Geometry Output Components: " << limits.maxGeometryOutputComponents << std::endl;
            std::cout << "\t\t- Max Geometry Output Vertices: " << limits.maxGeometryOutputVertices << std::endl;
            std::cout << "\t\t- Max Geometry Total Output Components: " << limits.maxGeometryTotalOutputComponents << std::endl;
            std::cout << "\t\t- Max Fragment Input Components: " << limits.maxFragmentInputComponents << std::endl;
            std::cout << "\t\t- Max Fragment Output Attachments: " << limits.maxFragmentOutputAttachments << std::endl;
            std::cout << "\t\t- Max Fragment Dual Src Attachments: " << limits.maxFragmentDualSrcAttachments << std::endl;
            std::cout << "\t\t- Max Fragment Combined Output Resources: " << limits.maxFragmentCombinedOutputResources << std::endl;
            std::cout << "\t\t- Max Compute Shared Memory Size: " << limits.maxComputeSharedMemorySize << std::endl;
            std::cout << "\t\t- Max Compute Work Group Count: (" << limits.maxComputeWorkGroupCount[0] << ", " << limits.maxComputeWorkGroupCount[1] << ", " << limits.maxComputeWorkGroupCount[2] << ")" << std::endl;
            std::cout << "\t\t- Max Compute Work Group Invocations: " << limits.maxComputeWorkGroupInvocations << std::endl;
            std::cout << "\t\t- Max Compute Work Group Size: (" << limits.maxComputeWorkGroupSize[0] << ", " << limits.maxComputeWorkGroupSize[1] << ", " << limits.maxComputeWorkGroupSize[2] << ")" << std::endl;
            std::cout << "\t\t- Sub Pixel Precision Bits: " << limits.subPixelPrecisionBits << std::endl;
            std::cout << "\t\t- Sub Texel Precision Bits: " << limits.subTexelPrecisionBits << std::endl;
            std::cout << "\t\t- Mipmap Precision Bits: " << limits.mipmapPrecisionBits << std::endl;
            std::cout << "\t\t- Max Draw Indexed Index Value: " << limits.maxDrawIndexedIndexValue << std::endl;
            std::cout << "\t\t- Max Draw Indirect Count: " << limits.maxDrawIndirectCount << std::endl;
            std::cout << "\t\t- Max Sampler LOD Bias: " << limits.maxSamplerLodBias << std::endl;
            std::cout << "\t\t- Max Sampler Anisotropy: " << limits.maxSamplerAnisotropy << std::endl;
            std::cout << "\t\t- Max Viewports: " << limits.maxViewports << std::endl;
            std::cout << "\t\t- Max Viewport Dimensions: (" << limits.maxViewportDimensions[0] << ", " << limits.maxViewportDimensions[1] << ")" << std::endl;
            std::cout << "\t\t- Viewport Bounds Range: (" << limits.viewportBoundsRange[0] << ", " << limits.viewportBoundsRange[1] << ")" << std::endl;
            std::cout << "\t\t- Viewport Sub Pixel Bits: " << limits.viewportSubPixelBits << std::endl;
            std::cout << "\t\t- Min Memory Map Alignment: " << limits.minMemoryMapAlignment << std::endl;
            std::cout << "\t\t- Min Texel Buffer Offset Alignment: " << limits.minTexelBufferOffsetAlignment << std::endl;
            std::cout << "\t\t- Min Uniform Buffer Offset Alignment: " << limits.minUniformBufferOffsetAlignment << std::endl;
            std::cout << "\t\t- Min Storage Buffer Offset Alignment: " << limits.minStorageBufferOffsetAlignment << std::endl;
            std::cout << "\t\t- Min Texel Offset: " << limits.minTexelOffset << std::endl;
            std::cout << "\t\t- Max Texel Offset: " << limits.maxTexelOffset << std::endl;
            std::cout << "\t\t- Min Texel Gather Offset: " << limits.minTexelGatherOffset << std::endl;
            std::cout << "\t\t- Max Texel Gather Offset: " << limits.maxTexelGatherOffset << std::endl;
            std::cout << "\t\t- Min Interpolation Offset: " << limits.minInterpolationOffset << std::endl;
            std::cout << "\t\t- Max Interpolation Offset: " << limits.maxInterpolationOffset << std::endl;
            std::cout << "\t\t- Sub Pixel Interpolation Offset Bits: " << limits.subPixelInterpolationOffsetBits << std::endl;
            std::cout << "\t\t- Max Framebuffer Width: " << limits.maxFramebufferWidth << std::endl;
            std::cout << "\t\t- Max Framebuffer Height: " << limits.maxFramebufferHeight << std::endl;
            std::cout << "\t\t- Max Framebuffer Layers: " << limits.maxFramebufferLayers << std::endl;
            std::cout << "\t\t- Framebuffer Color Sample Counts: " << limits.framebufferColorSampleCounts << std::endl;
            std::cout << "\t\t- Framebuffer Depth Sample Counts: " << limits.framebufferDepthSampleCounts << std::endl;
            std::cout << "\t\t- Framebuffer Stencil Sample Counts: " << limits.framebufferStencilSampleCounts << std::endl;
            std::cout << "\t\t- Framebuffer No Attachments Sample Counts: " << limits.framebufferNoAttachmentsSampleCounts << std::endl;
            std::cout << "\t\t- Max Color Attachments: " << limits.maxColorAttachments << std::endl;
            std::cout << "\t\t- Sampled Image Color Sample Counts: " << limits.sampledImageColorSampleCounts << std::endl;
            std::cout << "\t\t- Sampled Image Integer Sample Counts: " << limits.sampledImageIntegerSampleCounts << std::endl;
            std::cout << "\t\t- Sampled Image Depth Sample Counts: " << limits.sampledImageDepthSampleCounts << std::endl;
            std::cout << "\t\t- Sampled Image Stencil Sample Counts: " << limits.sampledImageStencilSampleCounts << std::endl;
            std::cout << "\t\t- Storage Image Sample Counts: " << limits.storageImageSampleCounts << std::endl;
            std::cout << "\t\t- Max Sample Mask Words: " << limits.maxSampleMaskWords << std::endl;
            std::cout << "\t\t- Timestamp Compute And Graphics: " << limits.timestampComputeAndGraphics << std::endl;
            std::cout << "\t\t- Timestamp Period: " << limits.timestampPeriod << std::endl;
            std::cout << "\t\t- Max Clip Distances: " << limits.maxClipDistances << std::endl;
            std::cout << "\t\t- Max Cull Distances: " << limits.maxCullDistances << std::endl;
            std::cout << "\t\t- Max Combined Clip And Cull Distances: " << limits.maxCombinedClipAndCullDistances << std::endl;
            std::cout << "\t\t- Discrete Queue Priorities: " << limits.discreteQueuePriorities << std::endl;
            std::cout << "\t\t- Point Size Range: (" << limits.pointSizeRange[0] << ", " << limits.pointSizeRange[1] << ")" << std::endl;
            std::cout << "\t\t- Line Width Range: (" << limits.lineWidthRange[0] << ", " << limits.lineWidthRange[1] << ")" << std::endl;
            std::cout << "\t\t- Point Size Granularity: " << limits.pointSizeGranularity << std::endl;
            std::cout << "\t\t- Line Width Granularity: " << limits.lineWidthGranularity << std::endl;
            std::cout << "\t\t- Strict Lines: " << limits.strictLines << std::endl;
            std::cout << "\t\t- Standard Sample Locations: " << limits.standardSampleLocations << std::endl;
            std::cout << "\t\t- Optimal Buffer Copy Offset Alignment: " << limits.optimalBufferCopyOffsetAlignment << std::endl;
            std::cout << "\t\t- Optimal Buffer Copy Row Pitch Alignment: " << limits.optimalBufferCopyRowPitchAlignment << std::endl;
            std::cout << "\t\t- Non Coherent Atom Size: " << limits.nonCoherentAtomSize << std::endl;
        }

        void MemoryProperties(const VkPhysicalDevice& device)
        {
            VkPhysicalDeviceMemoryProperties memoryProperties;
            vkGetPhysicalDeviceMemoryProperties(device, &memoryProperties);
            std::cout << "\t  Memory Heaps: " << memoryProperties.memoryHeapCount << std::endl;
            for (uint32_t i = 0; i < memoryProperties.memoryHeapCount; i++)
            {
                std::cout << "\t\t- Heap " << i << ": " << memoryProperties.memoryHeaps[i].size / (1024 * 1024) << " MB"
                          << std::endl;
            }
        }

        void ExtensionCount(const VkPhysicalDevice& device)
        {
            uint32_t extensionCount = 0;
            vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);
            std::cout << "\t  Extensions: " << extensionCount << std::endl;
        }

        void Extensions(const VkPhysicalDevice& device)
        {
            uint32_t extensionCount = 0;
            vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);
            std::vector<VkExtensionProperties> extensions(extensionCount);
            vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, extensions.data());
            for (const auto& extension : extensions)
            {
                std::cout << "\t\t- " << extension.extensionName << " (version " << extension.specVersion << ")"
                          << std::endl;
            }
        }

        void Features(const VkPhysicalDevice& device)
        {
            VkPhysicalDeviceFeatures deviceFeatures;
            vkGetPhysicalDeviceFeatures(device, &deviceFeatures);

            std::cout << "\t  Features:" << std::endl;
            std::cout << "\t\t- Geometry Shader: " << deviceFeatures.geometryShader << std::endl;
            std::cout << "\t\t- Tessellation Shader: " << deviceFeatures.tessellationShader << std::endl;
            std::cout << "\t\t- Multi Viewport: " << deviceFeatures.multiViewport << std::endl;
            std::cout << "\t\t- Sampler Anisotropy: " << deviceFeatures.samplerAnisotropy << std::endl;
            std::cout << "\t\t- Texture Compression ETC2: " << deviceFeatures.textureCompressionETC2 << std::endl;
            std::cout << "\t\t- Texture Compression ASTC_LDR: " << deviceFeatures.textureCompressionASTC_LDR
                      << std::endl;
            std::cout << "\t\t- Texture Compression BC: " << deviceFeatures.textureCompressionBC << std::endl;
            std::cout << "\t\t- Occlusion Query Precise: " << deviceFeatures.occlusionQueryPrecise << std::endl;
            std::cout << "\t\t- Pipeline Statistics Query: " << deviceFeatures.pipelineStatisticsQuery << std::endl;
            std::cout << "\t\t- Vertex Pipeline Stores and Atomics: " << deviceFeatures.vertexPipelineStoresAndAtomics
                      << std::endl;
            std::cout << "\t\t- Fragment Stores and Atomics: " << deviceFeatures.fragmentStoresAndAtomics << std::endl;
            std::cout << "\t\t- Shader Tessellation and Geometry Point Size: "
                      << deviceFeatures.shaderTessellationAndGeometryPointSize << std::endl;
            std::cout << "\t\t- Shader Image Gather Extended: " << deviceFeatures.shaderImageGatherExtended
                      << std::endl;
            std::cout << "\t\t- Shader Storage Image Extended Formats: "
                      << deviceFeatures.shaderStorageImageExtendedFormats << std::endl;
            std::cout << "\t\t- Shader Storage Image Multisample: " << deviceFeatures.shaderStorageImageMultisample
                      << std::endl;
            std::cout << "\t\t- Shader Storage Image Read Without Format: "
                      << deviceFeatures.shaderStorageImageReadWithoutFormat << std::endl;
            std::cout << "\t\t- Shader Storage Image Write Without Format: "
                      << deviceFeatures.shaderStorageImageWriteWithoutFormat << std::endl;
            std::cout << "\t\t- Shader Uniform Buffer Array Dynamic Indexing: "
                      << deviceFeatures.shaderUniformBufferArrayDynamicIndexing << std::endl;
            std::cout << "\t\t- Shader Sampled Image Array Dynamic Indexing: "
                      << deviceFeatures.shaderSampledImageArrayDynamicIndexing << std::endl;
            std::cout << "\t\t- Shader Storage Buffer Array Dynamic Indexing: "
                      << deviceFeatures.shaderStorageBufferArrayDynamicIndexing << std::endl;
            std::cout << "\t\t- Shader Storage Image Array Dynamic Indexing: "
                      << deviceFeatures.shaderStorageImageArrayDynamicIndexing << std::endl;
            std::cout << "\t\t- Shader Clip Distance: " << deviceFeatures.shaderClipDistance << std::endl;
            std::cout << "\t\t- Shader Cull Distance: " << deviceFeatures.shaderCullDistance << std::endl;
            std::cout << "\t\t- Shader Float64: " << deviceFeatures.shaderFloat64 << std::endl;
            std::cout << "\t\t- Shader Int64: " << deviceFeatures.shaderInt64 << std::endl;
            std::cout << "\t\t- Shader Int16: " << deviceFeatures.shaderInt16 << std::endl;
            std::cout << "\t\t- Shader Resource Residency: " << deviceFeatures.shaderResourceResidency << std::endl;
            std::cout << "\t\t- Shader Resource Min LOD: " << deviceFeatures.shaderResourceMinLod << std::endl;
            std::cout << "\t\t- Sparse Binding: " << deviceFeatures.sparseBinding << std::endl;
            std::cout << "\t\t- Sparse Residency Buffer: " << deviceFeatures.sparseResidencyBuffer << std::endl;
            std::cout << "\t\t- Sparse Residency Image2D: " << deviceFeatures.sparseResidencyImage2D << std::endl;
            std::cout << "\t\t- Sparse Residency Image3D: " << deviceFeatures.sparseResidencyImage3D << std::endl;
            std::cout << "\t\t- Sparse Residency2 Samples: " << deviceFeatures.sparseResidency2Samples << std::endl;
            std::cout << "\t\t- Sparse Residency4 Samples: " << deviceFeatures.sparseResidency4Samples << std::endl;
            std::cout << "\t\t- Sparse Residency8 Samples: " << deviceFeatures.sparseResidency8Samples << std::endl;
            std::cout << "\t\t- Sparse Residency16 Samples: " << deviceFeatures.sparseResidency16Samples << std::endl;
            std::cout << "\t\t- Sparse Residency Aliased: " << deviceFeatures.sparseResidencyAliased << std::endl;
            std::cout << "\t\t- Variable Multisample Rate: " << deviceFeatures.variableMultisampleRate << std::endl;
            std::cout << "\t\t- Inherited Queries: " << deviceFeatures.inheritedQueries << std::endl;
        }

        void QueueFamilies(const VkPhysicalDevice& device)
        {
            uint32_t queueFamilyCount = 0;
            vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);
            std::cout << "\t  Queue Families: " << queueFamilyCount << std::endl;

            if (queueFamilyCount == 0)
                return;

            std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
            vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());
            for (int i = 0; i < queueFamilyCount; i++)
            {
                const auto& queueFamily = queueFamilies[i];
                std::cout << "\t\t- Queue Count: " << queueFamily.queueCount << std::endl;
                if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
                    std::cout << "\t\t  VK_QUEUE_GRAPHICS_BIT" << std::endl;
                if (queueFamily.queueFlags & VK_QUEUE_COMPUTE_BIT)
                    std::cout << "\t\t  VK_QUEUE_COMPUTE_BIT" << std::endl;
                if (queueFamily.queueFlags & VK_QUEUE_TRANSFER_BIT)
                    std::cout << "\t\t  VK_QUEUE_TRANSFER_BIT" << std::endl;
                if (queueFamily.queueFlags & VK_QUEUE_SPARSE_BINDING_BIT)
                    std::cout << "\t\t  VK_QUEUE_SPARSE_BINDING_BIT" << std::endl;
                if (queueFamily.queueFlags & VK_QUEUE_PROTECTED_BIT)
                    std::cout << "\t\t  VK_QUEUE_PROTECTED_BIT" << std::endl;

                std::cout << "\t\t  Timestamp Valid Bits: " << queueFamily.timestampValidBits << std::endl;
                std::cout << "\t\t  Min Image Transfer Granularity: (" << queueFamily.minImageTransferGranularity.width
                          << ", " << queueFamily.minImageTransferGranularity.height << ", "
                          << queueFamily.minImageTransferGranularity.depth << ")" << std::endl;
            }
        }

        enum LogScope : uint8_t
        {
            SIMPLE = 0,
            VERSIONS = 1 << 0,
            SPARSE = 1 << 1,
            LIMITS = 1 << 2,
            MEMORY = 1 << 3,
            QUEUES = 1 << 4,
            EXT_COUNT = 1 << 5,
            EXT_ALL = 1 << 6,
            FEATURES = 1 << 7,
        };

        void Properties(const VkPhysicalDevice& device, uint8_t scope_flags = SIMPLE)
        {
            VkPhysicalDeviceProperties deviceProperties;
            vkGetPhysicalDeviceProperties(device, &deviceProperties);
            std::cout << "\t- " << deviceProperties.deviceName << " ("
                      << ToString::PhysicalDeviceVendor(deviceProperties.vendorID) << " - "
                      << ToString::PhysicalDeviceType(deviceProperties.deviceType) << " - "
                      << "ID:" << deviceProperties.deviceID << ")" << std::endl;

            if (scope_flags & VERSIONS)
            {
                std::cout << "\t  API Version: " << VK_VERSION_MAJOR(deviceProperties.apiVersion) << "."
                          << VK_VERSION_MINOR(deviceProperties.apiVersion) << "."
                          << VK_VERSION_PATCH(deviceProperties.apiVersion) << std::endl;
                std::cout << "\t  Driver Version: " << VK_VERSION_MAJOR(deviceProperties.driverVersion) << "."
                          << VK_VERSION_MINOR(deviceProperties.driverVersion) << "."
                          << VK_VERSION_PATCH(deviceProperties.driverVersion) << std::endl;
            }
            if (scope_flags & SPARSE) SparseProperties(deviceProperties.sparseProperties);
            if (scope_flags & LIMITS) DeviceLimits(deviceProperties.limits);
            if (scope_flags & MEMORY) MemoryProperties(device);
            if (scope_flags & QUEUES) QueueFamilies(device);
            if (scope_flags & EXT_COUNT) ExtensionCount(device);
            if (scope_flags & EXT_ALL) Extensions(device);
            if (scope_flags & FEATURES) Features(device);
        }

    } // namespace PhysicalDevice
} // namespace Log

namespace Populate
{
    namespace Instance
    {
        bool LayerProperties()
        {
            std::cout << "Retrieving available Vulkan layer properties." << std::endl;

            uint32_t layerCount;
            if (vkEnumerateInstanceLayerProperties(&layerCount, nullptr) != VK_SUCCESS)
            {
                std::cerr << "Failed to get Vulkan layer count." << std::endl;
                return false;
            }

            availableLayers.resize(layerCount);
            if (vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data()) != VK_SUCCESS)
            {
                std::cerr << "Failed to get Vulkan layer properties." << std::endl;
                return false;
            }

            std::cout << "Retrieved " << availableLayers.size() << " Vulkan layers:" << std::endl;
            for (auto& layer : availableLayers)
                std::cout << "\t-" << layer.layerName << std::endl;

            return true;
        }

        void Layers(std::vector<const char*>& layers)
        {
            for (const auto& prefered_layer : Criteria::Prefered::layers)
            {
                for (const auto& layer : availableLayers)
                {
                    if (strcmp(layer.layerName, prefered_layer) != 0)
                        continue;

                    layers.push_back(layer.layerName);
                    break;
                }
            }
        }

        bool RequiredSDLExtensions(SDL_Window* window, std::vector<const char*>& extensions)
        {
            std::cout << "Retrieving Vulkan window extensions." << std::endl;

            uint32_t extensionCount = 0;
            if (!SDL_Vulkan_GetInstanceExtensions(window, &extensionCount, nullptr))
            {
                std::cerr << "Failed to SDL get Vulkan instance extension count." << std::endl;
                return false;
            }
            if (extensionCount == 0)
            {
                std::cout << "Retrieved 0 Vulkan window extensions." << std::endl;
                return true;
            }

            extensions.resize(extensionCount);
            if (!SDL_Vulkan_GetInstanceExtensions(window, &extensionCount, extensions.data()))
            {
                std::cerr << "Failed to SDL get Vulkan instance extensions." << std::endl;
                return false;
            }

            std::cout << "Retrieved " << extensions.size() << " Vulkan surface extensions:" << std::endl;
            for (const char* extension : extensions)
                std::cout << "\t-" << extension << std::endl;

            return true;
        }
    }

    bool PhysicalDevices(VkInstance& instance, std::vector<VkPhysicalDevice>& physical_devices)
    {
        uint32_t deviceCount = 0;
        if (vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr) != VK_SUCCESS || deviceCount == 0)
        {
            std::cerr << "Failed to find GPUs with Vulkan support." << std::endl;
            return false;
        }

        physical_devices.resize(deviceCount);
        if (vkEnumeratePhysicalDevices(instance, &deviceCount, physical_devices.data()) != VK_SUCCESS)
        {
            std::cerr << "Failed to get Vulkan GPUs." << std::endl;
            return false;
        }

        std::cout << "Retrieved " << deviceCount << " GPUs with Vulkan support:" << std::endl;
        for (auto& device : physical_devices)
            Log::PhysicalDevice::Properties(device);

        return true;
    }

    namespace GraphicsPipeline
    {
        void InputAssemblyState(VkPipelineInputAssemblyStateCreateInfo& create_info)
        {
            create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
            create_info.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
            create_info.primitiveRestartEnable = VK_FALSE;
        }
        void ViewportState(VkPipelineViewportStateCreateInfo& create_info, VkRect2D& scissor, VkViewport& viewport)
        {
            create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
            create_info.viewportCount = 1;
            create_info.pViewports = &viewport;
            create_info.scissorCount = 1;
            create_info.pScissors = &scissor;
        }
        void RasterizationState(VkPipelineRasterizationStateCreateInfo& create_info,
                                float lineWidth, bool depthBiasEnabled, float depthBiasConstantFactor,
                                float depthBiasClamp, float depthBiasSlopeFactor)
        {
            create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
            create_info.depthClampEnable = VK_FALSE; // VK_TRUE requires enabling a GPU feature
            create_info.rasterizerDiscardEnable = VK_FALSE;
            create_info.polygonMode = VK_POLYGON_MODE_FILL;
            create_info.cullMode = VK_CULL_MODE_BACK_BIT;
            create_info.frontFace = VK_FRONT_FACE_CLOCKWISE;

            create_info.lineWidth = lineWidth;
            create_info.depthBiasEnable = depthBiasEnabled ? VK_TRUE : VK_FALSE;
            create_info.depthBiasConstantFactor = depthBiasConstantFactor;
            create_info.depthBiasClamp = depthBiasClamp;
            create_info.depthBiasSlopeFactor = depthBiasSlopeFactor;
        }
        void MultisampleState(VkPipelineMultisampleStateCreateInfo& create_info)
        {
            create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
            create_info.sampleShadingEnable = VK_FALSE;
            create_info.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
            create_info.minSampleShading = 1.0f;
            create_info.pSampleMask = nullptr;
            create_info.alphaToCoverageEnable = VK_FALSE;
            create_info.alphaToOneEnable = VK_FALSE;
        }
        void ColorBlendState(VkPipelineColorBlendStateCreateInfo& create_info,
                             VkPipelineColorBlendAttachmentState& attachment)
        {
            attachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
                                                  VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
            attachment.blendEnable = VK_FALSE;

            create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
            create_info.logicOpEnable = VK_FALSE;
            create_info.logicOp = VK_LOGIC_OP_COPY;
            create_info.attachmentCount = 1;
            create_info.pAttachments = &attachment;
            create_info.blendConstants[0] = 0.0f;
            create_info.blendConstants[1] = 0.0f;
            create_info.blendConstants[2] = 0.0f;
            create_info.blendConstants[3] = 0.0f;
        }

        void DepthStencilState(VkPipelineDepthStencilStateCreateInfo& create_info)
        {
            /* TODO: Populate Stencil State Data
            
                create_info.sType;
                create_info.flags;
                create_info.depthTestEnable;
                create_info.depthWriteEnable;
                create_info.depthCompareOp;
                create_info.depthBoundsTestEnable;
                create_info.stencilTestEnable;
                create_info.front;
                create_info.back;
                create_info.minDepthBounds;
                create_info.maxDepthBounds;
            */
        }
    }

} // namespace Populate

struct LogicalDevice
{
    VkDevice logical_device = VK_NULL_HANDLE;
    VkPhysicalDevice physical_device = VK_NULL_HANDLE;

    enum QueueFamilySetup
    {
        UNSET,
        SINGLE,
        SAME_GRAPHICS_PRESENT,
        SEPARATE

    } queue_setup = UNSET;

    uint32_t graphics_family = 0;
    uint32_t transfer_family = 0;
    uint32_t present_family = 0;

    VkPhysicalDeviceMemoryProperties mem_properties;

    bool Create(VkInstance instance, VkSurfaceKHR surface)
    {
        std::cout << "Finding suitable Vulkan Physical Devices." << std::endl;
        std::vector<VkPhysicalDevice> physical_devices{};
        if (!Populate::PhysicalDevices(instance, physical_devices))
            return false;

        for (size_t i = 0; i < physical_devices.size(); ++i)
        {
            physical_device = physical_devices[i];
            if (HasValidType() && HasRequiredFeatures() && SupportsRequiredExtensions() &&
                HasValidQueueFamiliesWithKHR(surface) && HasRequiredSurfaceFormat(surface) &&
                HasRequiredPresentMode(surface) && CorrectCreation())
            {
                std::cout << "Created Logical Device from suitable Vulkan Physical Device: " << i << std::endl;
                vkGetPhysicalDeviceMemoryProperties(physical_device, &mem_properties);
                return true;
            }
        }

        std::cerr << "Failed to find a suitable GPU." << std::endl;
        return false;
    }
    void Delete()
    {
        if (logical_device != VK_NULL_HANDLE)
            vkDestroyDevice(logical_device, allocation_callbacks);
    }

    VkQueue GetGraphicsQueue() const
    {
        VkQueue queue;
        vkGetDeviceQueue(logical_device, graphics_family, 0, &queue);
        return queue;
    }

    bool HasRequiredFeatures()
    {
        VkPhysicalDeviceFeatures deviceFeatures;
        vkGetPhysicalDeviceFeatures(physical_device, &deviceFeatures);

        const std::vector<std::pair<uint8_t, VkBool32>> requiredFeatures = {
            {DeviceFeature::GEOMETRY_SHADER, deviceFeatures.geometryShader},
            {DeviceFeature::TESSELLATION_SHADER, deviceFeatures.tessellationShader},
            {DeviceFeature::SAMPLER_ANISOTROPY, deviceFeatures.samplerAnisotropy}};

        for (const auto& [feature, isSupported] : requiredFeatures)
            if ((Criteria::Required::features & feature) && !isSupported)
                return false;

        return true;
    }

    bool HasValidType()
    {
        VkPhysicalDeviceProperties deviceProperties;
        vkGetPhysicalDeviceProperties(physical_device, &deviceProperties);
        return deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU;
    }

    bool SupportsRequiredExtensions()
    {
        uint32_t extensionCount;
        vkEnumerateDeviceExtensionProperties(physical_device, nullptr, &extensionCount, nullptr);
        std::vector<VkExtensionProperties> availableExtensions(extensionCount);
        vkEnumerateDeviceExtensionProperties(physical_device, nullptr, &extensionCount, availableExtensions.data());

        for (const char* required : Criteria::Required::extensions)
        {
            bool found = false;
            for (const auto& extension : availableExtensions)
            {
                if (strcmp(required, extension.extensionName) != 0)
                    continue;

                found = true;
                break;
            }
            if (!found)
                return false;
        }
        return true;
    }

    bool HasValidQueueFamiliesWithKHR(VkSurfaceKHR surface)
    {
        uint32_t queue_family_count = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &queue_family_count, nullptr);
        std::vector<VkQueueFamilyProperties> queue_families(queue_family_count);
        vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &queue_family_count, queue_families.data());

        // Get Families' Layout
        std::set<uint32_t> graphics_families;
        std::set<uint32_t> transfer_families;
        std::set<uint32_t> present_families;
        std::set<uint32_t> fullsupport_families;
        for (uint32_t family_index = 0; family_index < queue_family_count; ++family_index)
        {
            const auto& queueFamily = queue_families[family_index];

            bool has_graphics = queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT;
            bool has_transfer = queueFamily.queueFlags & VK_QUEUE_TRANSFER_BIT;
            VkBool32 present_support = false;

            if (vkGetPhysicalDeviceSurfaceSupportKHR(physical_device, family_index, surface, &present_support) !=
                VK_SUCCESS)
            {
                std::cerr << "Failed to retrieve Physical Device Surface KHR Support!" << std::endl;
                return false;
            }

            if (has_graphics)
                graphics_families.insert(family_index);

            if (has_transfer)
                transfer_families.insert(family_index);

            if (present_support == VK_TRUE)
                present_families.insert(family_index);

            if (has_graphics && has_transfer && present_support == VK_TRUE)
                fullsupport_families.insert(family_index);
        }

        if (graphics_families.empty() || transfer_families.empty() || present_families.empty())
            return false;

        switch (fullsupport_families.size())
        {
            case 0: // Rendering will requiere different queue families
                break;
            case 1:
                queue_setup = SINGLE;
                graphics_family = transfer_family = present_family = *fullsupport_families.begin();
                return true;
            default:
                queue_setup = SINGLE;
                uint32_t best_family = *fullsupport_families.begin();
                uint32_t max_width = 0;
                for (uint32_t family_index : fullsupport_families)
                {
                    const VkExtent3D& minImageTG = queue_families[family_index].minImageTransferGranularity;
                    if (minImageTG.width <= max_width)
                        continue;

                    best_family = family_index;
                    max_width = minImageTG.width;
                }
                graphics_family = transfer_family = present_family = best_family;
                return true;
        }

        // Prioritize queue families with graphics & present
        for (auto graphics_fam : graphics_families)
        {
            for (auto present_fam : present_families)
            {
                if (graphics_fam == present_fam)
                {
                    queue_setup = SAME_GRAPHICS_PRESENT;
                    graphics_family = present_family = graphics_fam;
                    transfer_family = *transfer_families.begin();
                    return true;
                }
            }
        }

        queue_setup = SEPARATE;
        graphics_family = *graphics_families.begin();
        transfer_family = *transfer_families.begin();
        transfer_family = *present_families.begin();
        return true;
    }

    bool HasRequiredSurfaceFormat(VkSurfaceKHR surface)
    {
        // Choose the surface format
        uint32_t formatCount;
        if (vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device, surface, &formatCount, nullptr) != VK_SUCCESS)
        {
            std::cerr << "Failed to get surface formats count!" << std::endl;
            return false;
        }

        std::vector<VkSurfaceFormatKHR> formats(formatCount);
        if (vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device, surface, &formatCount, formats.data()) != VK_SUCCESS)
        {
            std::cerr << "Failed to get surface formats!" << std::endl;
            return false;
        }

        for (const auto& availableFormat : formats)
        {
            if (availableFormat.format == Criteria::Required::surface_format.format &&
                availableFormat.colorSpace == Criteria::Required::surface_format.colorSpace)
                return true;
        }

        std::cerr << "Device doesn't support required surface format!" << std::endl;
        return false;
    }

    bool HasRequiredPresentMode(VkSurfaceKHR surface)
    {
        uint32_t mode_count;
        if (vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device, surface, &mode_count, nullptr) != VK_SUCCESS)
        {
            std::cerr << "Failed to get present modes count!" << std::endl;
            return false;
        }
        std::vector<VkPresentModeKHR> supported_modes(mode_count);
        if (vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device, surface, &mode_count, supported_modes.data()) !=
            VK_SUCCESS)
        {
            std::cerr << "Failed to get present modes!" << std::endl;
            return false;
        }

        for (const auto& mode : supported_modes)
            if (mode == Criteria::Required::present_mode)
                return true;

        std::cerr << "Device doesn't support required surface present mode!" << std::endl;
        return false;
    }

    bool CorrectCreation()
    {
        VkDeviceCreateInfo deviceCreateInfo{};
        deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

        VkDeviceQueueCreateInfo queueCreateInfo{};
        queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo.queueFamilyIndex = graphics_family;
        queueCreateInfo.queueCount = 1;
        float queuePriority = 1.0f;
        queueCreateInfo.pQueuePriorities = &queuePriority;
        deviceCreateInfo.queueCreateInfoCount = 1;
        deviceCreateInfo.pQueueCreateInfos = &queueCreateInfo;

        // TODO: Physical Device Features 
        //VkPhysicalDeviceFeatures features;
        //vkGetPhysicalDeviceFeatures(physical_device, &features);
        //deviceCreateInfo.pEnabledFeatures = &features;

        deviceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(Criteria::Required::extensions.size());
        deviceCreateInfo.ppEnabledExtensionNames = Criteria::Required::extensions.data();

        return vkCreateDevice(physical_device, &deviceCreateInfo, allocation_callbacks, &logical_device) == VK_SUCCESS;
    }
};

struct Swapchain
{
    VkSwapchainKHR swapchain = VK_NULL_HANDLE;

    // State
    VkExtent2D extent;
    VkSurfaceTransformFlagBitsKHR surface_transform;

    bool Create(LogicalDevice& device, VkSurfaceKHR surface, const VkSurfaceCapabilitiesKHR& capabilities,
                const VkExtent2D& window_size, VkSwapchainKHR oldSwapchain = VK_NULL_HANDLE)
    {
        VkSwapchainCreateInfoKHR createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        createInfo.surface = surface;
        createInfo.minImageCount = Criteria::Swapchain::MinImageCount(capabilities);
        createInfo.imageFormat = Criteria::Required::surface_format.format;
        createInfo.imageColorSpace = Criteria::Required::surface_format.colorSpace;
        createInfo.imageExtent = extent = Criteria::Swapchain::Extent(capabilities, window_size);
        createInfo.imageArrayLayers = 1;
        createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

        uint32_t queueFamilyIndices[] = {device.graphics_family, device.present_family};
        if (device.graphics_family != device.present_family)
        {
            createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
            createInfo.queueFamilyIndexCount = 2;
            createInfo.pQueueFamilyIndices = queueFamilyIndices;
        }
        else
        {
            createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
            createInfo.queueFamilyIndexCount = 0;
            createInfo.pQueueFamilyIndices = nullptr;
        }

        createInfo.preTransform = surface_transform = capabilities.currentTransform;
        createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
        createInfo.presentMode = Criteria::Required::present_mode;
        createInfo.clipped = VK_TRUE;
        createInfo.oldSwapchain = oldSwapchain;

        std::cout << "Creating swapchain." << std::endl;
        if (vkCreateSwapchainKHR(device.logical_device, &createInfo, allocation_callbacks, &swapchain) != VK_SUCCESS)
        {
            std::cerr << "Failed to create swapchain!" << std::endl;
            return false;
        }

        std::cout << "Swapchain created." << std::endl;
        return true;
    }

    
    enum CapabilityChanges : uint8_t
    {
        None = 0,
        Transform = 1 << 0,
        Extent = 1 << 1
    };

    uint8_t GetChanges(const VkSurfaceCapabilitiesKHR& new_capabilities)
    {
        uint8_t changes = 0;
        if (new_capabilities.currentTransform != surface_transform)
        {
            std::cout << "Surface transformation changed. ";
            changes |= CapabilityChanges::Transform;
        }
        if (new_capabilities.currentExtent.width != extent.width ||
            new_capabilities.currentExtent.height != extent.height)
        {
            std::cout << "Surface extent changed. ";
            changes |= CapabilityChanges::Extent;
        }

        return changes;
    }
};

struct DynamicState
{
    // Viewport
    VkExtent2D swapchain_extent;
    VkViewport viewport;
    VkRect2D scissor;

    // Rasterization
    float depth_bias[3] = {0.f, 0.f, 0.f}; // { ConstantFactor, Clamp, SlopeFactor }
    float line_width = 1.f;                // width > 1.f requires "wideLines" GPU feature

    // Blend
    float blend_constants[4] = {0.f, 0.f, 0.f, 0.f};

    // Stencil
    struct StencilFaceConfig
    {
        VkStencilFaceFlags face_mask;
        uint32_t mask; // or reference
    };
    float depth_bounds[2] = {0.f, 1.f}; // { min, max }
    StencilFaceConfig compare = {VK_STENCIL_FACE_FRONT_AND_BACK, 0xFF};
    StencilFaceConfig write = {VK_STENCIL_FACE_FRONT_AND_BACK, 0xFF};
    StencilFaceConfig reference = {VK_STENCIL_FACE_FRONT_AND_BACK, 0};

    enum Flags : uint16_t
    {
        NONE = 0,

        // Viewport
        VIEWPORT = 1 << 0,
        SCISSOR = 1 << 1,

        // Rasterization
        DEPTH_BIAS_ENABLED = 1 << 2,
        LINE_WIDTH = 1 << 3,
        DEPTH_BIAS = 1 << 4,

        // Color Blend
        BLEND_CONSTANTS = 1 << 5,

        // Stencil
        DEPTH_BOUNDS = 1 << 6,
        STENCIL_COMPARE_MASK = 1 << 7,
        STENCIL_WRITE_MASK = 1 << 8,
        STENCIL_REFERENCE = 1 << 9
    };
    uint16_t current_flags = 0;
    uint16_t update_flags = 0;
    bool requires_recreation = false;

    void Setup(const VkExtent2D& extent, uint16_t flags = VIEWPORT | SCISSOR)
    {
        current_flags = flags;
        swapchain_extent = extent;
        scissor = {{0, 0}, extent};
        viewport = {0.f, 0.f, static_cast<float>(extent.width), static_cast<float>(extent.height), 0.f, 1.f};
    }

    void PopulatePipelineCreateInfo(VkGraphicsPipelineCreateInfo& pipelineInfo,
                                    VkPipelineViewportStateCreateInfo& viewportState,
                                    VkPipelineDepthStencilStateCreateInfo& depthStencilState,
                                    VkPipelineRasterizationStateCreateInfo& rasterizer,
                                    VkPipelineDynamicStateCreateInfo& dynamicState,
                                    std::vector<VkDynamicState>& states)
    {
        requires_recreation = false;

        // Setup Rasterization State Create Info
        Populate::GraphicsPipeline::RasterizationState(rasterizer, line_width, current_flags & DEPTH_BIAS_ENABLED,
                                                       depth_bias[0], depth_bias[1], depth_bias[2]);
        pipelineInfo.pRasterizationState = &rasterizer;

        if (current_flags == 0)
        {
            Populate::GraphicsPipeline::ViewportState(viewportState, scissor, viewport);
            pipelineInfo.pViewportState = &viewportState;
            pipelineInfo.pDepthStencilState = nullptr;
            pipelineInfo.pDynamicState = nullptr;
            return;
        }

        // Fill Dynamic States' Array
        states.clear();
        if (current_flags & VIEWPORT) states.push_back(VK_DYNAMIC_STATE_VIEWPORT);
        if (current_flags & SCISSOR) states.push_back(VK_DYNAMIC_STATE_SCISSOR);
        if (current_flags & LINE_WIDTH) states.push_back(VK_DYNAMIC_STATE_LINE_WIDTH);
        if (current_flags & DEPTH_BIAS) states.push_back(VK_DYNAMIC_STATE_DEPTH_BIAS);
        if (current_flags & BLEND_CONSTANTS) states.push_back(VK_DYNAMIC_STATE_BLEND_CONSTANTS);
        if (current_flags & DEPTH_BOUNDS) states.push_back(VK_DYNAMIC_STATE_DEPTH_BOUNDS);
        if (current_flags & STENCIL_COMPARE_MASK) states.push_back(VK_DYNAMIC_STATE_STENCIL_COMPARE_MASK);
        if (current_flags & STENCIL_WRITE_MASK) states.push_back(VK_DYNAMIC_STATE_STENCIL_WRITE_MASK);
        if (current_flags & STENCIL_REFERENCE) states.push_back(VK_DYNAMIC_STATE_STENCIL_REFERENCE);

        // Setup Dynamic State Create Info
        dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
        dynamicState.dynamicStateCount = static_cast<uint32_t>(states.size());
        dynamicState.pDynamicStates = states.data();
        pipelineInfo.pDynamicState = &dynamicState;

        // Setup Viewport State Create Info
        if (current_flags & VIEWPORT && current_flags & SCISSOR)
        {
            pipelineInfo.pViewportState = nullptr;
            update_flags |= VIEWPORT | SCISSOR;
        }
        else
        {
            Populate::GraphicsPipeline::ViewportState(viewportState, scissor, viewport);
            pipelineInfo.pViewportState = &viewportState;
        }

        // Setup Depth Stencil State Create Info
        if (current_flags & (DEPTH_BOUNDS | STENCIL_COMPARE_MASK | STENCIL_WRITE_MASK | STENCIL_REFERENCE))
        {
            Populate::GraphicsPipeline::DepthStencilState(depthStencilState);
            pipelineInfo.pDepthStencilState = &depthStencilState;
        }
        else
            pipelineInfo.pDepthStencilState = nullptr;
    }

    void OnSwapchainExtentChanged(const VkExtent2D& next_extent)
    {
        if (swapchain_extent.width == next_extent.width &&
            swapchain_extent.height == next_extent.height)
            return;

        // New extent linear interpolation
        float width_ratio = static_cast<float>(next_extent.width) / swapchain_extent.width;
        float height_ratio = static_cast<float>(next_extent.height) / swapchain_extent.height;

        UpdateViewport({
            viewport.x * width_ratio, viewport.y * height_ratio, viewport.width * width_ratio,
                                      viewport.height * height_ratio, viewport.minDepth, viewport.maxDepth});
        UpdateScissor({{static_cast<int32_t>(scissor.offset.x * width_ratio),
                        static_cast<int32_t>(scissor.offset.y * height_ratio)},
                       {static_cast<uint32_t>(scissor.extent.width * width_ratio),
                        static_cast<uint32_t>(scissor.extent.height * height_ratio)}});
    }

    void UpdateViewport(const VkViewport& next_viewport)
    {
        if (viewport.x == next_viewport.x &&
            viewport.y == next_viewport.y &&
            viewport.width == next_viewport.width &&
            viewport.height == next_viewport.height &&
            viewport.minDepth == next_viewport.minDepth &&
            viewport.maxDepth == next_viewport.maxDepth)
            return;

        viewport = next_viewport;
        update_flags |= VIEWPORT;
        bool has_dyn_viewport = current_flags & VIEWPORT;
        requires_recreation |= !has_dyn_viewport;
    }

    void UpdateScissor(const VkRect2D& next_scissor)
    {
        if (scissor.offset.x == next_scissor.offset.x &&
            scissor.offset.y == next_scissor.offset.y &&
            scissor.extent.width == next_scissor.extent.width &&
            scissor.extent.height == next_scissor.extent.height)
            return;

        scissor = next_scissor;
        update_flags |= SCISSOR;
        bool has_dyn_scissor = current_flags & SCISSOR;
        requires_recreation |= !has_dyn_scissor;
    }

    void Update(VkCommandBuffer commandBuffer)
    {
        if (update_flags == 0) return;
        if (update_flags & VIEWPORT) vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
        if (update_flags & SCISSOR) vkCmdSetScissor(commandBuffer, 0, 1, &scissor);
        if (update_flags & LINE_WIDTH) vkCmdSetLineWidth(commandBuffer, line_width);
        if (update_flags & DEPTH_BIAS) vkCmdSetDepthBias(commandBuffer, depth_bias[0], depth_bias[1], depth_bias[2]);
        if (update_flags & BLEND_CONSTANTS) vkCmdSetBlendConstants(commandBuffer, blend_constants);
        if (update_flags & DEPTH_BOUNDS) vkCmdSetDepthBounds(commandBuffer, depth_bounds[0], depth_bounds[1]);
        if (update_flags & STENCIL_COMPARE_MASK) vkCmdSetStencilCompareMask(commandBuffer, compare.face_mask, compare.mask);
        if (update_flags & STENCIL_WRITE_MASK) vkCmdSetStencilWriteMask(commandBuffer, write.face_mask, write.mask);
        if (update_flags & STENCIL_REFERENCE) vkCmdSetStencilReference(commandBuffer, reference.face_mask, reference.mask);
        update_flags = 0;
    }
};

struct Buffer
{
    uint32_t count = 0;
    VkBuffer buffer = VK_NULL_HANDLE;
    VkDeviceMemory memory = VK_NULL_HANDLE;

    template <typename T>
    bool CreateFromStagedCopy(VkDevice logical_device, VkQueue queue,
                              const VkPhysicalDeviceMemoryProperties& mem_properties, VkCommandPool commandPool,
                              VkBufferUsageFlagBits usage_flags, const std::vector<T>& vector,
                              VkDeviceSize element_size)
    {
        count = static_cast<uint32_t>(vector.size());
        VkDeviceSize buffer_size = element_size * count;
        Buffer staging_buffer;
        if (!staging_buffer.Create(logical_device, mem_properties, buffer_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                                   VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT))
        {
            std::cerr << "Failed to create Staging Buffer." << std::endl;
            return false;
        }

        if (!staging_buffer.FillData(logical_device, vector.data(), buffer_size))
        {
            std::cerr << "Failed to copy data to Staging Buffer." << std::endl;
            return false;
        }
        if (!Create(logical_device, mem_properties, buffer_size, VK_BUFFER_USAGE_TRANSFER_DST_BIT | usage_flags,
                    VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT))
        {
            std::cerr << "Failed to create new Buffer." << std::endl;
            return false;
        }
        if (!staging_buffer.CopyTo(logical_device, buffer, buffer_size, queue, commandPool))
            return false;

        staging_buffer.Clear(logical_device);
        return true;
    }

    bool Create(VkDevice device, const VkPhysicalDeviceMemoryProperties& mem_properties, VkDeviceSize size,
                VkBufferUsageFlags usage, VkMemoryPropertyFlags properties)
    {
        VkBufferCreateInfo buffer_info{};
        buffer_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        buffer_info.size = size;
        buffer_info.usage = usage;
        buffer_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        if (vkCreateBuffer(device, &buffer_info, nullptr, &buffer) != VK_SUCCESS)
        {
            std::cerr << "Failed to create buffer!" << std::endl;
            return false;
        }

        VkMemoryRequirements mem_requirements;
        vkGetBufferMemoryRequirements(device, buffer, &mem_requirements);

        VkMemoryAllocateInfo alloc_info{};
        alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        alloc_info.allocationSize = mem_requirements.size;

        bool found_valid_mem_type = false;
        for (uint32_t i = 0; i < mem_properties.memoryTypeCount; i++)
        {
            if ((mem_requirements.memoryTypeBits & (1 << i)) &&
                (mem_properties.memoryTypes[i].propertyFlags & properties) == properties)
            {
                found_valid_mem_type = true;
                alloc_info.memoryTypeIndex = i;
                break;
            }
        }
        if (!found_valid_mem_type)
        {
            std::cerr << "Failed to find suitable memory type!" << std::endl;
            return false;
        }

        if (vkAllocateMemory(device, &alloc_info, nullptr, &memory) != VK_SUCCESS)
        {
            std::cerr << "Failed to allocate buffer memory!" << std::endl;
            return false;
        }

        if (vkBindBufferMemory(device, buffer, memory, 0) != VK_SUCCESS)
        {
            std::cerr << "Failed to bind buffer memory!" << std::endl;
            return false;
        }

        return true;
    }

    void Clear(VkDevice logical_device)
    {
        if (buffer != VK_NULL_HANDLE)
            vkDestroyBuffer(logical_device, buffer, allocation_callbacks);

        if (memory != VK_NULL_HANDLE)
            vkFreeMemory(logical_device, memory, allocation_callbacks);
    }

    bool CreateDescriptorSet(VkDevice logical_device, VkDescriptorPool& descriptorPool,
                             VkDescriptorSetLayout& descriptorSetLayout, VkDeviceSize size,
                             VkDescriptorSet& descriptorSet)
    {
        // Create Descriptor Set
        VkDescriptorSetAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocInfo.descriptorPool = descriptorPool;
        allocInfo.descriptorSetCount = 1;
        allocInfo.pSetLayouts = &descriptorSetLayout;

        if (vkAllocateDescriptorSets(logical_device, &allocInfo, &descriptorSet) != VK_SUCCESS)
        {
            std::cerr << "Failed to allocate descriptor set!" << std::endl;
            return false;
        }

        // Update Descriptor Set
        VkDescriptorBufferInfo bufferInfo = {buffer, 0, size};
        VkWriteDescriptorSet descriptorWrite{};
        descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrite.dstSet = descriptorSet;
        descriptorWrite.dstBinding = 0;
        descriptorWrite.dstArrayElement = 0;
        descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descriptorWrite.descriptorCount = 1;
        descriptorWrite.pBufferInfo = &bufferInfo;
        vkUpdateDescriptorSets(logical_device, 1, &descriptorWrite, 0, nullptr);
        return true;
    }

    bool FillData(VkDevice device, const void* source, VkDeviceSize size) const
    {
        void* tmp;
        if (vkMapMemory(device, memory, 0, size, 0, &tmp) != VK_SUCCESS)
        {
            std::cerr << "Failed to map buffer data for copy operation." << std::endl;
            return false;
        }
        memcpy(tmp, source, static_cast<size_t>(size));
        vkUnmapMemory(device, memory);
        return true;
    }

    bool CopyTo(VkDevice device, VkBuffer dst_buffer, VkDeviceSize size, VkQueue queue, VkCommandPool commandPool) const
    {
        VkCommandBufferAllocateInfo alloc_info{};
        alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        alloc_info.commandPool = commandPool;
        alloc_info.commandBufferCount = 1;

        VkCommandBuffer command_buffer;
        if (vkAllocateCommandBuffers(device, &alloc_info, &command_buffer) != VK_SUCCESS)
        {
            std::cerr << "Failed to allocate command buffer in Buffer::CopyTo(...)." << std::endl;
            return false;
        }

        VkCommandBufferBeginInfo begin_info{};
        begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

        if (vkBeginCommandBuffer(command_buffer, &begin_info) != VK_SUCCESS)
        {
            std::cerr << "Failed to begin command buffer in Buffer::CopyTo(...)." << std::endl;
            return false;
        }

        VkBufferCopy copy_region{};
        copy_region.size = size;
        vkCmdCopyBuffer(command_buffer, buffer, dst_buffer, 1, &copy_region);

        if (vkEndCommandBuffer(command_buffer) != VK_SUCCESS)
        {
            std::cerr << "Failed to end command buffer in Buffer::CopyTo(...)." << std::endl;
            return false;
        }

        VkSubmitInfo submit_info{};
        submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submit_info.commandBufferCount = 1;
        submit_info.pCommandBuffers = &command_buffer;

        if (vkQueueSubmit(queue, 1, &submit_info, VK_NULL_HANDLE) != VK_SUCCESS)
        {
            std::cerr << "Failed to submit command buffer to queue in Buffer::CopyTo(...)." << std::endl;
            return false;
        }

        if (vkQueueWaitIdle(queue) != VK_SUCCESS)
        {
            std::cerr << "Failed to wait for idle queue in Buffer::CopyTo(...)." << std::endl;
            return false;
        }

        vkFreeCommandBuffers(device, commandPool, 1, &command_buffer);
        return true;
    }
};

struct ModelData
{
    Buffer vertex{};
    Buffer index{};
    VkDeviceSize offset = 0;

    std::vector<Buffer> uniforms{};
    std::vector<VkDescriptorSet> descriptor_sets{};

    bool Create(const LogicalDevice& device, VkCommandPool commandPool, VkDescriptorPool& descriptorPool,
                VkDescriptorSetLayout& descriptorSetLayout, const std::vector<Shaders::Vertex>& vertices,
                const std::vector<uint32_t>& indices)
    {
        VkQueue queue = device.GetGraphicsQueue();
        if (!vertex.CreateFromStagedCopy(device.logical_device, queue, device.mem_properties, commandPool,
                                 VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, vertices, sizeof(vertices[0])))
        {
            std::cerr << "Failed to create vertex buffer!" << std::endl;
            return false;
        }
        if (!index.CreateFromStagedCopy(device.logical_device, queue, device.mem_properties, commandPool,
                             VK_BUFFER_USAGE_INDEX_BUFFER_BIT, indices, sizeof(indices[0])))
        {
            std::cerr << "Failed to create index buffer!" << std::endl;
            return false;
        }

        VkDeviceSize size = sizeof(Shaders::TransfomMatrices);
        uniforms.push_back({});
        if (!uniforms[0].Create(device.logical_device, device.mem_properties, size,
                            VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT))
        {
            std::cerr << "Failed to create <TransfomMatrices> Uniform Buffer!" << std::endl;
            return false;
        }

        descriptor_sets.push_back({});
        if (!uniforms[0].CreateDescriptorSet(device.logical_device, descriptorPool, descriptorSetLayout, size,
                                             descriptor_sets[0]))
        {
            std::cerr << "Failed to create <TransfomMatrices> Descriptor Set!" << std::endl;
            return false;
        }

        return true;
    }

    void Destroy(VkDevice logical_device)
    {
        vertex.Clear(logical_device);
        index.Clear(logical_device);

        for (auto& uniform : uniforms)
            uniform.Clear(logical_device);
    }

    bool UpdateAndBindUniformBuffers(VkDevice logical_device, const std::vector<std::pair<void*, VkDeviceSize>>& uniform_data,
                                     VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout) const
    {
        const auto count = uniforms.size();
        if (count != uniform_data.size())
        {
            std::cerr << "Error: Supplied " << uniform_data.size() << "/" << count << " uniforms for model!" << std::endl;
            return false;
        }

        // Update Uniforms' Data
        for (int i = 0; i < count; i++)
        {
            const auto& data = uniform_data[i];
            if (!uniforms[i].FillData(logical_device, data.first, data.second))
                return false;
        }

        // Bind Uniforms' Descriptor Sets
        vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0,
                                static_cast<uint32_t>(descriptor_sets.size()), descriptor_sets.data(), 0, nullptr);

        return true;
    }
};

struct GraphicPipeline
{
    VkDevice logical_device = VK_NULL_HANDLE;
    VkPipeline graphicsPipeline = VK_NULL_HANDLE;

    // State
    VkClearColorValue clear_color = {0.f, 0.f, 0.f, 1.f};
    DynamicState dynamic_state{};

    // Swapchain
    Swapchain swapchain{};
    std::vector<VkImage> images{};
    std::vector<VkImageView> image_views{};  // Image views for swapchain images.
    std::vector<VkFramebuffer> framebuffers{}; // Framebuffers for swapchain images.

    // Commands
    VkCommandPool commandPool = VK_NULL_HANDLE;
    VkCommandBuffer commandBuffer = VK_NULL_HANDLE;

    // Semaphores
    VkSemaphore imageAvailableSemaphore = VK_NULL_HANDLE;
    VkSemaphore renderFinishedSemaphore = VK_NULL_HANDLE;

    VkRenderPass renderPass = VK_NULL_HANDLE; // Describes rendering operations.
    VkPipelineLayout pipelineLayout = VK_NULL_HANDLE; // Manages shaders and resources.

    VkDescriptorSetLayout descriptorSetLayout = VK_NULL_HANDLE;
    VkDescriptorPool descriptorPool = VK_NULL_HANDLE;

    // Graphic Pipeline Creation

    bool Create(LogicalDevice& device, VkSurfaceKHR surface, VkSurfaceCapabilitiesKHR& surface_capabilities,
                const VkExtent2D& window_size)
    {
        if (!swapchain.Create(device, surface, surface_capabilities, window_size))
        {
            std::cerr << "Failed to create swapchain for Graphic Pipeline." << std::endl;
            return false;
        }

        logical_device = device.logical_device;
        dynamic_state.Setup(swapchain.extent);

        if (!RetrieveImages() ||
            !CreateCommandPool(device.graphics_family) ||
            !CreateCommandBuffer() ||
            !CreateSemaphores() ||
            !CreateRenderPass() ||
            !CreateDescriptorSetLayout() ||
            !CreatePipelineLayout() ||
            !CreateGraphicsPipeline())
        {
            std::cerr << "Failed to create Graphic Pipeline." << std::endl;
            return false;
        }

        std::cout << "Graphic Pipeline created." << std::endl;
        return CreateDescriptorPool();
    }

    void Delete()
    {
        if (pipelineLayout != VK_NULL_HANDLE)
            vkDestroyPipelineLayout(logical_device, pipelineLayout, allocation_callbacks);
        if (renderPass != VK_NULL_HANDLE)
            vkDestroyRenderPass(logical_device, renderPass, allocation_callbacks);

        for (auto framebuffer : framebuffers)
            vkDestroyFramebuffer(logical_device, framebuffer, allocation_callbacks);
        for (auto imageView : image_views)
            vkDestroyImageView(logical_device, imageView, allocation_callbacks);

        if (swapchain.swapchain != VK_NULL_HANDLE)
            vkDestroySwapchainKHR(logical_device, swapchain.swapchain, allocation_callbacks);
        if (commandPool != VK_NULL_HANDLE)
            vkDestroyCommandPool(logical_device, commandPool, allocation_callbacks);

        if (imageAvailableSemaphore != VK_NULL_HANDLE)
            vkDestroySemaphore(logical_device, imageAvailableSemaphore, allocation_callbacks);
        if (renderFinishedSemaphore != VK_NULL_HANDLE)
            vkDestroySemaphore(logical_device, renderFinishedSemaphore, allocation_callbacks);

        if (descriptorSetLayout != VK_NULL_HANDLE)
            vkDestroyDescriptorSetLayout(logical_device, descriptorSetLayout, allocation_callbacks);
        if (descriptorPool != VK_NULL_HANDLE)
            vkDestroyDescriptorPool(logical_device, descriptorPool, allocation_callbacks);
    }

    bool Render(const std::vector<ModelData>& geometry, LogicalDevice& device, VkSurfaceKHR surface,
                VkSurfaceCapabilitiesKHR& surface_capabilities, const VkExtent2D& window_size)
    {
        uint32_t next_swapchain_image;
        if (!ValidatePipeline(device, surface, surface_capabilities, window_size) ||
            !GetNextImage(next_swapchain_image) || !BeginCommandRecording())
            return false;

        BeginRenderPass(framebuffers[next_swapchain_image]);
        vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline);
        dynamic_state.Update(commandBuffer);

        if (!DrawGeometry(geometry))
            return false;

        vkCmdEndRenderPass(commandBuffer);
        if (!EndCommandRecording())
            return false;

        VkQueue graphics_queue = device.GetGraphicsQueue();
        if (!SubmitCommands(graphics_queue) || !Present(graphics_queue, next_swapchain_image))
            return false;

        return true;
    }

  private:

    bool RetrieveImages()
    {
        // Get VkImages
        uint32_t image_count;
        if (vkGetSwapchainImagesKHR(logical_device, swapchain.swapchain, &image_count, nullptr) != VK_SUCCESS)
        {
            std::cerr << "Failed to get swapchain image count!" << std::endl;
            return false;
        }

        images.resize(image_count);
        if (vkGetSwapchainImagesKHR(logical_device, swapchain.swapchain, &image_count, images.data()) != VK_SUCCESS)
        {
            std::cerr << "Failed to get swapchain images!" << std::endl;
            return false;
        }

        // Get VkImageViews & VkFramebuffers
        image_views.resize(image_count);
        framebuffers.resize(image_count);
        for (auto i = 0; i < image_count; i++)
        {
            VkImageViewCreateInfo createInfo{};
            createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            createInfo.image = images[i];
            createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
            createInfo.format = Criteria::Required::surface_format.format;
            createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            createInfo.subresourceRange.baseMipLevel = 0;
            createInfo.subresourceRange.levelCount = 1;
            createInfo.subresourceRange.baseArrayLayer = 0;
            createInfo.subresourceRange.layerCount = 1;

            if (vkCreateImageView(logical_device, &createInfo, nullptr, &image_views[i]) != VK_SUCCESS)
            {
                std::cerr << "Failed to create image view " << i + 1 << "/" << image_count << "!" << std::endl;
                return false;
            }

            VkFramebufferCreateInfo framebufferInfo{};
            framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
            framebufferInfo.renderPass = renderPass;
            framebufferInfo.attachmentCount = 1;
            framebufferInfo.pAttachments = &image_views[i];
            framebufferInfo.width = swapchain.extent.width;
            framebufferInfo.height = swapchain.extent.height;
            framebufferInfo.layers = 1;

            if (vkCreateFramebuffer(logical_device, &framebufferInfo, nullptr, &framebuffers[i]) != VK_SUCCESS)
            {
                std::cerr << "Failed to create framebuffer " << i + 1 << " / " << image_count << "!" << std::endl;
                return false;
            }
        }

        return true;
    }

    bool CreateCommandPool(uint32_t graphics_family)
    {
        VkCommandPoolCreateInfo poolInfo{};
        poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        poolInfo.queueFamilyIndex = graphics_family;
        poolInfo.flags = 0;

        std::cout << "Creating Command Pool." << std::endl;
        if (vkCreateCommandPool(logical_device, &poolInfo, allocation_callbacks, &commandPool) != VK_SUCCESS)
        {
            std::cerr << "Failed to create Command Pool!" << std::endl;
            return false;
        }
        return true;
    }

    bool CreateCommandBuffer()
    {
        VkCommandBufferAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.commandPool = commandPool;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandBufferCount = 1;

        std::cout << "Creating Command Buffer." << std::endl;
        if (vkAllocateCommandBuffers(logical_device, &allocInfo, &commandBuffer) != VK_SUCCESS)
        {
            std::cerr << "Failed to create Command Buffer!" << std::endl;
            return false;
        }
        return true;
    }

    bool CreateRenderPass()
    {
        VkAttachmentDescription colorAttachment{};
        colorAttachment.format = Criteria::Required::surface_format.format;
        colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
        colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

        VkAttachmentReference colorAttachmentRef{};
        colorAttachmentRef.attachment = 0;
        colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        VkSubpassDescription subpass{};
        subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpass.colorAttachmentCount = 1;
        subpass.pColorAttachments = &colorAttachmentRef;

        VkSubpassDependency dependency{};
        dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
        dependency.dstSubpass = 0;
        dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependency.srcAccessMask = 0;
        dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

        VkRenderPassCreateInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        renderPassInfo.attachmentCount = 1;
        renderPassInfo.pAttachments = &colorAttachment;
        renderPassInfo.subpassCount = 1;
        renderPassInfo.pSubpasses = &subpass;
        renderPassInfo.dependencyCount = 1;
        renderPassInfo.pDependencies = &dependency;

        std::cout << "Creating Render Pass." << std::endl;
        if (vkCreateRenderPass(logical_device, &renderPassInfo, allocation_callbacks, &renderPass) != VK_SUCCESS)
        {
            std::cerr << "Failed to create render pass!" << std::endl;
            return false;
        }

        return true;
    }

    bool CreateSemaphores()
    {
        std::cout << "Creating Semaphores." << std::endl;
        VkSemaphoreCreateInfo semaphoreInfo{};
        semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
        return vkCreateSemaphore(logical_device, &semaphoreInfo, allocation_callbacks,
                                 &imageAvailableSemaphore) == VK_SUCCESS &&
               vkCreateSemaphore(logical_device, &semaphoreInfo, allocation_callbacks,
                                 &renderFinishedSemaphore) == VK_SUCCESS;
    }

    bool CreatePipelineLayout()
    {
        std::cout << "Creating Pipeline Layout." << std::endl;

        VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
        pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutInfo.setLayoutCount = 1;
        pipelineLayoutInfo.pSetLayouts = &descriptorSetLayout;
        pipelineLayoutInfo.pushConstantRangeCount = 0;
        pipelineLayoutInfo.pPushConstantRanges = nullptr;

        if (vkCreatePipelineLayout(logical_device, &pipelineLayoutInfo, allocation_callbacks, &pipelineLayout) !=
            VK_SUCCESS)
        {
            std::cerr << "Failed to create pipeline layout!" << std::endl;
            return false;
        }

        return true;
    }

    bool CreateGraphicsPipeline()
    {
        // Shaders
        const int shader_count = 2;
        VkPipelineShaderStageCreateInfo shader_stages[shader_count]{};
        const char* names[] = {"Vertex", "Fragment"};
        const char* codes[] = {Shaders::Default::Vertex, Shaders::Default::Fragment};
        shader_stages[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
        shader_stages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;

        std::set<VkShaderModule> shader_modules{};
        for (int i = 0; i < shader_count; i++)
        {
            auto& shader_stage = shader_stages[i];
            shader_stage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
            shader_stage.pName = "main";

            std::cout << "Creating " << names[i] << " Shader Module." << std::endl;
            if (!Shaders::CreateModule(logical_device, codes[i], shader_stage.module))
                break;
            shader_modules.insert(shader_stage.module);
        }

        bool success = false;
        if (shader_modules.size() == shader_count)
        {
            std::cout << "Creating Graphics Pipeline with " << shader_count << " shaders." << std::endl;
            VkGraphicsPipelineCreateInfo pipelineInfo{};
            pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
            pipelineInfo.stageCount = shader_count;
            pipelineInfo.pStages = shader_stages;
            pipelineInfo.layout = pipelineLayout;
            pipelineInfo.renderPass = renderPass;
            pipelineInfo.subpass = 0;
            pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
            pipelineInfo.basePipelineIndex = -1;

            // Static States
            VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
            Shaders::PopulateVertexInputState(vertexInputInfo);
            pipelineInfo.pVertexInputState = &vertexInputInfo;

            VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
            Populate::GraphicsPipeline::InputAssemblyState(inputAssembly);
            pipelineInfo.pInputAssemblyState = &inputAssembly;

            VkPipelineMultisampleStateCreateInfo multisampling{};
            Populate::GraphicsPipeline::MultisampleState(multisampling);
            pipelineInfo.pMultisampleState = &multisampling;

            VkPipelineColorBlendAttachmentState colorBlendAttachment{};
            VkPipelineColorBlendStateCreateInfo colorBlending{};
            Populate::GraphicsPipeline::ColorBlendState(colorBlending, colorBlendAttachment);
            pipelineInfo.pColorBlendState = &colorBlending;

            // Dynamic States
            VkPipelineViewportStateCreateInfo viewportState{};
            VkPipelineDepthStencilStateCreateInfo depth_stencil{};
            VkPipelineRasterizationStateCreateInfo rasterizer{};
            VkPipelineDynamicStateCreateInfo dynamicState{};
            std::vector<VkDynamicState> states{};
            dynamic_state.PopulatePipelineCreateInfo(pipelineInfo, viewportState, depth_stencil, rasterizer,
                                                     dynamicState, states);

            // Build Graphics Pipeline
            success = vkCreateGraphicsPipelines(logical_device, VK_NULL_HANDLE, 1, &pipelineInfo,
                                                allocation_callbacks, &graphicsPipeline) == VK_SUCCESS;
        }
        else
            std::cerr << "Failed to create shader modules!" << std::endl;

        // Release shader Modules
        for (auto shader_module : shader_modules)
            vkDestroyShaderModule(logical_device, shader_module, allocation_callbacks);

        if (!success)
        {
            std::cerr << "Failed to create graphics pipeline!" << std::endl;
            return false;
        }

        return true;
    }

    bool CreateDescriptorSetLayout()
    {
        std::cout << "Creating Descriptor Set Layout." << std::endl;

        VkDescriptorSetLayoutCreateInfo layoutInfo{};
        layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        layoutInfo.bindingCount = static_cast<uint32_t>(Shaders::layout_bindings.size());
        layoutInfo.pBindings = Shaders::layout_bindings.data();

        if (vkCreateDescriptorSetLayout(logical_device, &layoutInfo, allocation_callbacks, &descriptorSetLayout) !=
            VK_SUCCESS)
        {
            std::cerr << "Failed to create Descriptor Set Layout!" << std::endl;
            return false;
        }

        return true;
    }

    bool CreateDescriptorPool()
    {
        VkDescriptorPoolCreateInfo poolInfo{};
        poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        poolInfo.poolSizeCount = static_cast<uint32_t>(Shaders::pool_sizes.size());
        poolInfo.pPoolSizes = Shaders::pool_sizes.data();
        poolInfo.maxSets = 1;

        if (vkCreateDescriptorPool(logical_device, &poolInfo, allocation_callbacks, &descriptorPool) != VK_SUCCESS)
        {
            std::cerr << "Failed to create descriptor pool!" << std::endl;
            return false;
        }

        return true;
    }

    // Rendering

    bool ValidatePipeline(LogicalDevice& device, VkSurfaceKHR surface, VkSurfaceCapabilitiesKHR& surface_capabilities,
                          const VkExtent2D& window_size)
    {
        const uint8_t changes = swapchain.GetChanges(surface_capabilities);
        if (changes != 0)
        {
            std::cout << "Recreating swapchain..." << std::endl;

            // Freeing old resources
            for (auto framebuffer : framebuffers)
                vkDestroyFramebuffer(logical_device, framebuffer, allocation_callbacks);
            for (auto imageView : image_views)
                vkDestroyImageView(logical_device, imageView, allocation_callbacks);

            if (!swapchain.Create(device, surface, surface_capabilities, window_size, swapchain.swapchain) ||
                !RetrieveImages())
            {
                std::cerr << "Failed to recreate swapchain!" << std::endl;
                return false;
            }

            if(changes & Swapchain::CapabilityChanges::Extent)
                dynamic_state.OnSwapchainExtentChanged(swapchain.extent);
        }

        if (dynamic_state.requires_recreation && !CreateGraphicsPipeline())
        {
            std::cerr << "Failed to recreate Graphics Pipeline!" << std::endl;
            return false;
        }

        return true;
    }

    bool GetNextImage(uint32_t& next_swapchain_image) const
    {
        if (vkAcquireNextImageKHR(logical_device, swapchain.swapchain, UINT64_MAX, imageAvailableSemaphore,
                                  VK_NULL_HANDLE, &next_swapchain_image) != VK_SUCCESS)
        {
            std::cerr << "Failed to acquire next swapchain image." << std::endl;
            return false;
        }
        return true;
    }

    bool BeginCommandRecording() const
    {
        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS)
        {
            std::cerr << "Failed to Begin Command Buffer." << std::endl;
            return false;
        }
        
        return true;
    }

    void BeginRenderPass(VkFramebuffer framebuffer) const
    {
        VkClearValue clear = {clear_color};
        VkRenderPassBeginInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.renderPass = renderPass;
        renderPassInfo.framebuffer = framebuffer;
        renderPassInfo.renderArea.offset = {0, 0};
        renderPassInfo.renderArea.extent = swapchain.extent;
        renderPassInfo.clearValueCount = 1;
        renderPassInfo.pClearValues = &clear;
        vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
    }

    bool DrawGeometry(const std::vector<ModelData>& geometry) const
    {
        if (geometry.empty())
            return true;

        // Fake time & Transform Matrices
        static float time = 0.0f;
        static Shaders::TransfomMatrices ubo{};
        time += 0.05f;
        ubo.model[12] = sin(time) * 0.5f; // X-axis
        ubo.model[13] = cos(time) * 0.5f; // Y-axis

        std::vector<std::pair<void*, VkDeviceSize>> uniform_data = {
            {&ubo, sizeof(ubo)}};

        for (const auto& geo : geometry)
        {
            // Update Uniforms
            if (!geo.UpdateAndBindUniformBuffers(logical_device, uniform_data, commandBuffer, pipelineLayout))
                return false;

            // Bind Buffers & Draw
            vkCmdBindVertexBuffers(commandBuffer, 0, 1, &geo.vertex.buffer, &geo.offset);
            vkCmdBindIndexBuffer(commandBuffer, geo.index.buffer, geo.offset, VK_INDEX_TYPE_UINT32);
            vkCmdDrawIndexed(commandBuffer, geo.index.count, 1, 0, 0, 0);
        }

        return true;
    }

    bool EndCommandRecording() const
    {
        if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS)
        {
            std::cerr << "Failed to end command buffer." << std::endl;
            return false;
        }

        return true;
    }

    bool SubmitCommands(VkQueue graphics_queue) const
    {
        // Submit the command buffer.
        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

        VkSemaphore waitSemaphores[] = {imageAvailableSemaphore};
        VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
        submitInfo.waitSemaphoreCount = 1;
        submitInfo.pWaitSemaphores = waitSemaphores;
        submitInfo.pWaitDstStageMask = waitStages;

        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &commandBuffer;

        VkSemaphore signalSemaphores[] = {renderFinishedSemaphore};
        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pSignalSemaphores = signalSemaphores;

        if (vkQueueSubmit(graphics_queue, 1, &submitInfo, VK_NULL_HANDLE) != VK_SUCCESS)
        {
            std::cerr << "Failed to submit command buffer." << std::endl;
            return false;
        }

        return true;
    }

    bool Present(VkQueue graphics_queue, uint32_t& next_swapchain_image) const
    {
        // Present the image.
        VkPresentInfoKHR presentInfo{};
        presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
        presentInfo.waitSemaphoreCount = 1;
        presentInfo.pWaitSemaphores = &renderFinishedSemaphore;
        presentInfo.swapchainCount = 1;
        presentInfo.pSwapchains = &swapchain.swapchain;
        presentInfo.pImageIndices = &next_swapchain_image;

        if (vkQueuePresentKHR(graphics_queue, &presentInfo) != VK_SUCCESS)
        {
            std::cerr << "Failed to present swapchain image." << std::endl;
            return false;
        }

        return true;
    }
};

struct DebugMessenger
{
    VkDebugUtilsMessengerEXT messenger = VK_NULL_HANDLE;

    bool Create(VkInstance instance)
    {
        VkDebugUtilsMessengerCreateInfoEXT createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
        createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
                                     VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                                     VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
        createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                                 VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                                 VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
        createInfo.pfnUserCallback = DebugCallback;

        auto func =
            (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
        if (func == nullptr)
        {
            std::cerr << "Failed to retrieve vkCreateDebugUtilsMessengerEXT!" << std::endl;
            return false;
        }
        if (func(instance, &createInfo, allocation_callbacks, &messenger) != VK_SUCCESS)
        {
            std::cerr << "Failed to Create debug messenger!" << std::endl;
            return false;
        }

        return true;
    }

    bool Delete(VkInstance instance)
    {
        if (messenger == VK_NULL_HANDLE)
            return true;

        auto func =
            (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
        if (func == nullptr)
        {
            std::cerr << "Failed to retrieve vkDestroyDebugUtilsMessengerEXT!" << std::endl;
            return false;
        }

        func(instance, messenger, allocation_callbacks);
        return true;
    }
};

export namespace RE
{
    namespace Vulkan
    {
        bool Init()
        {
            std::cout << "Loading default Vulkan library." << std::endl;
            if (SDL_Vulkan_LoadLibrary(nullptr) != 0)
            {
                std::cerr << "Failed to load default Vulkan library: " << SDL_GetError() << std::endl;
                return false;
            }

            return Populate::Instance::LayerProperties();
        }

        bool CreateTriangle(ModelData& out_data, const LogicalDevice& device, VkCommandPool commandPool,
                                   VkDescriptorPool& descriptorPool, VkDescriptorSetLayout& descriptorSetLayout)
        {
            std::vector<uint32_t> indices = {0, 1, 2};
            std::vector<Shaders::Vertex> vertices = {{{0.0f, -0.5f, 0.0f}, {1.0f, 0.0f, 0.0f}},
                                                     {{0.5f, 0.5f, 0.0f}, {0.0f, 1.0f, 0.0f}},
                                                     {{-0.5f, 0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}}};

            if (!out_data.Create(device, commandPool, descriptorPool, descriptorSetLayout, vertices, indices))
            {
                std::cerr << "Failed to create triangle!" << std::endl;
                return false;
            }

            return true;
        }

        struct Context
        {
            VkInstance instance = VK_NULL_HANDLE;
            VkSurfaceKHR surface = VK_NULL_HANDLE;

            DebugMessenger debug_messenger{};
            LogicalDevice device{};
            GraphicPipeline pipeline{};

            // State
            VkSurfaceCapabilitiesKHR surface_capabilities{};
            VkExtent2D window_size{};
            std::vector<ModelData> to_draw{};

            bool Create(SDL_Window* window, int w, int h)
            {
                to_draw.push_back({});
                window_size = {static_cast<uint32_t>(w), static_cast<uint32_t>(h)};

                if (!CreateInstance(window) ||
                    !CreateSurface(window) ||
                    !device.Create(instance, surface) ||
                    !GetSurfaceCapabilities() ||
                    !pipeline.Create(device, surface, surface_capabilities, window_size) ||
                    !CreateTriangle(to_draw[0], device, pipeline.commandPool, pipeline.descriptorPool,
                                               pipeline.descriptorSetLayout))
                {
                    std::cerr << "Failed to setup Context for Vulkan rendering." << std::endl;
                    Delete();
                    return false;
                }

                std::cout << "Successfull Vulkan Context creation." << std::endl;
                return true;
            }

            bool Delete()
            {
                for (auto& drawable : to_draw)
                    drawable.Destroy(device.logical_device);

                pipeline.Delete();

                if (device.logical_device != VK_NULL_HANDLE)
                    vkDestroyDevice(device.logical_device, allocation_callbacks);
                if (surface != VK_NULL_HANDLE)
                    vkDestroySurfaceKHR(instance, surface, allocation_callbacks);

                if (!debug_messenger.Delete(instance))
                    return false;

                if (instance != VK_NULL_HANDLE)
                    vkDestroyInstance(instance, allocation_callbacks);

                return true;
            }

            bool RenderTriangle()
            {
                if (!GetSurfaceCapabilities() ||
                    !pipeline.Render(to_draw, device, surface, surface_capabilities, window_size))
                {
                    std::cerr << "Failed to render!" << std::endl;
                    return false;
                }

                return true;
            }

          private:

            bool CreateInstance(SDL_Window* window)
            {
                std::vector<const char*> instance_extensions;
                if (!Populate::Instance::RequiredSDLExtensions(window, instance_extensions))
                    return false;

                std::vector<const char*> layers{};
                Populate::Instance::Layers(layers);

                VkApplicationInfo appInfo{};
                appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
                appInfo.pApplicationName = "RedEye Engine";
                appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
                appInfo.pEngineName = "RedEye Engine";
                appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
                appInfo.apiVersion = VK_API_VERSION_1_0;

                VkInstanceCreateInfo createInfo{};
                createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
                createInfo.pApplicationInfo = &appInfo;
                createInfo.enabledExtensionCount = static_cast<uint32_t>(instance_extensions.size());
                createInfo.ppEnabledExtensionNames = instance_extensions.data();
                createInfo.enabledLayerCount = static_cast<uint32_t>(layers.size());
                createInfo.ppEnabledLayerNames = layers.data();

                if (vkCreateInstance(&createInfo, allocation_callbacks, &instance) != VK_SUCCESS)
                {
                    std::cerr << "Failed to create Vulkan Instance." << std::endl;
                    return false;
                }

                for (auto layer : layers)
                    if (strcmp(layer, "VK_LAYER_KHRONOS_validation") == 0)
                        return debug_messenger.Create(instance);

                return true;
            }

            bool CreateSurface(SDL_Window* window)
            {
                if (SDL_Vulkan_CreateSurface(window, instance, &surface) == SDL_TRUE)
                    return true;

                std::cerr << "Failed to get surface capabilities!" << std::endl;
                return false;
            }

            bool GetSurfaceCapabilities()
            {
                if (vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device.physical_device, surface, &surface_capabilities) ==
                    VK_SUCCESS)
                    return true;

                std::cerr << "Failed to get surface capabilities!" << std::endl;
                return false;
            }
        };
    } // namespace Vulkan
} // namespace RE