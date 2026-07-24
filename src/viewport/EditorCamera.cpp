#include "viewport/EditorCamera.h"

#include <QtMath>

namespace
{
constexpr float kMinPitch = -89.0f;
constexpr float kMaxPitch = 89.0f;
constexpr float kMinDistance = 1.0f;
constexpr float kMaxDistance = 100.0f;
constexpr float kMinOrthoScale = 0.5f;
constexpr float kMaxOrthoScale = 250.0f;
}

EditorCamera::EditorCamera()
{
    reset();
}

void EditorCamera::reset()
{
    target_ = QVector3D(0.0f, 0.0f, 0.0f);
    distance_ = 8.0f;
    orthoScale_ = 6.0f;
    if (viewPreset_ == ViewPreset::Perspective) {
        yawDegrees_ = 45.0f;
        pitchDegrees_ = -30.0f;
        projectionMode_ = ProjectionMode::Perspective;
    } else {
        projectionMode_ = ProjectionMode::Orthographic;
    }
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
    orthoScale_ = qBound(kMinOrthoScale, radius * 1.25f, kMaxOrthoScale);
}

void EditorCamera::orbit(float deltaYawDegrees, float deltaPitchDegrees)
{
    if (projectionMode_ == ProjectionMode::Orthographic) {
        return;
    }

    yawDegrees_ += deltaYawDegrees;
    pitchDegrees_ += deltaPitchDegrees;
    clampPitch();
}

void EditorCamera::pan(float deltaX, float deltaY)
{
    const float panScale = projectionMode_ == ProjectionMode::Orthographic
        ? worldUnitsPerPixelAt(target_)
        : qMax(distance_, 1.0f) * 0.0015f;
    target_ -= rightVector() * (deltaX * panScale);
    target_ += upVector() * (deltaY * panScale);
}

void EditorCamera::zoom(float steps)
{
    if (projectionMode_ == ProjectionMode::Orthographic) {
        orthoScale_ = qBound(kMinOrthoScale, orthoScale_ - (steps * zoomSensitivity_), kMaxOrthoScale);
        return;
    }

    distance_ = qBound(kMinDistance, distance_ - (steps * zoomSensitivity_), kMaxDistance);
}

void EditorCamera::setViewportSize(int width, int height)
{
    viewportWidth_ = qMax(1, width);
    viewportHeight_ = qMax(1, height);
}

void EditorCamera::setViewPreset(ViewPreset preset)
{
    viewPreset_ = preset;
    projectionMode_ = preset == ViewPreset::Perspective
        ? ProjectionMode::Perspective
        : ProjectionMode::Orthographic;

    if (preset == ViewPreset::Perspective) {
        yawDegrees_ = 45.0f;
        pitchDegrees_ = -30.0f;
    }
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

QVector3D EditorCamera::forwardDirection() const
{
    return forwardVector();
}

float EditorCamera::worldUnitsPerPixelAt(const QVector3D& worldPosition) const
{
    if (projectionMode_ == ProjectionMode::Orthographic) {
        return (orthoScale_ * 2.0f) / static_cast<float>(qMax(1, viewportHeight_));
    }

    const QVector3D eye = target_ - (forwardVector() * distance_);
    const float distanceToPoint = qMax(0.001f, (worldPosition - eye).length());
    const float worldHeight = 2.0f * distanceToPoint * qTan(qDegreesToRadians(fovDegrees_ * 0.5f));
    return worldHeight / static_cast<float>(qMax(1, viewportHeight_));
}

EditorCamera::ProjectionMode EditorCamera::projectionMode() const
{
    return projectionMode_;
}

EditorCamera::ViewPreset EditorCamera::viewPreset() const
{
    return viewPreset_;
}

QMatrix4x4 EditorCamera::viewMatrix() const
{
    QMatrix4x4 view;
    const QVector3D eye = target_ - (forwardVector() * distance_);
    view.lookAt(eye, target_, upVector());
    return view;
}

QMatrix4x4 EditorCamera::projectionMatrix() const
{
    QMatrix4x4 projection;
    const float aspectRatio = static_cast<float>(viewportWidth_) / static_cast<float>(viewportHeight_);
    if (projectionMode_ == ProjectionMode::Orthographic) {
        projection.ortho(-orthoScale_ * aspectRatio, orthoScale_ * aspectRatio, -orthoScale_, orthoScale_, nearPlane_, farPlane_);
    } else {
        projection.perspective(fovDegrees_, aspectRatio, nearPlane_, farPlane_);
    }
    return projection;
}

QVector3D EditorCamera::forwardVector() const
{
    if (viewPreset_ != ViewPreset::Perspective) {
        return presetForwardVector(viewPreset_);
    }

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
    if (viewPreset_ != ViewPreset::Perspective) {
        return QVector3D::crossProduct(forwardVector(), upVector()).normalized();
    }

    return QVector3D::crossProduct(forwardVector(), QVector3D(0.0f, 1.0f, 0.0f)).normalized();
}

QVector3D EditorCamera::upVector() const
{
    if (viewPreset_ != ViewPreset::Perspective) {
        return presetUpVector(viewPreset_);
    }

    return QVector3D::crossProduct(rightVector(), forwardVector()).normalized();
}

void EditorCamera::clampPitch()
{
    pitchDegrees_ = qBound(kMinPitch, pitchDegrees_, kMaxPitch);
}

QVector3D EditorCamera::presetForwardVector(ViewPreset preset) const
{
    switch (preset) {
    case ViewPreset::Front:
        return QVector3D(0.0f, 0.0f, -1.0f);
    case ViewPreset::Back:
        return QVector3D(0.0f, 0.0f, 1.0f);
    case ViewPreset::Left:
        return QVector3D(1.0f, 0.0f, 0.0f);
    case ViewPreset::Right:
        return QVector3D(-1.0f, 0.0f, 0.0f);
    case ViewPreset::Top:
        return QVector3D(0.0f, -1.0f, 0.0f);
    case ViewPreset::Bottom:
        return QVector3D(0.0f, 1.0f, 0.0f);
    case ViewPreset::Perspective:
        break;
    }

    return QVector3D(0.0f, 0.0f, -1.0f);
}

QVector3D EditorCamera::presetUpVector(ViewPreset preset) const
{
    switch (preset) {
    case ViewPreset::Top:
        return QVector3D(0.0f, 0.0f, -1.0f);
    case ViewPreset::Bottom:
        return QVector3D(0.0f, 0.0f, 1.0f);
    case ViewPreset::Front:
    case ViewPreset::Back:
    case ViewPreset::Left:
    case ViewPreset::Right:
    case ViewPreset::Perspective:
        break;
    }

    return QVector3D(0.0f, 1.0f, 0.0f);
}
