#version 450

layout(location = 0) in vec4 fragColor;
layout(location = 1) in vec2 fragTexCoord;
layout(location = 2) flat  in uint fragIndex;

layout(location = 0) out vec4 outColor;

layout(binding=0) uniform UniformBufferObject {
    mat4 model;
    uint texW[32];
    uint texH[32];
} ubo;

#extension GL_EXT_scalar_block_layout: enable
layout(binding = 1) uniform sampler2D texSampler[2];

//layout(binding = 2) uniform texWDesc { uint texW[32];};
//layout(binding = 3) uniform texHDesc { uint texH[32];};
//layout(binding = 2) uniform sampler2D texSampler2;


void main() {
    //vec4 dColor = vec4(1,1,1,1);
    outColor = texture(texSampler[fragIndex], fragTexCoord)  * fragColor ;
    //outColor =  texture(texSampler, fragTexCoord) ;
    //outColor = fragColor;
}

//void main() { outColor = vec4(1,1,1,1); }
//void main() { outColor = uvec4(fragColor[0],fragColor[1],fragColor[2],fragColor[3]); }

//void main() { outColor = uvec4(0,0,0,0); }