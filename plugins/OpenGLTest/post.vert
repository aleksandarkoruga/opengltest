#version 430 core

layout(location = 0) in vec2 vPos;     // Input position from vertex data
layout(location = 1) uniform mat4 MVP;       // Model-View-Projection matrix


void main()
{
    gl_Position = MVP*vec4(vPos.xy, 0.0, 1.0); // Compute vertex position
    //gl_Position = MVP * vec4(vPos, 1.0); // Apply aspect ratio correction
}
