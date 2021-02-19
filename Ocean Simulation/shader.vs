#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoord;

out VS_OUT
{
	vec2 TexCoord;
}vs_out;

uniform mat4 view;
uniform mat4 projection;
uniform mat4 model;

void main()
{
	gl_Position = projection * view * model * vec4(aPos, 1.0f);
	vs_out.TexCoord = vec2(aTexCoord);
}