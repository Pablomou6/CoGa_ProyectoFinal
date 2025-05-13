#version 330 core
layout(location = 0) in vec3 aPos;    // Posici�n del v�rtice
layout(location = 1) in vec3 aColor;  // Color del v�rtice
layout(location = 2) in vec3 aNormal; // Normal del v�rtice (nuevo)
layout(location = 3) in vec2 aTexCoord; // Coordenadas de textura (nuevo)

// Salidas
out vec3 vertexColor;  // Color al fragment shader (lo original)
out vec3 fragNormal;   // Normal transformada para iluminaci�n
out vec3 fragPos;      // Posici�n del v�rtice en mundo
out vec2 TexCoord;     // Coordenadas de textura (nuevo)

// Uniformes
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main() {
    vec4 worldPosition = model * vec4(aPos, 1.0);
    fragPos = vec3(worldPosition); // Posici�n en mundo

    // Normal transformada correctamente
    fragNormal = mat3(transpose(inverse(model))) * aNormal;

    vertexColor = aColor; // Lo original, sin cambios
    TexCoord = aTexCoord; // Pasar coordenadas de textura al fragment shader
    gl_Position = projection * view * worldPosition;
}