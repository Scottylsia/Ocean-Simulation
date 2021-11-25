#version 330
 
layout (points) in;
layout (triangle_strip) out;
layout (max_vertices = 6) out;  

out GS_OUT {
	vec2 TexCoord;
}gs_out;

uniform float alpha = 1;
uniform float screenRatio;

void main ()
{
    vec3	right = vec3(0.1,0.0,0.0) * alpha * 2;  
    vec3    up = vec3(0.0, 0.1, 0.0) * screenRatio * alpha * 2;
    vec3	pos = gl_in [0].gl_Position.xyz;  
    float	w = gl_in [0].gl_Position.w;

    gl_Position = vec4 ( pos - up - right, w );	
gs_out.TexCoord = vec2(0.0f, 0.0f);
    EmitVertex ();
    gl_Position = vec4 ( pos - up + right, w );
gs_out.TexCoord = vec2(0.0,  1.0);
    EmitVertex ();
    gl_Position = vec4 ( pos + up + right, w );
gs_out.TexCoord = vec2(1.0, 1.0);
    EmitVertex   ();
    EndPrimitive ();				// 1st triangle
    gl_Position = vec4 ( pos + up + right, w );
gs_out.TexCoord = vec2(1.0, 1.0f);
    EmitVertex   ();
    gl_Position = vec4 ( pos + up - right, w );
gs_out.TexCoord = vec2(1.0f, 0.0f);
    EmitVertex ();
    gl_Position = vec4 ( pos - up - right, w );
gs_out.TexCoord = vec2(0.0f, 0.0f);
    EmitVertex   ();
    EndPrimitive ();				// 2nd triangle
}
