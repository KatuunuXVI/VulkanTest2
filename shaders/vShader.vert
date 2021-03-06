#version 450
#extension GL_EXT_scalar_block_layout: enable
layout(binding=0, std430) uniform UniformBufferObject {
    mat4 model;
    uint texW[32];
    uint texH[32];

} ubo;

layout(location = 0) in ivec3 inPosition;
layout(location = 1) in uvec4 inColor;
layout(location = 2) in vec2 inTexCoord;
layout(location = 3) in int inIndex;

layout(location = 0) out vec4 fragColor;
layout(location = 1) out vec2 fragTexCoord;
layout(location = 2) flat out int texIndex;




void main() {

    gl_Position =  ubo.model * vec4((inPosition.x/512.0 - 1),-(inPosition.y/512.0-1) ,inPosition.z, 1.0);
    fragColor = vec4(inColor.x/255.0,inColor.y/255.0,inColor.z/255.0,inColor.w/255.0);
    fragTexCoord = vec2(inTexCoord.x/ubo.texW[inIndex], inTexCoord.y/ubo.texH[inIndex]) ;
    texIndex = inIndex;
}