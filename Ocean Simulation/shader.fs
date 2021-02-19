#version 330 core
out vec4 FragColor;

in GS_OUT
{
	vec2 TexCoord;
	vec3 fColor;
}gs_in;

uniform sampler2D texture1;
uniform float alpha;

void main()
{
	 //FragColor = vec4(gs_in.fColor, 1.0f);   
         FragColor = texture(texture1, gs_in.TexCoord)*alpha;
};