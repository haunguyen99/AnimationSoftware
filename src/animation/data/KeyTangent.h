#pragma once

enum class TangentMode
{
    Auto,    // Catmull-Rom auto tangents
    Linear,  // Linear interpolation to next key
    Flat,    // Horizontal tangents (angle = 0)
    Stepped, // Hold value until next key
    Broken   // Independent in/out angles
};

struct KeyTangent
{
    TangentMode mode    = TangentMode::Auto;
    float inAngle       = 0.f;
    float inWeight      = 1.f / 3.f; // fraction of adjacent segment (0-1)
    float outAngle      = 0.f;
    float outWeight     = 1.f / 3.f;
};
