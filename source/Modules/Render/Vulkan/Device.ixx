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

#include <vulkan/vulkan.h>

#include <iostream>
#include <vector>
#include <set>

export module Device;

import VkDebug;
import Surface;

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

namespace Log
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
        std::cout << "\t\t- Max Per Stage Descriptor Uniform Buffers: " << limits.maxPerStageDescriptorUniformBuffers
                  << std::endl;
        std::cout << "\t\t- Max Per Stage Descriptor Storage Buffers: " << limits.maxPerStageDescriptorStorageBuffers
                  << std::endl;
        std::cout << "\t\t- Max Per Stage Descriptor Sampled Images: " << limits.maxPerStageDescriptorSampledImages
                  << std::endl;
        std::cout << "\t\t- Max Per Stage Descriptor Storage Images: " << limits.maxPerStageDescriptorStorageImages
                  << std::endl;
        std::cout << "\t\t- Max Per Stage Descriptor Input Attachments: "
                  << limits.maxPerStageDescriptorInputAttachments << std::endl;
        std::cout << "\t\t- Max Per Stage Resources: " << limits.maxPerStageResources << std::endl;
        std::cout << "\t\t- Max Descriptor Set Samplers: " << limits.maxDescriptorSetSamplers << std::endl;
        std::cout << "\t\t- Max Descriptor Set Uniform Buffers: " << limits.maxDescriptorSetUniformBuffers << std::endl;
        std::cout << "\t\t- Max Descriptor Set Uniform Buffers Dynamic: "
                  << limits.maxDescriptorSetUniformBuffersDynamic << std::endl;
        std::cout << "\t\t- Max Descriptor Set Storage Buffers: " << limits.maxDescriptorSetStorageBuffers << std::endl;
        std::cout << "\t\t- Max Descriptor Set Storage Buffers Dynamic: "
                  << limits.maxDescriptorSetStorageBuffersDynamic << std::endl;
        std::cout << "\t\t- Max Descriptor Set Sampled Images: " << limits.maxDescriptorSetSampledImages << std::endl;
        std::cout << "\t\t- Max Descriptor Set Storage Images: " << limits.maxDescriptorSetStorageImages << std::endl;
        std::cout << "\t\t- Max Descriptor Set Input Attachments: " << limits.maxDescriptorSetInputAttachments
                  << std::endl;
        std::cout << "\t\t- Max Vertex Input Attributes: " << limits.maxVertexInputAttributes << std::endl;
        std::cout << "\t\t- Max Vertex Input Bindings: " << limits.maxVertexInputBindings << std::endl;
        std::cout << "\t\t- Max Vertex Input Attribute Offset: " << limits.maxVertexInputAttributeOffset << std::endl;
        std::cout << "\t\t- Max Vertex Input Binding Stride: " << limits.maxVertexInputBindingStride << std::endl;
        std::cout << "\t\t- Max Vertex Output Components: " << limits.maxVertexOutputComponents << std::endl;
        std::cout << "\t\t- Max Tessellation Generation Level: " << limits.maxTessellationGenerationLevel << std::endl;
        std::cout << "\t\t- Max Tessellation Patch Size: " << limits.maxTessellationPatchSize << std::endl;
        std::cout << "\t\t- Max Tessellation Control Per Vertex Input Components: "
                  << limits.maxTessellationControlPerVertexInputComponents << std::endl;
        std::cout << "\t\t- Max Tessellation Control Per Vertex Output Components: "
                  << limits.maxTessellationControlPerVertexOutputComponents << std::endl;
        std::cout << "\t\t- Max Tessellation Control Per Patch Output Components: "
                  << limits.maxTessellationControlPerPatchOutputComponents << std::endl;
        std::cout << "\t\t- Max Tessellation Control Total Output Components: "
                  << limits.maxTessellationControlTotalOutputComponents << std::endl;
        std::cout << "\t\t- Max Tessellation Evaluation Input Components: "
                  << limits.maxTessellationEvaluationInputComponents << std::endl;
        std::cout << "\t\t- Max Tessellation Evaluation Output Components: "
                  << limits.maxTessellationEvaluationOutputComponents << std::endl;
        std::cout << "\t\t- Max Geometry Shader Invocations: " << limits.maxGeometryShaderInvocations << std::endl;
        std::cout << "\t\t- Max Geometry Input Components: " << limits.maxGeometryInputComponents << std::endl;
        std::cout << "\t\t- Max Geometry Output Components: " << limits.maxGeometryOutputComponents << std::endl;
        std::cout << "\t\t- Max Geometry Output Vertices: " << limits.maxGeometryOutputVertices << std::endl;
        std::cout << "\t\t- Max Geometry Total Output Components: " << limits.maxGeometryTotalOutputComponents
                  << std::endl;
        std::cout << "\t\t- Max Fragment Input Components: " << limits.maxFragmentInputComponents << std::endl;
        std::cout << "\t\t- Max Fragment Output Attachments: " << limits.maxFragmentOutputAttachments << std::endl;
        std::cout << "\t\t- Max Fragment Dual Src Attachments: " << limits.maxFragmentDualSrcAttachments << std::endl;
        std::cout << "\t\t- Max Fragment Combined Output Resources: " << limits.maxFragmentCombinedOutputResources
                  << std::endl;
        std::cout << "\t\t- Max Compute Shared Memory Size: " << limits.maxComputeSharedMemorySize << std::endl;
        std::cout << "\t\t- Max Compute Work Group Count: (" << limits.maxComputeWorkGroupCount[0] << ", "
                  << limits.maxComputeWorkGroupCount[1] << ", " << limits.maxComputeWorkGroupCount[2] << ")"
                  << std::endl;
        std::cout << "\t\t- Max Compute Work Group Invocations: " << limits.maxComputeWorkGroupInvocations << std::endl;
        std::cout << "\t\t- Max Compute Work Group Size: (" << limits.maxComputeWorkGroupSize[0] << ", "
                  << limits.maxComputeWorkGroupSize[1] << ", " << limits.maxComputeWorkGroupSize[2] << ")" << std::endl;
        std::cout << "\t\t- Sub Pixel Precision Bits: " << limits.subPixelPrecisionBits << std::endl;
        std::cout << "\t\t- Sub Texel Precision Bits: " << limits.subTexelPrecisionBits << std::endl;
        std::cout << "\t\t- Mipmap Precision Bits: " << limits.mipmapPrecisionBits << std::endl;
        std::cout << "\t\t- Max Draw Indexed Index Value: " << limits.maxDrawIndexedIndexValue << std::endl;
        std::cout << "\t\t- Max Draw Indirect Count: " << limits.maxDrawIndirectCount << std::endl;
        std::cout << "\t\t- Max Sampler LOD Bias: " << limits.maxSamplerLodBias << std::endl;
        std::cout << "\t\t- Max Sampler Anisotropy: " << limits.maxSamplerAnisotropy << std::endl;
        std::cout << "\t\t- Max Viewports: " << limits.maxViewports << std::endl;
        std::cout << "\t\t- Max Viewport Dimensions: (" << limits.maxViewportDimensions[0] << ", "
                  << limits.maxViewportDimensions[1] << ")" << std::endl;
        std::cout << "\t\t- Viewport Bounds Range: (" << limits.viewportBoundsRange[0] << ", "
                  << limits.viewportBoundsRange[1] << ")" << std::endl;
        std::cout << "\t\t- Viewport Sub Pixel Bits: " << limits.viewportSubPixelBits << std::endl;
        std::cout << "\t\t- Min Memory Map Alignment: " << limits.minMemoryMapAlignment << std::endl;
        std::cout << "\t\t- Min Texel Buffer Offset Alignment: " << limits.minTexelBufferOffsetAlignment << std::endl;
        std::cout << "\t\t- Min Uniform Buffer Offset Alignment: " << limits.minUniformBufferOffsetAlignment
                  << std::endl;
        std::cout << "\t\t- Min Storage Buffer Offset Alignment: " << limits.minStorageBufferOffsetAlignment
                  << std::endl;
        std::cout << "\t\t- Min Texel Offset: " << limits.minTexelOffset << std::endl;
        std::cout << "\t\t- Max Texel Offset: " << limits.maxTexelOffset << std::endl;
        std::cout << "\t\t- Min Texel Gather Offset: " << limits.minTexelGatherOffset << std::endl;
        std::cout << "\t\t- Max Texel Gather Offset: " << limits.maxTexelGatherOffset << std::endl;
        std::cout << "\t\t- Min Interpolation Offset: " << limits.minInterpolationOffset << std::endl;
        std::cout << "\t\t- Max Interpolation Offset: " << limits.maxInterpolationOffset << std::endl;
        std::cout << "\t\t- Sub Pixel Interpolation Offset Bits: " << limits.subPixelInterpolationOffsetBits
                  << std::endl;
        std::cout << "\t\t- Max Framebuffer Width: " << limits.maxFramebufferWidth << std::endl;
        std::cout << "\t\t- Max Framebuffer Height: " << limits.maxFramebufferHeight << std::endl;
        std::cout << "\t\t- Max Framebuffer Layers: " << limits.maxFramebufferLayers << std::endl;
        std::cout << "\t\t- Framebuffer Color Sample Counts: " << limits.framebufferColorSampleCounts << std::endl;
        std::cout << "\t\t- Framebuffer Depth Sample Counts: " << limits.framebufferDepthSampleCounts << std::endl;
        std::cout << "\t\t- Framebuffer Stencil Sample Counts: " << limits.framebufferStencilSampleCounts << std::endl;
        std::cout << "\t\t- Framebuffer No Attachments Sample Counts: " << limits.framebufferNoAttachmentsSampleCounts
                  << std::endl;
        std::cout << "\t\t- Max Color Attachments: " << limits.maxColorAttachments << std::endl;
        std::cout << "\t\t- Sampled Image Color Sample Counts: " << limits.sampledImageColorSampleCounts << std::endl;
        std::cout << "\t\t- Sampled Image Integer Sample Counts: " << limits.sampledImageIntegerSampleCounts
                  << std::endl;
        std::cout << "\t\t- Sampled Image Depth Sample Counts: " << limits.sampledImageDepthSampleCounts << std::endl;
        std::cout << "\t\t- Sampled Image Stencil Sample Counts: " << limits.sampledImageStencilSampleCounts
                  << std::endl;
        std::cout << "\t\t- Storage Image Sample Counts: " << limits.storageImageSampleCounts << std::endl;
        std::cout << "\t\t- Max Sample Mask Words: " << limits.maxSampleMaskWords << std::endl;
        std::cout << "\t\t- Timestamp Compute And Graphics: " << limits.timestampComputeAndGraphics << std::endl;
        std::cout << "\t\t- Timestamp Period: " << limits.timestampPeriod << std::endl;
        std::cout << "\t\t- Max Clip Distances: " << limits.maxClipDistances << std::endl;
        std::cout << "\t\t- Max Cull Distances: " << limits.maxCullDistances << std::endl;
        std::cout << "\t\t- Max Combined Clip And Cull Distances: " << limits.maxCombinedClipAndCullDistances
                  << std::endl;
        std::cout << "\t\t- Discrete Queue Priorities: " << limits.discreteQueuePriorities << std::endl;
        std::cout << "\t\t- Point Size Range: (" << limits.pointSizeRange[0] << ", " << limits.pointSizeRange[1] << ")"
                  << std::endl;
        std::cout << "\t\t- Line Width Range: (" << limits.lineWidthRange[0] << ", " << limits.lineWidthRange[1] << ")"
                  << std::endl;
        std::cout << "\t\t- Point Size Granularity: " << limits.pointSizeGranularity << std::endl;
        std::cout << "\t\t- Line Width Granularity: " << limits.lineWidthGranularity << std::endl;
        std::cout << "\t\t- Strict Lines: " << limits.strictLines << std::endl;
        std::cout << "\t\t- Standard Sample Locations: " << limits.standardSampleLocations << std::endl;
        std::cout << "\t\t- Optimal Buffer Copy Offset Alignment: " << limits.optimalBufferCopyOffsetAlignment
                  << std::endl;
        std::cout << "\t\t- Optimal Buffer Copy Row Pitch Alignment: " << limits.optimalBufferCopyRowPitchAlignment
                  << std::endl;
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
        std::cout << "\t\t- Texture Compression ASTC_LDR: " << deviceFeatures.textureCompressionASTC_LDR << std::endl;
        std::cout << "\t\t- Texture Compression BC: " << deviceFeatures.textureCompressionBC << std::endl;
        std::cout << "\t\t- Occlusion Query Precise: " << deviceFeatures.occlusionQueryPrecise << std::endl;
        std::cout << "\t\t- Pipeline Statistics Query: " << deviceFeatures.pipelineStatisticsQuery << std::endl;
        std::cout << "\t\t- Vertex Pipeline Stores and Atomics: " << deviceFeatures.vertexPipelineStoresAndAtomics
                  << std::endl;
        std::cout << "\t\t- Fragment Stores and Atomics: " << deviceFeatures.fragmentStoresAndAtomics << std::endl;
        std::cout << "\t\t- Shader Tessellation and Geometry Point Size: "
                  << deviceFeatures.shaderTessellationAndGeometryPointSize << std::endl;
        std::cout << "\t\t- Shader Image Gather Extended: " << deviceFeatures.shaderImageGatherExtended << std::endl;
        std::cout << "\t\t- Shader Storage Image Extended Formats: " << deviceFeatures.shaderStorageImageExtendedFormats
                  << std::endl;
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

    enum Scope : uint8_t
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
                  << PhysicalDeviceVendor(deviceProperties.vendorID) << " - "
                  << PhysicalDeviceType(deviceProperties.deviceType) << " - "
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
        if (scope_flags & SPARSE)
            SparseProperties(deviceProperties.sparseProperties);
        if (scope_flags & LIMITS)
            DeviceLimits(deviceProperties.limits);
        if (scope_flags & MEMORY)
            MemoryProperties(device);
        if (scope_flags & QUEUES)
            QueueFamilies(device);
        if (scope_flags & EXT_COUNT)
            ExtensionCount(device);
        if (scope_flags & EXT_ALL)
            Extensions(device);
        if (scope_flags & FEATURES)
            Features(device);
    }

} // namespace PhysicalDevice

