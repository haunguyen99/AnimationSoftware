#include "viewport/EditorCamera.h"

#include <QtMath>

namespace
{
constexpr float kMinPitch = -89.0f;
constexpr float kMaxPitch = 89.0f;
constexpr float kMinDistance = 1.0f;
constexpr float kMaxDistance = 100.0f;
}

EditorCamera::EditorCamera()
{
    reset();
}

void EditorCamera::reset()
{
    target_ = QVector3D(0.0f, 0.0f, 0.0f);
    distance_ = 8.0f;
    yawDegrees_ = 45.0f;
    pitchDegrees_ = -30.0f;
    fovDegrees_ = 45.0f;
    nearPlane_ = 0.1f;
    farPlane_ = 200.0f;
}

void EditorCamera::frameScene(float radius)
{
    frameBounds(QVector3D(0.0f, 0.0f, 0.0f), radius);
}

void EditorCamera::frameBounds(const QVector3D& center, float radius)
{
    target_ = center;
    distance_ = qBound(kMinDistance, radius * 1.5f, kMaxDistance);
}

void EditorCamera::orbit(float deltaYawDegrees, float deltaPitchDegrees)
{
    yawDegrees_ += deltaYawDegrees;
    pitchDegrees_ += deltaPitchDegrees;
    clampPitch();
}

void EditorCamera::pan(float deltaX, float deltaY)
{
    const float panScale = qMax(distance_, 1.0f) * 0.0015f;
    target_ -= rightVector() * (deltaX * panScale);
    target_ += upVector() * (deltaY * panScale);
}

void EditorCamera::zoom(float steps)
{
    distance_ = qBound(kMinDistance, distance_ - (steps * zoomSensitivity_), kMaxDistance);
}

void EditorCamera::setViewportSize(int width, int height)
{
    viewportWidth_ = qMax(1, width);
    viewportHeight_ = qMax(1, height);
}

float EditorCamera::zoomSensitivity() const
{
    return zoomSensitivity_;
}

void EditorCamera::setZoomSensitivity(float value)
{
    zoomSensitivity_ = qMax(0.05f, value);
}

QVector3D EditorCamera::target() const
{
    return target_;
}

QMatrix4x4 EditorCamera::viewMatrix() const
{
    QMatrix4x4 view;
    const QVector3D eye = target_ - (forwardVector() * distance_);
    view.lookAt(eye, target_, QVector3D(0.0f, 1.0f, 0.0f));
    return view;
}

QMatrix4x4 EditorCamera::projectionMatrix() const
{
    QMatrix4x4 projection;
    projection.perspective(fovDegrees_, static_cast<float>(viewportWidth_) / static_cast<float>(viewportHeight_),
        nearPlane_, farPlane_);
    return projection;
}

QVector3D EditorCamera::forwardVector() const
{
    const float yawRadians = qDegreesToRadians(yawDegrees_);
    const float pitchRadians = qDegreesToRadians(pitchDegrees_);

    return QVector3D(
        qCos(pitchRadians) * qSin(yawRadians),
        qSin(pitchRadians),
        qCos(pitchRadians) * qCos(yawRadians))
        .normalized();
}

QVector3D EditorCamera::rightVector() const
{
    return QVector3D::crossProduct(forwardVector(), QVector3D(0.0f, 1.0f, 0.0f)).normalized();
}

QVector3D EditorCamera::upVector() const
{
    return QVector3D::crossProduct(rightVector(), forwardVector()).normalized();
}

void EditorCamera::clampPitch()
{
    pitchDegrees_ = qBound(kMinPitch, pitchDegrees_, kMaxPitch);
}
