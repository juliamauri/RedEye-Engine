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

#include <MathGeoLib.h>

export module Camera;

export enum class AspectRatio : unsigned short {
    Fit_Window = 0,
    Square_1x1,
    TraditionalTV_4x3,
    Movietone_16x9
};

export struct Camera
{
    AspectRatio target_ar = AspectRatio::Fit_Window;
    math::float2 bounds = {500.f, 500.f};
    math::Frustum frustum{};

    Camera(FrustumType type = PerspectiveFrustum)
    {
        switch (type)
        {
            case OrthographicFrustum: SetOrthographic(); break;
            case PerspectiveFrustum: SetPerspective(); break;
            default: break;
        }
    }

    // Perspective & Orthographic
    void SetOrthographic(
        math::float3 pos = math::float3::zero,
        math::float3 front = math::float3::unitZ,
        math::float3 up = math::float3::unitY,
        math::float2 size = { 500.f, 500.f },
        float near_dist = 1.f, float far_dist = 10.f)
    {
        frustum.type = OrthographicFrustum;
        frustum.pos = pos;
        frustum.front = front;
        frustum.up = up;
        frustum.nearPlaneDistance = near_dist;
        frustum.farPlaneDistance = far_dist;
        SetBounds(size.x, size.y);
    }

    void SetPerspective(
        math::float3 pos = math::float3::zero,
        math::float3 front = math::float3::unitZ,
        math::float3 up = math::float3::unitY,
        math::float2 size = { 500.f, 500.f },
        float v_fov_degrees = 30.f,
        float near_dist = 1.0f, float far_dist = 10.f)
    {
        frustum.type = PerspectiveFrustum;
        frustum.pos = pos;
        frustum.front = front;
        frustum.up = up;
        frustum.nearPlaneDistance = near_dist;
        frustum.farPlaneDistance = far_dist;
        frustum.verticalFov = math::DegToRad(v_fov_degrees);
        SetBounds(size.x, size.y);
    }

    inline float AspectRatio() const { return bounds.x / bounds.y; }
    inline float NearDist() const { return frustum.nearPlaneDistance; }
    inline float FarDist() const { return frustum.farPlaneDistance; }
    void SetBounds(float width, float height)
    {
        switch (target_ar)
        {
            default: bounds = {width, height}; break;
            case AspectRatio::Square_1x1:
            {
                if (width >= height)
                    bounds.x = bounds.y = height;
                else
                    bounds.x = bounds.y = width;
                break;
            }
            case AspectRatio::TraditionalTV_4x3:
            {
                if (width / 4.f >= height / 3.f)
                    bounds = {(height * 4.f) / 3.f, height};
                else 
                    bounds = {width, (width * 3.0f) / 4.0f};
                break;
            }
            case AspectRatio::Movietone_16x9:
            {
                if (width / 16.0f >= height / 9.0f)
                    bounds = {(height * 16.0f) / 9.0f, height};
                else
                    bounds = {width, (width * 9.0f) / 16.0f};
                break;
            }
        }

        switch (frustum.type)
        {
            case OrthographicFrustum:
                frustum.orthographicWidth = bounds.x;
                frustum.orthographicHeight = bounds.y;
                break;
            case PerspectiveFrustum:
                frustum.horizontalFov = 2.0f * math::Atan(math::Tan(frustum.verticalFov / 2.0f) * AspectRatio());
                break;
            default: break;
        }
    }
    void SetFOVDegrees(float vfov_degrees)
    {
        frustum.verticalFov = math::DegToRad(vfov_degrees);
        frustum.horizontalFov = 2.0f * math::Atan(math::Tan(frustum.verticalFov / 2.0f) * AspectRatio());
    }

    // Camera Controls
    void Pan(float rad_dx, float rad_dy, float rad_dz = 0.f)
    {
        if (rad_dx == 0.f && rad_dy == 0.f && rad_dz == 0.f)
            return;

        frustum.front = 
            math::Quat::RotateX(rad_dx) * 
            math::Quat::RotateY(rad_dy) * 
            math::Quat::RotateZ(rad_dz) * frustum.front;
    }
    void Orbit(float rad_dx, float rad_dy, math::float3 center)
    {
        if (rad_dx == 0.f && rad_dy == 0.f)
            return;

        frustum.front = 
            math::Quat::RotateX(-rad_dx) * 
            math::Quat::RotateY(-rad_dy) * frustum.front;

        frustum.front.Normalize();

        frustum.pos = center - (frustum.front * frustum.pos.Distance(center));
    }
    void Focus(math::float3 center, float radius = 1.f, float min_dist = 3.f)
    {
        if (radius <= 0)
            return;

        float camDistance = min_dist;

        // Vertical distance
        float v_dist = radius / math::Sin(frustum.verticalFov / 2.0f);
        if (v_dist > camDistance)
            camDistance = v_dist;

        // Horizontal distance
        float h_dist = radius / math::Sin(frustum.horizontalFov / 2.0f);
        if (h_dist > camDistance)
            camDistance = h_dist;

        frustum.pos = center - (frustum.front * camDistance);
    }
    void Focus(math::AABB box, float min_dist = 3.f)
    {
        Focus(box.CenterPoint(), box.HalfSize().Length(), min_dist);
    }

    // Projection
    math::float4x4 View() const
    {
        math::float3 right = frustum.up.Cross(frustum.front);

        return math::float4x4(
            right.x, frustum.up.x, frustum.front.x, 0.0f,
            right.y, frustum.up.y, frustum.front.y, 0.0f,
            right.z, frustum.up.z, frustum.front.z, 0.0f,
            -right.Dot(frustum.pos),
            -frustum.up.Dot(frustum.pos), 
            -frustum.front.Dot(frustum.pos), 1.0f);
    }

    math::float4x4 Projection(bool right_handed = true) const
    {
        return math::float4x4::identity;
        switch (frustum.type)
        {
            case OrthographicFrustum:
            {
                //return math::float4x4::D3DOrthoProjLH(
                //    frustum.nearPlaneDistance, frustum.farPlaneDistance,
                //    bounds.x, bounds.y);








                float _00 = 2.f / bounds.x;
                float _11 = (right_handed ? -2.f : 2.f) / bounds.y;
                float _22 = -2.f / (frustum.farPlaneDistance - frustum.nearPlaneDistance);
                float _32 = -(frustum.farPlaneDistance + frustum.nearPlaneDistance) /
                            (frustum.farPlaneDistance - frustum.nearPlaneDistance);

                return math::float4x4(
                    _00, 0.f, 0.f, 0.f,
                    0.f, _11, 0.f, 0.f,
                    0.f, 0.f, _22, 0.f,
                    0.f, 0.f, _32, 1.f);
            }
            case PerspectiveFrustum:
            {
                //float _00 = ;
                //float _11 = ;
                //float _22 = ;

                 return math::float4x4(
                    2.0f * NearDist() / bounds.x, 0, 0, 0,
                    0, 2.0f * NearDist() / bounds.y, 0, 0, 
                    0, 0, FarDist() / (NearDist() - FarDist()),
                    NearDist() * FarDist() / (NearDist() - FarDist()),
                    0, 0, -1, 0);
            }
            default:
                return math::float4x4::identity;
        }
    }
};
