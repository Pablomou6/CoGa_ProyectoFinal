#version 330 core
in vec3 vertexColor; // Color recibido del shader de vértices

out vec4 FragColor;

uniform vec3 ourColor; // Color uniforme
uniform bool useVertexColor; // Interruptor para usar el color del VAO

void main() {
    if (useVertexColor) {
        FragColor = vec4(vertexColor, 1.0); // Usar el color del VAO
    } else {
        FragColor = vec4(ourColor, 1.0); // Usar el color uniforme
    }
}
