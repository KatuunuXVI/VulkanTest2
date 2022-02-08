#version 450

layout(binding=0) uniform UniformBufferObject {
    mat4 model;
    mat4 view;
    mat4 proj;
} ubo;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in uvec4 inColor;
layout(location = 0) out vec4 fragColor;




void main() {
    gl_Position =  vec4(inPosition, 1.0);
    fragColor = vec4(float(inColor.x)/255,inColor.y/255,inColor.z/255,inColor.w/255);
}