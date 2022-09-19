#version 330 core

uniform vec4 geometry;
uniform vec4 color;
uniform float type;
uniform float thickness = 1.0;
uniform float rounded = 0.0;

out vec4 FragColor;

float roundedBoxSDF(vec2 CenterPosition, vec2 Size, float Radius) {
    return length(max(abs(CenterPosition)-Size+Radius,0.0))-Radius;
}

void main()
{
    float fade = 0.005;
    vec2 pos = gl_FragCoord.xy - geometry.xy - geometry.zw/2.0;
    vec4 outColor;
    float smoothed;

    if (type == 0.0) { // circle
        float r = min(geometry.z, geometry.w) / 2.0;
        float distance = max(length(pos), 0.0) - r;
        smoothed = 1.0 - smoothstep(0.0, fade * 2.0f, distance);
        // smoothed *= smoothstep(thickness + fade, thickness, distance);
        if (smoothed == 0.0) {
            discard;
        }

        outColor = vec4(color.rgb * smoothed, color.a);
    } else {
        float distance  = roundedBoxSDF(pos, geometry.zw/2.0, rounded);

        // Smooth the result (free antialiasing).
        smoothed = 1.0f - smoothstep(0.0f, fade * 2.0f, distance);
        if (smoothed == 0.0)
            discard;

        outColor = vec4(color.rgb * smoothed, color.a);
    }

    FragColor = outColor;
}
