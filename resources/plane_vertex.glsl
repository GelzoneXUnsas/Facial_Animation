#version 330 core
layout(location = 0) in vec3 vertPos1;
layout(location = 1) in vec3 vertPos2;
layout(location = 2) in vec3 vertPos3;
layout(location = 3) in vec3 vertPos4;
layout(location = 4) in vec3 norPos;

uniform mat4 P;
uniform mat4 V;
uniform mat4 M;
uniform float interp_factor_1;
uniform float interp_factor_2;
uniform float interp_factor_3;
uniform float interp_factor_4;
out vec3 vertex_pos;
out vec3 vertex_normal;

void main()
{
    vec3 actualFace = vertPos1 * interp_factor_1 + vertPos2 * interp_factor_2 + vertPos3 * interp_factor_3 + vertPos4 * interp_factor_4;
	vec4 tpos =  M * vec4(actualFace, 1.0);
    vertex_normal = vec4(M * vec4(norPos,0.0)).xyz;
	vertex_pos = tpos.xyz;
	gl_Position = P * V * tpos;
}