bool GetPhysicalDevices(VkInstance& instance, std::vector<VkPhysicalDevice>& physical_devices)
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
        Log::Properties(device);

    return true;
}

export struct LogicalDevice
{
    enum QueueFamilySetup
    {
        UNSET,
        SINGLE,
        SAME_GRAPHICS_PRESENT,
        SEPARATE
    };

    enum Feature : uint8_t
    {
        GEOMETRY_SHADER = 0,
        TESSELLATION_SHADER = 1,
        SAMPLER_ANISOTROPY = 2
    };

    VkDevice logical_device = VK_NULL_HANDLE;
    VkPhysicalDevice physical_device = VK_NULL_HANDLE;

    QueueFamilySetup queue_setup = UNSET;
    uint32_t graphics_family = 0;
    uint32_t transfer_family = 0;
    uint32_t present_family = 0;

    VkPhysicalDeviceMemoryProperties mem_properties;

    bool Create(VkInstance instance, const Surface& surface,
                uint8_t required_features = Feature::GEOMETRY_SHADER | Feature::TESSELLATION_SHADER |
                                   Feature::SAMPLER_ANISOTROPY,
                const std::vector<const char*>& required_extensions = {VK_KHR_SWAPCHAIN_EXTENSION_NAME})
    {
        std::cout << "Finding suitable Vulkan Physical Devices." << std::endl;
        std::vector<VkPhysicalDevice> physical_devices{};
        if (!GetPhysicalDevices(instance, physical_devices))
            return false;

        for (size_t i = 0; i < physical_devices.size(); ++i)
        {
            physical_device = physical_devices[i];
            if (HasValidType() && 
                HasRequiredFeatures(required_features) &&
                SupportsRequiredExtensions(required_extensions) && 
                HasValidQueueFamiliesWithKHR(surface.surface) &&
                HasRequiredSurfaceFormat(surface.surface, surface.format) &&
                HasRequiredPresentMode(surface.surface, surface.present_mode) &&
                CorrectCreation(required_extensions))
            {
                std::cout << "Created Logical Device from suitable Vulkan Physical Device ID: " << i << std::endl;
                vkGetPhysicalDeviceMemoryProperties(physical_device, &mem_properties);
                return true;
            }
        }

        std::cerr << "Failed to find a suitable GPU." << std::endl;
        return false;
    }

