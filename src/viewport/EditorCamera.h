#pragma once

#include <QMatrix4x4>
#include <QVector3D>

class EditorCamera
{
public:
    enum class ProjectionMode
    {
        Perspective,
        Orthographic
    };

    enum class ViewPreset
    {
        Perspective,
        Front,
        Back,
        Left,
        Right,
        Top,
        Bottom
    };

    EditorCamera();

    void reset();
    void frameScene(float radius = 8.0f);
    void frameBounds(const QVector3D& center, float radius);
    void orbit(float deltaYawDegrees, float deltaPitchDegrees);
    void pan(float deltaX, float deltaY);
    void zoom(float steps);
    void setViewportSize(int width, int height);
    void setViewPreset(ViewPreset preset);

    float zoomSensitivity() const;
    void setZoomSensitivity(float value);
    QVector3D target() const;
    QVector3D forwardDirection() const;
    float worldUnitsPerPixelAt(const QVector3D& worldPosition) const;
    ProjectionMode projectionMode() const;
    ViewPreset viewPreset() const;

    QMatrix4x4 viewMatrix() const;
    QMatrix4x4 projectionMatrix() const;

private:
    QVector3D forwardVector() const;
    QVector3D rightVector() const;
    QVector3D upVector() const;
    void clampPitch();
    QVector3D presetForwardVector(ViewPreset preset) const;
    QVector3D presetUpVector(ViewPreset preset) const;

    QVector3D target_;
    float distance_ = 8.0f;
    float orthoScale_ = 6.0f;
    float yawDegrees_ = 45.0f;
    float pitchDegrees_ = -30.0f;
    float fovDegrees_ = 45.0f;
    float nearPlane_ = 0.1f;
    float farPlane_ = 200.0f;
    float zoomSensitivity_ = 0.75f;
    int viewportWidth_ = 1280;
    int viewportHeight_ = 720;
    ProjectionMode projectionMode_ = ProjectionMode::Perspective;
    ViewPreset viewPreset_ = ViewPreset::Perspective;
};
