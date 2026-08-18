#include "Camera.h"

#include <cassert>
#include <SDL_keyboard.h>
#include <SDL_mouse.h>

namespace dae
{
    Camera::Camera()
    {
        UpdateViewMatrixCache();
        UpdateProjMatrixCache();
        UpdateFrustumPlanesCache();
    }

    Camera::Camera(const Vector3& position, const Vector3& rotation) :
        m_Position(position), m_Rotation(rotation), m_Fov(m_Fov)
    {
        UpdateViewMatrixCache();
        UpdateProjMatrixCache();
        UpdateFrustumPlanesCache();
    }

    Camera::Camera(const Vector3& position, const Vector3& rotation, float fov,
        float aspect, float nearClip, float farClip) : m_Position(position), m_Rotation(rotation),
        m_Fov(fov), m_Aspect(aspect), m_NearClip(nearClip), m_FarClip(farClip)
    {
        UpdateViewMatrixCache();
        UpdateProjMatrixCache();
        UpdateFrustumPlanesCache();
    }

    void Camera::UpdateViewMatrixCache() const
    {
        m_ViewMatrix = Matrix::Inverse(
            Matrix::CreateRotation(m_Rotation) *
            Matrix::CreateTranslation(m_Position)
            );

        m_ViewMatrixDirty = false;
    }

    void Camera::UpdateProjMatrixCache() const
    {
        m_ProjMatrix = Matrix::CreatePerspectiveFovLH(m_Fov * TO_RADIANS,
            m_Aspect, m_NearClip, m_FarClip);

        m_ProjMatrixDirty = false;
    }

    void Camera::UpdateFrustumPlanesCache() const
    {
        if (m_ViewMatrixDirty)
            UpdateViewMatrixCache();

        if (m_ProjMatrixDirty)
            UpdateProjMatrixCache();

        Matrix viewProj { m_ViewMatrix * m_ProjMatrix };

        // Left plane
        m_FrustumPlanes[0] = {
            viewProj[0][3] + viewProj[0][0],
            viewProj[1][3] + viewProj[1][0],
            viewProj[2][3] + viewProj[2][0],
            viewProj[3][3] + viewProj[3][0]
        };

        // Right plane
        m_FrustumPlanes[1] = {
            viewProj[0][3] - viewProj[0][0],
            viewProj[1][3] - viewProj[1][0],
            viewProj[2][3] - viewProj[2][0],
            viewProj[3][3] - viewProj[3][0]
        };

        // Bottom plane
        m_FrustumPlanes[2] = {
            viewProj[0][3] + viewProj[0][1],
            viewProj[1][3] + viewProj[1][1],
            viewProj[2][3] + viewProj[2][1],
            viewProj[3][3] + viewProj[3][1]
        };

        // Top plane
        m_FrustumPlanes[3] = {
            viewProj[0][3] - viewProj[0][1],
            viewProj[1][3] - viewProj[1][1],
            viewProj[2][3] - viewProj[2][1],
            viewProj[3][3] - viewProj[3][1]
        };

        // Near plane
        m_FrustumPlanes[4] = {
            viewProj[0][3] + viewProj[0][2],
            viewProj[1][3] + viewProj[1][2],
            viewProj[2][3] + viewProj[2][2],
            viewProj[3][3] + viewProj[3][2]
        };

        // Far plane
        m_FrustumPlanes[5] = {
            viewProj[0][3] - viewProj[0][2],
            viewProj[1][3] - viewProj[1][2],
            viewProj[2][3] - viewProj[2][2],
            viewProj[3][3] - viewProj[3][2]
        };

        // Normalize all planes
        for (int i = 0; i < 6; ++i)
        {
            float len = sqrtf(m_FrustumPlanes[i].x * m_FrustumPlanes[i].x +
                m_FrustumPlanes[i].y * m_FrustumPlanes[i].y +
                m_FrustumPlanes[i].z * m_FrustumPlanes[i].z);

            m_FrustumPlanes[i].x /= len;
            m_FrustumPlanes[i].y /= len;
            m_FrustumPlanes[i].z /= len;
            m_FrustumPlanes[i].w /= len;
        }
    }

    void Camera::SetPosition(const Vector3& position)
    {
        if (m_Position == position)
            return;

        m_Position = position;

        m_ViewMatrixDirty = true;
    }

    void Camera::SetRotation(const Vector3& rotation)
    {
        if (m_Rotation == rotation)
            return;

        m_Rotation = rotation;

        m_ViewMatrixDirty = true;
    }

    void Camera::SetFOV(float fov)
    {
        if (m_Fov == fov)
            return;

        m_Fov = fov;

        m_ProjMatrixDirty = true;
    }

    void Camera::SetAspect(float aspect)
    {
        if (m_Aspect == aspect)
            return;

        m_Aspect = aspect;

        m_ProjMatrixDirty = true;
    }

    void Camera::SetNearClip(float nearClip)
    {
        if (m_NearClip == nearClip)
            return;

        m_NearClip = nearClip;

        m_ProjMatrixDirty = true;
    }

    void Camera::SetFarClip(float farClip)
    {
        if (m_FarClip == farClip)
            return;

        m_FarClip = farClip;

        m_ProjMatrixDirty = true;
    }

