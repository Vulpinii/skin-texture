#version 410 core

// Input vertex data, different for all executions of this shader.
layout(location = 0) in vec3 vertexPosition_modelspace;
layout(location = 1) in vec2 vertexUV;
layout(location = 2) in vec3 vertexNormal_modelspace;

// Values that stay constant for the whole mesh.
uniform mat4 projection;
uniform mat4 modelGlobal;
uniform mat4 view;
uniform mat4 model;

out vec3 Normal;
out vec3 FragPos;

void main(){
	gl_Position =  projection * view * model * vec4(vertexPosition_modelspace,1);
	Normal = vertexNormal_modelspace;
	FragPos = (model * vec4(vertexPosition_modelspace,1)).xyz;
}