    void Clear()
    {
        if (physical_device == VK_NULL_HANDLE)
            return;
        
        vkDestroyDevice(logical_device, VkDebug::Allocation());
        logical_device = VK_NULL_HANDLE;
    }

    VkQueue GetGraphicsQueue() const
    {
        VkQueue queue;
        vkGetDeviceQueue(logical_device, graphics_family, 0, &queue);
        return queue;
    }

    private:

    bool HasValidType()
    {
        VkPhysicalDeviceProperties deviceProperties;
        vkGetPhysicalDeviceProperties(physical_device, &deviceProperties);
        return deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU;
    }

    bool HasRequiredFeatures(const uint8_t required_features)
    {
        VkPhysicalDeviceFeatures deviceFeatures;
        vkGetPhysicalDeviceFeatures(physical_device, &deviceFeatures);

        std::vector<std::pair<uint8_t, VkBool32>> features = {
            {Feature::GEOMETRY_SHADER, deviceFeatures.geometryShader},
            {Feature::TESSELLATION_SHADER, deviceFeatures.tessellationShader},
            {Feature::SAMPLER_ANISOTROPY, deviceFeatures.samplerAnisotropy}};

        for (const auto& [feature, supported] : features)
            if ((required_features & feature) && !supported)
                return false;

        return true;
    }

