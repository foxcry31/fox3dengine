#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aColor;
layout (location = 2) in vec2 aTex;

out vec3 color;
out vec2 texCoord;

uniform mat4 camera;

void main()
{
   gl_Position = camera * vec4(aPos, 1.0);
   color = aColor;
   texCoord = aTex;
}