    void Camera::Update(Timer* timer)
    {
        const float deltaTime { timer->GetElapsed() };

        // Movement logic here

        const uint8_t* pKeyboardState = SDL_GetKeyboardState(nullptr);
        const bool wKey { static_cast<bool>(pKeyboardState[SDL_SCANCODE_W]) ||
            static_cast<bool>(pKeyboardState[SDL_SCANCODE_UP]) };
        const bool aKey { static_cast<bool>(pKeyboardState[SDL_SCANCODE_A]) ||
            static_cast<bool>(pKeyboardState[SDL_SCANCODE_LEFT]) };
        const bool sKey { static_cast<bool>(pKeyboardState[SDL_SCANCODE_S]) ||
            static_cast<bool>(pKeyboardState[SDL_SCANCODE_DOWN]) };
        const bool dKey { static_cast<bool>(pKeyboardState[SDL_SCANCODE_D]) ||
            static_cast<bool>(pKeyboardState[SDL_SCANCODE_RIGHT]) };
        const bool eKey { static_cast<bool>(pKeyboardState[SDL_SCANCODE_E]) };
        const bool qKey { static_cast<bool>(pKeyboardState[SDL_SCANCODE_Q]) };
        const bool shiftKey {
            static_cast<bool>(pKeyboardState[SDL_SCANCODE_LSHIFT]) };

        Vector3 translationInput { };

        if (dKey && !aKey)
            translationInput.x = 1.0f;
        else if (!dKey && aKey)
            translationInput.x = -1.0f;

        if (eKey && !qKey)
            translationInput.y = 1.0f;
        else if (!eKey && qKey)
            translationInput.y = -1.0f;

        if (wKey && !sKey)
            translationInput.z = 1.0f;
        else if (!wKey && sKey)
            translationInput.z = -1.0f;

        translationInput.SafeNormalize() > 0.0f;

        // Mouse input:
        int mouseX { }, mouseY { };
        const uint32_t mouseState = SDL_GetRelativeMouseState(&mouseX, &mouseY);
        const bool lmb { (mouseState & SDL_BUTTON(SDL_BUTTON_LEFT)) != 0 };
        const bool rmb { (mouseState & SDL_BUTTON(SDL_BUTTON_RIGHT)) != 0 };

        // Controller parameters:
        const float shiftSpeedMult { 5.0f };

        const float keyboardSpeed { 20.0f * (shiftKey ? shiftSpeedMult : 1.0f) * deltaTime };
        const float mouseSpeed { 0.03f * (shiftKey ? shiftSpeedMult : 1.0f) };
        const float radiansDelta { 0.005f };

        Vector2 localRotation { };
        Vector3 translation { };

        // Control modes:
        if (rmb && !lmb)
        {
            localRotation.x = radiansDelta * -mouseY;
            localRotation.y = radiansDelta * mouseX;

            translation = translationInput * keyboardSpeed;
        }
        else if (!rmb && lmb)
        {
            translation.z = -mouseY * mouseSpeed;

            localRotation.y = radiansDelta * mouseX;
        }
        else if (rmb && lmb)
        {
            translation.x = -mouseX * mouseSpeed;
            translation.y = mouseY * mouseSpeed;
        }
        else
        {
            translation = translationInput * keyboardSpeed;
        }

        m_TotalPitch += localRotation.x;
        m_TotalPitch = std::clamp(m_TotalPitch, -PI_DIV_2, PI_DIV_2);
        m_TotalYaw += localRotation.y;

        SetRotation(Vector3 { m_TotalPitch, m_TotalYaw, 0.0f });

        const Matrix rot { Matrix::CreateRotation(m_Rotation) };

        const Vector3 forward { rot.TransformVector(Vector3::UnitZ) };
        const Vector3 right { rot.TransformVector(Vector3::UnitX) };
        const Vector3 up { rot.TransformVector(Vector3::UnitY) };

        SetPosition(m_Position + (forward * translation.z) + (right * translation.x) +
            (up * translation.y));
    }

    const Matrix& Camera::GetViewMatrix() const
    {
        if (m_ViewMatrixDirty)
            UpdateViewMatrixCache();

        return m_ViewMatrix;
    }

    const Matrix& Camera::GetProjMatrix() const
    {
        if (m_ProjMatrixDirty)
            UpdateProjMatrixCache();

        return m_ProjMatrix;
    }

    bool Camera::IsInFrustum(const Bounds& bounds) const
    {
        if (m_ViewMatrixDirty || m_ProjMatrixDirty)
            UpdateFrustumPlanesCache();

        const Vector3& bmin { bounds.min };
        const Vector3& bmax { bounds.max };

        for (int i = 0; i < 6; ++i)
        {
            const Vector4& p = m_FrustumPlanes[i];

            // Positive vertex (the one most in direction of the plane normal)
            Vector3 positive {
                p.x >= 0 ? bmax.x : bmin.x,
                p.y >= 0 ? bmax.y : bmin.y,
                p.z >= 0 ? bmax.z : bmin.z
            };

            // Distance from plane: ax + by + cz + d
            float distance =
                p.x * positive.x +
                p.y * positive.y +
                p.z * positive.z +
                p.w;

            if (distance < 0.0f)
                return false; // Completely outside
        }

        return true;
    }

    Vector3 Camera::GetPosition()
    {
        return m_Position;
    }
}