    bool SupportsRequiredExtensions(const std::vector<const char*>& required_extensions)
    {
        uint32_t extensionCount;
        vkEnumerateDeviceExtensionProperties(physical_device, nullptr, &extensionCount, nullptr);
        std::vector<VkExtensionProperties> availableExtensions(extensionCount);
        vkEnumerateDeviceExtensionProperties(physical_device, nullptr, &extensionCount, availableExtensions.data());

        for (const char* required : required_extensions)
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

    bool HasRequiredSurfaceFormat(VkSurfaceKHR surface, const VkSurfaceFormatKHR& required_surface_format)
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
            if (availableFormat.format == required_surface_format.format &&
                availableFormat.colorSpace == required_surface_format.colorSpace)
                return true;
        }

        std::cerr << "Device doesn't support required surface format!" << std::endl;
        return false;
    }

    bool HasRequiredPresentMode(VkSurfaceKHR surface, const VkPresentModeKHR present_mode)
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
            if (mode == present_mode)
                return true;

        std::cerr << "Device doesn't support required surface present mode!" << std::endl;
        return false;
    }

    bool CorrectCreation(const std::vector<const char*>& extensions)
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
        // VkPhysicalDeviceFeatures features;
        // vkGetPhysicalDeviceFeatures(physical_device, &features);
        // deviceCreateInfo.pEnabledFeatures = &features;

        deviceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
        deviceCreateInfo.ppEnabledExtensionNames = extensions.data();

        return vkCreateDevice(physical_device, &deviceCreateInfo, VkDebug::Allocation(), &logical_device) == VK_SUCCESS;
    }
};