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
import Swapchain;
import PipelineDynamicStates;

export struct GraphicPipeline
{
    VkDevice logical_device = VK_NULL_HANDLE;
    VkPipeline pipeline = VK_NULL_HANDLE;

    // State
    VkClearColorValue clear_color = {0.f, 0.f, 0.f, 1.f};
    PipelineDynamicStates dynamic_state{};

    // Swapchain
    Swapchain swapchain{};
    std::vector<VkImage> images{};
    std::vector<VkImageView> image_views{};    // Image views for swapchain images.
    std::vector<VkFramebuffer> framebuffers{}; // Framebuffers for swapchain images.

    // Commands
    VkCommandPool commandPool = VK_NULL_HANDLE;
    VkCommandBuffer commandBuffer = VK_NULL_HANDLE;

    // Semaphores
    VkSemaphore imageAvailableSemaphore = VK_NULL_HANDLE;
    VkSemaphore renderFinishedSemaphore = VK_NULL_HANDLE;

    VkRenderPass renderPass = VK_NULL_HANDLE;         // Describes rendering operations.
    VkPipelineLayout pipelineLayout = VK_NULL_HANDLE; // Manages shaders and resources.

    VkDescriptorSetLayout descriptorSetLayout = VK_NULL_HANDLE; // Describes the layout of descriptor sets.
    VkDescriptorPool descriptorPool = VK_NULL_HANDLE;           // Manages descriptor sets.

    bool Create(const Shading* shading, const LogicalDevice& device, VkSurfaceKHR surface,
                VkSurfaceCapabilitiesKHR& surface_capabilities,
                const VkExtent2D& window_size)
    {
        if (shading == nullptr)
        {
            std::cerr << "Error: Shading is null!" << std::endl;
            return false;
        }

        if (!swapchain.Create(device, surface, surface_capabilities, window_size))
        {
            std::cerr << "Failed to create swapchain for Graphic Pipeline." << std::endl;
            return false;
        }

        logical_device = device.logical_device;
        dynamic_state.Setup(swapchain.extent);

        if (!RetrieveImages(device.surface_format) ||
            !CreateCommandPool(device.graphics_family) ||
            !CreateCommandBuffer() || 
            !CreateSemaphores() ||
            !CreateRenderPass(device.surface_format) ||
            !shading->CreateDescriptorSetLayout(logical_device, descriptorSetLayout) ||
            !CreatePipelineLayout() || 
            !CreateGraphicsPipeline(shading) ||
            !shading->CreateDescriptorPool(logical_device, descriptorPool))
        {
            std::cerr << "Failed to create Graphic Pipeline." << std::endl;
            return false;
        }
        
        return true;
    }

    void Delete()
    {
        if (pipelineLayout != VK_NULL_HANDLE)
            vkDestroyPipelineLayout(logical_device, pipelineLayout, VkDebug::Allocation());
        if (renderPass != VK_NULL_HANDLE)
            vkDestroyRenderPass(logical_device, renderPass, VkDebug::Allocation());

        for (auto framebuffer : framebuffers)
            vkDestroyFramebuffer(logical_device, framebuffer, VkDebug::Allocation());
        for (auto imageView : image_views)
            vkDestroyImageView(logical_device, imageView, VkDebug::Allocation());

        if (swapchain.swapchain != VK_NULL_HANDLE)
            vkDestroySwapchainKHR(logical_device, swapchain.swapchain, VkDebug::Allocation());
        if (commandPool != VK_NULL_HANDLE)
            vkDestroyCommandPool(logical_device, commandPool, VkDebug::Allocation());

        if (imageAvailableSemaphore != VK_NULL_HANDLE)
            vkDestroySemaphore(logical_device, imageAvailableSemaphore, VkDebug::Allocation());
        if (renderFinishedSemaphore != VK_NULL_HANDLE)
            vkDestroySemaphore(logical_device, renderFinishedSemaphore, VkDebug::Allocation());

        if (descriptorSetLayout != VK_NULL_HANDLE)
            vkDestroyDescriptorSetLayout(logical_device, descriptorSetLayout, VkDebug::Allocation());
        if (descriptorPool != VK_NULL_HANDLE)
            vkDestroyDescriptorPool(logical_device, descriptorPool, VkDebug::Allocation());
    }
    
