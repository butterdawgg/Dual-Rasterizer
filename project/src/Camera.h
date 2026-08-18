#pragma once

#include "Maths.h"
#include "Timer.h"
#include "Bounds.h"

namespace dae
{
    class Camera final
    {
        private:

        Vector3 m_Position { };
        Vector3 m_Rotation { };

        float m_Fov { 90.0f };
        float m_Aspect { 16.0f / 9.0f };
        float m_NearClip { 0.1f };
        float m_FarClip { 100.0f };

        float m_TotalPitch { 0.0f };
        float m_TotalYaw { 0.0f };

        // Cache the view matrix
        mutable Matrix m_ViewMatrix { };
        mutable bool m_ViewMatrixDirty { };

        // Cache the projection matrix
        mutable Matrix m_ProjMatrix { };
        mutable bool m_ProjMatrixDirty { };

        // Cache the frustum planes
        mutable Vector4 m_FrustumPlanes[6] { };

        void UpdateViewMatrixCache() const;
        void UpdateProjMatrixCache() const;
        void UpdateFrustumPlanesCache() const;

        public:

        Camera();

        Camera(const Vector3& position, const Vector3& rotation);

        Camera(const Vector3& position, const Vector3& rotation, float fov, float aspect,
            float nearClip, float farClip);

        void SetPosition(const Vector3& position);
        void SetRotation(const Vector3& rotation);

        void SetFOV(float fov);
        void SetAspect(float aspect);
        void SetNearClip(float nearClip);
        void SetFarClip(float farClip);

        void Update(Timer* timer);

        const Matrix& GetViewMatrix() const;
        const Matrix& GetProjMatrix() const;

        bool IsInFrustum(const Bounds& bounds) const;

        Vector3 GetPosition();
    };
}