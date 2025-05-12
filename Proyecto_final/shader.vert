#version 330 core
layout(location = 0) in vec3 aPos;    // Posici�n del v�rtice
layout(location = 1) in vec3 aColor;  // Color del v�rtice

out vec3 vertexColor; // Pasar el color al shader de fragmentos

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main() {
    gl_Position = projection * view * model * vec4(aPos, 1.0);
    vertexColor = aColor; // Pasar el color al shader de fragmentos
}