    bool PrepareRender(const Shading* shading, LogicalDevice& device, VkSurfaceKHR surface,
                       VkSurfaceCapabilitiesKHR& surface_capabilities,
                       const VkExtent2D& window_size, uint32_t& next_swapchain_image)
    {
        if (!ValidatePipeline(shading, device, surface, surface_capabilities, window_size) ||
            !GetNextImage(next_swapchain_image) || !BeginCommandRecording())
            return false;

        BeginRenderPass(framebuffers[next_swapchain_image]);
        vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
        dynamic_state.Update(commandBuffer);
        return true;
    }

    bool SubmitRender(VkQueue graphics_queue, uint32_t& next_swapchain_image)
    {
        vkCmdEndRenderPass(commandBuffer);
        return EndCommandRecording() && SubmitCommands(graphics_queue) && Present(graphics_queue, next_swapchain_image);
    }

  private:
    bool RetrieveImages(const VkSurfaceFormatKHR& surface_format)
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
            createInfo.format = surface_format.format;
            createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            createInfo.subresourceRange.baseMipLevel = 0;
            createInfo.subresourceRange.levelCount = 1;
            createInfo.subresourceRange.baseArrayLayer = 0;
            createInfo.subresourceRange.layerCount = 1;

            if (vkCreateImageView(logical_device, &createInfo, VkDebug::Allocation(), &image_views[i]) != VK_SUCCESS)
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

            if (vkCreateFramebuffer(logical_device, &framebufferInfo, VkDebug::Allocation(), &framebuffers[i]) !=
                VK_SUCCESS)
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
        if (vkCreateCommandPool(logical_device, &poolInfo, VkDebug::Allocation(), &commandPool) != VK_SUCCESS)
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
        if (vkCreateRenderPass(logical_device, &renderPassInfo, VkDebug::Allocation(), &renderPass) != VK_SUCCESS)
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
        return vkCreateSemaphore(logical_device, &semaphoreInfo, VkDebug::Allocation(), &imageAvailableSemaphore) ==
                   VK_SUCCESS &&
               vkCreateSemaphore(logical_device, &semaphoreInfo, VkDebug::Allocation(), &renderFinishedSemaphore) ==
                   VK_SUCCESS;
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

        if (vkCreatePipelineLayout(logical_device, &pipelineLayoutInfo, VkDebug::Allocation(), &pipelineLayout) !=
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
        pipelineInfo.layout = pipelineLayout;
        pipelineInfo.renderPass = renderPass;
        pipelineInfo.subpass = 0;
        pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
        pipelineInfo.basePipelineIndex = -1;

        // Static States - VertexInput
        VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
        std::vector<VkVertexInputBindingDescription> binding_descriptions{};
        std::vector<VkVertexInputAttributeDescription> attribute_descriptions{};

        shading->BindingDescriptions(binding_descriptions);
        shading->AttributeDescriptions(attribute_descriptions);

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

    // Rendering

    bool ValidatePipeline(const Shading* shading, LogicalDevice& device, VkSurfaceKHR surface,
                          VkSurfaceCapabilitiesKHR& surface_capabilities,
                          const VkExtent2D& window_size)
    {
        const uint8_t changes = swapchain.GetChanges(surface_capabilities);
        if (changes != 0)
        {
            std::cout << "Recreating swapchain..." << std::endl;
            if (changes & Swapchain::CapabilityChanges::Transform)
                std::cout << "Surface transformation changed. ";
            if (changes & Swapchain::CapabilityChanges::Extent)
                std::cout << "Surface extent changed. ";

            // Freeing old resources
            for (auto framebuffer : framebuffers)
                vkDestroyFramebuffer(logical_device, framebuffer, VkDebug::Allocation());
            for (auto imageView : image_views)
                vkDestroyImageView(logical_device, imageView, VkDebug::Allocation());

            if (!swapchain.Create(device, surface, surface_capabilities, window_size, swapchain.swapchain) ||
                !RetrieveImages(device.surface_format))
            {
                std::cerr << "Failed to recreate swapchain!" << std::endl;
                return false;
            }

            if (changes & Swapchain::CapabilityChanges::Extent)
                dynamic_state.OnSwapchainExtentChanged(swapchain.extent);
        }

        if (dynamic_state.requires_recreation && !CreateGraphicsPipeline(shading))
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

    bool Present(VkQueue graphics_queue, const uint32_t& next_swapchain_image) const
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
