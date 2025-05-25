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

export module GraphicPipeline;

import VkDebug;
import Device;
import Buffers;
import Shading;
import Surface;
import Swapchain;
import PipelineDynamicStates;

export struct GraphicPipeline
{
    VkDevice logical_device = VK_NULL_HANDLE;
    VkPipeline pipeline = VK_NULL_HANDLE;

    Swapchain swapchain{};
    VkRenderPass render_pass = VK_NULL_HANDLE; // Describes rendering operations.
    VkPipelineLayout layout = VK_NULL_HANDLE;  // Manages shaders and resources.
    PipelineDynamicStates dynamic_state{};

    // Commands
    VkCommandPool cmd_pool = VK_NULL_HANDLE;
    VkCommandBuffer cmd_buffer = VK_NULL_HANDLE;

    // Semaphores
    VkSemaphore imageAvailableSemaphore = VK_NULL_HANDLE;
    VkSemaphore renderFinishedSemaphore = VK_NULL_HANDLE;

    bool Create(const Shading* shading, VkDevice device, const Surface& surface,
                VkPresentModeKHR present_mode, uint32_t graphics_family, uint32_t present_family, 
                const ShadingDescriptor& descriptor)
    {
        std::cout << "Creating Graphic Pipeline..." << std::endl;

        if (!swapchain.Create(logical_device = device, surface, graphics_family, present_family, present_mode))
            return false;

        dynamic_state.Setup(swapchain.extent);

        if (!CreateRenderPass(surface.format) ||
            !CreatePipelineLayout(descriptor.set_layouts) || 
            !CreateGraphicsPipeline(shading))
        {
            std::cerr << "Failed to create Graphic Pipeline." << std::endl;
            return false;
        }

        return 
            swapchain.RetrieveImages(logical_device, surface.format, render_pass) &&
            CreateCommandPool(graphics_family) &&
            CreateCommandBuffer() && 
            CreateSemaphores();
    }

    void Clear()
    {
        vkDestroyPipelineLayout(logical_device, layout, VkDebug::Allocation());
        vkDestroyRenderPass(logical_device, render_pass, VkDebug::Allocation());
        layout = VK_NULL_HANDLE;
        render_pass = VK_NULL_HANDLE;

        swapchain.Clear(logical_device);

        vkDestroyCommandPool(logical_device, cmd_pool, VkDebug::Allocation());
        cmd_pool = VK_NULL_HANDLE;
        cmd_buffer = VK_NULL_HANDLE;

        vkDestroySemaphore(logical_device, imageAvailableSemaphore, VkDebug::Allocation());
        vkDestroySemaphore(logical_device, renderFinishedSemaphore, VkDebug::Allocation());
        imageAvailableSemaphore = VK_NULL_HANDLE;
        renderFinishedSemaphore = VK_NULL_HANDLE;
    }
    
    bool OnSurfaceCapabilitiesChanged(uint8_t changes, const Shading* shading, 
        const Surface& surface, uint32_t graphics_family, uint32_t present_family)
    {
        std::cout << "Recreating Swapchain..." << std::endl;
        if (!swapchain.Recreate(logical_device, surface, graphics_family, present_family, render_pass))
        {
            std::cerr << "Failed to recreate swapchain!" << std::endl;
            return false;
        }

        // If dynamic_state can handle Extent changes, no need to recreate the pipeline
        if (changes & Surface::CapabilityChanges::Extent)
            dynamic_state.OnSwapchainExtentChanged(surface.capabilities.currentExtent);
        if (!dynamic_state.requires_recreation)
            return true;

        std::cout << "Recreating Pipeline..." << std::endl;
        return CreateGraphicsPipeline(shading);
    }
    
