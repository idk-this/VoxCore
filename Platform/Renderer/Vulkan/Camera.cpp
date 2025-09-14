#include "Camera.h"

Camera::Camera(glm::vec3 position, float yaw, float pitch, float fov, float aspect, float nearPlane, float farPlane)
    : m_position(position), m_yaw(yaw), m_pitch(pitch), m_fov(fov), m_aspect(aspect), m_near(nearPlane), m_far(farPlane) {
    UpdateView();
    UpdateProjection();
}

void Camera::Move(const glm::vec3& offset) {
    // Перемещение вдоль локальных осей камеры
    glm::vec3 front = glm::normalize(glm::vec3(
        cos(glm::radians(m_yaw)) * cos(glm::radians(m_pitch)),
        sin(glm::radians(m_pitch)),
        sin(glm::radians(m_yaw)) * cos(glm::radians(m_pitch))));
    glm::vec3 right = glm::normalize(glm::cross(front, glm::vec3(0, 1, 0)));
    glm::vec3 up = glm::normalize(glm::cross(right, front));
    m_position += offset.x * right + offset.y * up + offset.z * front;
    UpdateView();
}

void Camera::Rotate(float yawOffset, float pitchOffset) {
    m_yaw += yawOffset;
    m_pitch += pitchOffset;
    if (m_pitch > 179.0f) m_pitch = 179.0f;
    if (m_pitch < -179.0f) m_pitch = -179.0f;
    UpdateView();
}

void Camera::UpdateView() {
    glm::vec3 front = glm::normalize(glm::vec3(
        cos(glm::radians(m_yaw)) * cos(glm::radians(m_pitch)),
        sin(glm::radians(m_pitch)),
        sin(glm::radians(m_yaw)) * cos(glm::radians(m_pitch))));
    m_view = glm::lookAt(m_position, m_position + front, glm::vec3(0, 1, 0));
}

void Camera::UpdateProjection() {
    m_projection = glm::perspective(glm::radians(m_fov), m_aspect, m_near, m_far);
    m_projection[1][1] *= -1; // Vulkan Y-flip
}

void Camera::SetAspect(float aspect) {
    m_aspect = aspect;
    UpdateProjection();
}

