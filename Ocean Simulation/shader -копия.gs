#version 330
 
layout (points) in;
layout (triangle_strip) out;
layout (max_vertices = 4) out;  

out vec2 TexCoord;

uniform mat4 view;
uniform mat4 projection;
uniform vec3 cameraPos;

void main()
{
    vec3 cameraRight = vec3(view[0][0], view[1][0], view[2][0]);
    vec3 cameraUp = vec3(view[0][1], view[1][1], view[2][1]);
    vec3 pos = gl_in[0].gl_Position.xyz;
    vec3 toCamera = normalize(cameraPos - pos);

    pos -= (cameraRight * 0.5);
    gl_Position = projection * vec4(pos, 1.0);
    TexCoord = vec2(0.0, 0.0);
    EmitVertex();

    pos.y += 1.0;
    gl_Position = projection * vec4(pos, 1.0);
    TexCoord = vec2(0.0, 1.0);
    EmitVertex();

    pos.y -= 1.0;
    pos += cameraRight;
    gl_Position = projection * vec4(pos, 1.0);
    TexCoord = vec2(1.0, 0.0);
    EmitVertex();

    pos.y += 1.0;
    pos += cameraRight;
    gl_Position = projection * vec4(pos, 1.0);
    TexCoord = vec2(1.0, 1.0);
    EmitVertex();

    EndPrimitive();
};