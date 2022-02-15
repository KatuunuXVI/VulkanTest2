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
layout(location = 3) in uint inIndex;

layout(location = 0) out vec4 fragColor;
layout(location = 1) out vec2 fragTexCoord;
layout(location = 2) out uint texIndex;




void main() {
    //gl_Position =  vec4(inPosition, 1.0);

    //((aPos.x+transX)/resWidRad)-1, 1-((aPos.y+transY)/resHeightRad), aPos.z/100
    //gl_Position = vec4((float(inPosition.x)/512)-1,float(inPosition.y)/512 - 1, inPosition.z, 1.0);
    //gl_Position = ubo.model * vec4(inPosition, 1.0);
    gl_Position =  ubo.model * vec4(inPosition.x/512.0 - 1,inPosition.y/512.0-1 ,inPosition.z, 1.0);
    //fragColor = vec4(float(inColor.x)/255.0,float(inColor.y)/255.0,float(inColor.z/255),float(inColor.w)/255);
    fragColor = vec4(float(inColor.x)/255.0,float(inColor.y)/255.0,float(inColor.z/255),inColor.w/255.0);
    //fragColor = vec4(1,1,1,1);
    fragTexCoord = vec2(inTexCoord.x/ubo.texW[1], inTexCoord.y/ubo.texH[0]) ;

}