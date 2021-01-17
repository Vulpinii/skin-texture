#version 410 core
layout(location = 0) in vec3 aPos;
uniform mat4 lightSpaceMatrix;
uniform mat4 model;
uniform mat4 modelGlobal;

void main(){
	gl_Position =  lightSpaceMatrix * modelGlobal * model * vec4(aPos, 1.0);
}

