#version 450

layout(location = 0) in vec4 fragColor;

layout(location = 0) out vec4 outColor;

void main() { outColor = fragColor; }

//void main() { outColor = vec4(1,1,1,1); }
//void main() { outColor = uvec4(fragColor[0],fragColor[1],fragColor[2],fragColor[3]); }

//void main() { outColor = uvec4(0,0,0,0); }