    bool PrepareRender(uint32_t& next_swapchain_image)
    {
        if (!swapchain.GetNextImage(logical_device, next_swapchain_image, imageAvailableSemaphore) ||
            !BeginCommandRecording())
            return false;

        BeginRenderPass(swapchain.framebuffers[next_swapchain_image], dynamic_state.GetClearValues());
        vkCmdBindPipeline(cmd_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
        dynamic_state.Update(cmd_buffer);
        return true;
    }

    bool SubmitRender(VkQueue graphics_queue, uint32_t& next_swapchain_image)
    {
        vkCmdEndRenderPass(cmd_buffer);
        return EndCommandRecording() && SubmitCommands(graphics_queue, {cmd_buffer}) &&
               Present(graphics_queue, next_swapchain_image);
    }

  private:

    // Pipeline Creation

    bool CreateRenderPass(const VkSurfaceFormatKHR& surface_format)
    {
        VkAttachmentDescription colorAttachment{};
        colorAttachment.format = surface_format.format;
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
        if (vkCreateRenderPass(logical_device, &renderPassInfo, VkDebug::Allocation(), &render_pass) != VK_SUCCESS)
        {
            std::cerr << "Failed to create render pass!" << std::endl;
            return false;
        }

        return true;
    }

    bool CreatePipelineLayout(const std::vector<VkDescriptorSetLayout>& set_layouts)
    {
        std::cout << "Creating Pipeline Layout." << std::endl;

        VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
        pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(set_layouts.size());
        pipelineLayoutInfo.pSetLayouts = set_layouts.data();
        pipelineLayoutInfo.pushConstantRangeCount = 0;
        pipelineLayoutInfo.pPushConstantRanges = nullptr;

        if (vkCreatePipelineLayout(logical_device, &pipelineLayoutInfo, VkDebug::Allocation(), &layout) !=
            VK_SUCCESS)
        {
            std::cerr << "Failed to create pipeline layout!" << std::endl;
            return false;
        }

        return true;
    }

    bool CreateGraphicsPipeline(const Shading* shading)
    {
        std::cout << "Creating Shader Modules." << std::endl;
        std::vector<VkPipelineShaderStageCreateInfo> shader_stages{};
        if (!shading->GetStageCreateInfo(logical_device, shader_stages))
        {
            std::cerr << "Failed to create shader modules!" << std::endl;

            for (const auto& shader_stage : shader_stages)
                vkDestroyShaderModule(logical_device, shader_stage.module, VkDebug::Allocation());

            return false;
        }

        std::cout << "Creating Graphics Pipeline with " << shader_stages.size() << " shaders." << std::endl;
        VkGraphicsPipelineCreateInfo pipelineInfo{};
        pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        pipelineInfo.stageCount = shader_stages.size();
        pipelineInfo.pStages = shader_stages.data();
        pipelineInfo.layout = layout;
        pipelineInfo.renderPass = render_pass;
        pipelineInfo.subpass = 0;
        pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
        pipelineInfo.basePipelineIndex = -1;

        // Static States - VertexInput from Shading
        std::vector<VkVertexInputBindingDescription> binding_descriptions{};
        shading->BindingDescriptions(binding_descriptions);
        std::vector<VkVertexInputAttributeDescription> attribute_descriptions{};
        shading->AttributeDescriptions(attribute_descriptions);

        // Static States - VertexInput
        VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
        vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
        vertexInputInfo.vertexBindingDescriptionCount = static_cast<uint32_t>(binding_descriptions.size());
        vertexInputInfo.pVertexBindingDescriptions = binding_descriptions.data();
        vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attribute_descriptions.size());
        vertexInputInfo.pVertexAttributeDescriptions = attribute_descriptions.data();
        pipelineInfo.pVertexInputState = &vertexInputInfo;

        // Static States - InputAssembly, Multisampling, ColorBlend
        VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
        inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        inputAssembly.primitiveRestartEnable = VK_FALSE;
        pipelineInfo.pInputAssemblyState = &inputAssembly;

        VkPipelineMultisampleStateCreateInfo multisampling{};
        multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
        multisampling.sampleShadingEnable = VK_FALSE;
        multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
        multisampling.minSampleShading = 1.0f;
        multisampling.pSampleMask = nullptr;
        multisampling.alphaToCoverageEnable = VK_FALSE;
        multisampling.alphaToOneEnable = VK_FALSE;
        pipelineInfo.pMultisampleState = &multisampling;

        VkPipelineColorBlendAttachmentState colorBlendAttachment{};
        colorBlendAttachment.colorWriteMask =
            VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
        colorBlendAttachment.blendEnable = VK_FALSE;

        VkPipelineColorBlendStateCreateInfo colorBlending{};
        colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
        colorBlending.logicOpEnable = VK_FALSE;
        colorBlending.logicOp = VK_LOGIC_OP_COPY;
        colorBlending.attachmentCount = 1;
        colorBlending.pAttachments = &colorBlendAttachment;
        colorBlending.blendConstants[0] = 0.0f;
        colorBlending.blendConstants[1] = 0.0f;
        colorBlending.blendConstants[2] = 0.0f;
        colorBlending.blendConstants[3] = 0.0f;
        pipelineInfo.pColorBlendState = &colorBlending;

        // Dynamic States
        VkPipelineViewportStateCreateInfo viewportState{};
        VkPipelineDepthStencilStateCreateInfo depth_stencil{};
        VkPipelineRasterizationStateCreateInfo rasterizer{};
        VkPipelineDynamicStateCreateInfo dynamicState{};
        std::vector<VkDynamicState> states{};
        dynamic_state.PopulatePipelineCreateInfo(pipelineInfo, viewportState, depth_stencil, rasterizer, dynamicState,
                                                 states);

        // Build Graphics Pipeline
        bool success = vkCreateGraphicsPipelines(logical_device, VK_NULL_HANDLE, 1, &pipelineInfo,
                                                 VkDebug::Allocation(), &pipeline) == VK_SUCCESS;
        if (!success)
            std::cerr << "Failed to create graphics pipeline!" << std::endl;

        // Release shader Modules
        for (const auto& shader_stage : shader_stages)
            vkDestroyShaderModule(logical_device, shader_stage.module, VkDebug::Allocation());

        return success;
    }

