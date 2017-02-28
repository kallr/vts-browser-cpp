#version 440

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec2 inUvsInternal;
layout(location = 2) in vec2 inUvsExternal;

layout(location = 0) uniform mat4 uniMvp;

out vec2 varUvs;

void main()
{
	gl_Position = uniMvp * vec4(inPosition, 1.0);
	varUvs = inUvsInternal;
}

