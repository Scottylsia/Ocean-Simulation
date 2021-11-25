#version 330 core
out vec4 FragColor;

in GS_OUT
{
	vec2 TexCoord;
}gs_in;

uniform sampler2D texture1;
uniform sampler2D textureBlueCircle;
uniform mat4 model;
uniform float alpha = 1.0;

void main()
{   
	FragColor = mix(texture(textureBlueCircle, gs_in.TexCoord), texture(texture1, gs_in.TexCoord), (model[3][1] + 1.5) /3.0) * alpha;
};