    // Pipeline Resources' Creation

    bool CreateCommandPool(uint32_t graphics_family)
    {
        VkCommandPoolCreateInfo poolInfo{};
        poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        poolInfo.queueFamilyIndex = graphics_family;
        poolInfo.flags = 0;

        std::cout << "Creating Command Pool." << std::endl;
        if (vkCreateCommandPool(logical_device, &poolInfo, VkDebug::Allocation(), &cmd_pool) != VK_SUCCESS)
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
        allocInfo.commandPool = cmd_pool;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandBufferCount = 1;

        std::cout << "Creating Command Buffer." << std::endl;
        if (vkAllocateCommandBuffers(logical_device, &allocInfo, &cmd_buffer) != VK_SUCCESS)
        {
            std::cerr << "Failed to create Command Buffer!" << std::endl;
            return false;
        }
        return true;
    }

    bool CreateSemaphores()
    {
        std::cout << "Creating Semaphores." << std::endl;
        VkSemaphoreCreateInfo semaphoreInfo{};
        semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
        return vkCreateSemaphore(logical_device, &semaphoreInfo, VkDebug::Allocation(), &imageAvailableSemaphore) ==
                   VK_SUCCESS &&
               vkCreateSemaphore(logical_device, &semaphoreInfo, VkDebug::Allocation(), &renderFinishedSemaphore) ==
                   VK_SUCCESS;
    }

    // Rendering - Prepare

    bool BeginCommandRecording() const
    {
        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        if (vkBeginCommandBuffer(cmd_buffer, &beginInfo) != VK_SUCCESS)
        {
            std::cerr << "Failed to Begin Command Buffer." << std::endl;
            return false;
        }

        return true;
    }

    void BeginRenderPass(VkFramebuffer framebuffer, const std::vector<VkClearValue>& clear_values,
                         VkOffset2D offset = {0, 0}) const
    {
        VkRenderPassBeginInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.renderPass = render_pass;
        renderPassInfo.framebuffer = framebuffer;
        renderPassInfo.renderArea.offset = offset;
        renderPassInfo.renderArea.extent = swapchain.extent;
        renderPassInfo.clearValueCount = static_cast<uint32_t>(clear_values.size());
        renderPassInfo.pClearValues = clear_values.data();

        vkCmdBeginRenderPass(cmd_buffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
    }
    
    // Rendering - Submit & Present

    bool EndCommandRecording() const
    {
        if (vkEndCommandBuffer(cmd_buffer) != VK_SUCCESS)
        {
            std::cerr << "Failed to end command buffer." << std::endl;
            return false;
        }

        return true;
    }

    bool SubmitCommands(VkQueue graphics_queue, const std::vector<VkCommandBuffer>& cmd_buffers) const
    {
        // Submit the command buffer.
        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

        VkSemaphore waitSemaphores[] = {imageAvailableSemaphore};
        VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
        submitInfo.waitSemaphoreCount = 1;
        submitInfo.pWaitSemaphores = waitSemaphores;
        submitInfo.pWaitDstStageMask = waitStages;

        submitInfo.commandBufferCount = static_cast<uint32_t>(cmd_buffers.size());
        submitInfo.pCommandBuffers = cmd_buffers.data();

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

    bool Present(VkQueue graphics_queue, const uint32_t& next_swapchain_image) const
    {
        // Present the image.
        VkPresentInfoKHR presentInfo{};
        presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
        presentInfo.waitSemaphoreCount = 1;
        presentInfo.pWaitSemaphores = &renderFinishedSemaphore;
        presentInfo.swapchainCount = 1;
        presentInfo.pSwapchains = &swapchain.id;
        presentInfo.pImageIndices = &next_swapchain_image;

        if (vkQueuePresentKHR(graphics_queue, &presentInfo) != VK_SUCCESS)
        {
            std::cerr << "Failed to present swapchain image." << std::endl;
            return false;
        }

        return true;
    }
};
