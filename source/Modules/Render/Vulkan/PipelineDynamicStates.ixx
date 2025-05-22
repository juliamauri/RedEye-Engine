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

#include <vector>

export module PipelineDynamicStates;

void ViewportState(VkPipelineViewportStateCreateInfo& create_info, VkRect2D& scissor, VkViewport& viewport)
{
    create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    create_info.viewportCount = 1;
    create_info.pViewports = &viewport;
    create_info.scissorCount = 1;
    create_info.pScissors = &scissor;
}
void RasterizationState(VkPipelineRasterizationStateCreateInfo& create_info, float lineWidth,
                               bool depthBiasEnabled, float depthBiasConstantFactor, float depthBiasClamp,
                               float depthBiasSlopeFactor)
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

export struct PipelineDynamicStates
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
                                    VkPipelineDynamicStateCreateInfo& dynamicState, std::vector<VkDynamicState>& states)
    {
        requires_recreation = false;

        // Setup Rasterization State Create Info
        RasterizationState(rasterizer, line_width, current_flags & DEPTH_BIAS_ENABLED,
                                                       depth_bias[0], depth_bias[1], depth_bias[2]);
        pipelineInfo.pRasterizationState = &rasterizer;

        if (current_flags == 0)
        {
            ViewportState(viewportState, scissor, viewport);
            pipelineInfo.pViewportState = &viewportState;
            pipelineInfo.pDepthStencilState = nullptr;
            pipelineInfo.pDynamicState = nullptr;
            return;
        }

        // Fill Dynamic States' Array
        states.clear();
        if (current_flags & VIEWPORT)
            states.push_back(VK_DYNAMIC_STATE_VIEWPORT);
        if (current_flags & SCISSOR)
            states.push_back(VK_DYNAMIC_STATE_SCISSOR);
        if (current_flags & LINE_WIDTH)
            states.push_back(VK_DYNAMIC_STATE_LINE_WIDTH);
        if (current_flags & DEPTH_BIAS)
            states.push_back(VK_DYNAMIC_STATE_DEPTH_BIAS);
        if (current_flags & BLEND_CONSTANTS)
            states.push_back(VK_DYNAMIC_STATE_BLEND_CONSTANTS);
        if (current_flags & DEPTH_BOUNDS)
            states.push_back(VK_DYNAMIC_STATE_DEPTH_BOUNDS);
        if (current_flags & STENCIL_COMPARE_MASK)
            states.push_back(VK_DYNAMIC_STATE_STENCIL_COMPARE_MASK);
        if (current_flags & STENCIL_WRITE_MASK)
            states.push_back(VK_DYNAMIC_STATE_STENCIL_WRITE_MASK);
        if (current_flags & STENCIL_REFERENCE)
            states.push_back(VK_DYNAMIC_STATE_STENCIL_REFERENCE);

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
            ViewportState(viewportState, scissor, viewport);
            pipelineInfo.pViewportState = &viewportState;
        }

        // Setup Depth Stencil State Create Info
        if (current_flags & (DEPTH_BOUNDS | STENCIL_COMPARE_MASK | STENCIL_WRITE_MASK | STENCIL_REFERENCE))
        {
            DepthStencilState(depthStencilState);
            pipelineInfo.pDepthStencilState = &depthStencilState;
        }
        else
            pipelineInfo.pDepthStencilState = nullptr;
    }

    void OnSwapchainExtentChanged(const VkExtent2D& next_extent)
    {
        if (swapchain_extent.width == next_extent.width && swapchain_extent.height == next_extent.height)
            return;

        // New extent linear interpolation
        float width_ratio = static_cast<float>(next_extent.width) / swapchain_extent.width;
        float height_ratio = static_cast<float>(next_extent.height) / swapchain_extent.height;

        UpdateViewport({viewport.x * width_ratio, viewport.y * height_ratio, viewport.width * width_ratio,
                        viewport.height * height_ratio, viewport.minDepth, viewport.maxDepth});
        UpdateScissor({{static_cast<int32_t>(scissor.offset.x * width_ratio),
                        static_cast<int32_t>(scissor.offset.y * height_ratio)},
                       {static_cast<uint32_t>(scissor.extent.width * width_ratio),
                        static_cast<uint32_t>(scissor.extent.height * height_ratio)}});
    }

    void UpdateViewport(const VkViewport& next_viewport)
    {
        if (viewport.x == next_viewport.x && viewport.y == next_viewport.y && viewport.width == next_viewport.width &&
            viewport.height == next_viewport.height && viewport.minDepth == next_viewport.minDepth &&
            viewport.maxDepth == next_viewport.maxDepth)
            return;

        viewport = next_viewport;
        update_flags |= VIEWPORT;
        bool has_dyn_viewport = current_flags & VIEWPORT;
        requires_recreation |= !has_dyn_viewport;
    }

    void UpdateScissor(const VkRect2D& next_scissor)
    {
        if (scissor.offset.x == next_scissor.offset.x && scissor.offset.y == next_scissor.offset.y &&
            scissor.extent.width == next_scissor.extent.width && scissor.extent.height == next_scissor.extent.height)
            return;

        scissor = next_scissor;
        update_flags |= SCISSOR;
        bool has_dyn_scissor = current_flags & SCISSOR;
        requires_recreation |= !has_dyn_scissor;
    }

    void Update(VkCommandBuffer commandBuffer)
    {
        if (update_flags == 0)
            return;
        if (update_flags & VIEWPORT)
            vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
        if (update_flags & SCISSOR)
            vkCmdSetScissor(commandBuffer, 0, 1, &scissor);
        if (update_flags & LINE_WIDTH)
            vkCmdSetLineWidth(commandBuffer, line_width);
        if (update_flags & DEPTH_BIAS)
            vkCmdSetDepthBias(commandBuffer, depth_bias[0], depth_bias[1], depth_bias[2]);
        if (update_flags & BLEND_CONSTANTS)
            vkCmdSetBlendConstants(commandBuffer, blend_constants);
        if (update_flags & DEPTH_BOUNDS)
            vkCmdSetDepthBounds(commandBuffer, depth_bounds[0], depth_bounds[1]);
        if (update_flags & STENCIL_COMPARE_MASK)
            vkCmdSetStencilCompareMask(commandBuffer, compare.face_mask, compare.mask);
        if (update_flags & STENCIL_WRITE_MASK)
            vkCmdSetStencilWriteMask(commandBuffer, write.face_mask, write.mask);
        if (update_flags & STENCIL_REFERENCE)
            vkCmdSetStencilReference(commandBuffer, reference.face_mask, reference.mask);
        update_flags = 0;
    }
};
