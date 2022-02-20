#version 450

layout(location = 0) in vec4 fragColor;
layout(location = 1) in vec2 fragTexCoord;
layout(location = 2) flat in int fragIndex;

layout(location = 0) out vec4 outColor;

layout(binding=0) uniform UniformBufferObject {
    mat4 model;
    uint texW[32];
    uint texH[32];
} ubo;

#extension GL_EXT_scalar_block_layout: enable
layout(binding = 1) uniform sampler2D texSampler[2];


void main() {
    //vec4 dColor = vec4(1,1,1,1);
    outColor = fragColor;
    if(fragIndex >= 0) outColor = texture(texSampler[fragIndex], fragTexCoord)  * fragColor ;


}

