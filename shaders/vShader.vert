#version 450

layout(binding=0) uniform UniformBufferObject {
    mat4 model;
} ubo;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in uvec4 inColor;
layout(location = 2) in vec2 inTexCoord;

layout(location = 0) out vec4 fragColor;
layout(location = 1) out vec2 fragTexCoord;




void main() {
    //gl_Position =  vec4(inPosition, 1.0);
    mat4 m4 = mat4(
    vec4(1,0,0,0),
    vec4(0,1,0,0),
    vec4(0,0,1,0),
    vec4(0,0,0,1)
    );
    //((aPos.x+transX)/resWidRad)-1, 1-((aPos.y+transY)/resHeightRad), aPos.z/100
    gl_Position = vec4((float(inPosition.x)/512)-1,float(inPosition.y)/512 - 1, inPosition.z, 1.0);
    gl_Position = ubo.model * vec4(inPosition, 1.0);
    fragColor = vec4(float(inColor.x)/255,inColor.y/255,inColor.z/255,inColor.w/255);
    fragTexCoord = inTexCoord;
}