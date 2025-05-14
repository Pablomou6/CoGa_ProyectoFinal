#version 330 core

// Entradas del vértice
layout(location = 0) in vec3 aPos;       // Posición del vértice (x, y, z)
layout(location = 1) in vec3 aColor;     // Color asociado al vértice (r, g, b)
layout(location = 2) in vec3 aNormal;    // Vector normal para iluminación (x, y, z)
layout(location = 3) in vec2 aTexCoord;  // Coordenadas de textura (u, v)

// Variables de salida hacia el fragment shader
out vec3 vertexColor;  // Color 
out vec3 fragNormal;   // Normal transformada 
out vec3 fragPos;      // Posición del vértice 
out vec2 TexCoord;     // Coordenadas de textura 

// Matrices de transformación pasadas como uniformes desde la aplicación
uniform mat4 model;      // Matriz de transformación
uniform mat4 view;       // Matriz de cámara 
uniform mat4 projection; // Matriz de proyección

void main() {
    // Calcula la posición del vértice 
    vec4 worldPosition = model * vec4(aPos, 1.0);
    fragPos = vec3(worldPosition); // Almacena la posición para usar en el fragment shader

    // Transforma la normal 
    fragNormal = mat3(transpose(inverse(model))) * aNormal;

    // Pasa el color del vértice directamente al fragment shader
    vertexColor = aColor;

    // Pasa las coordenadas de textura
    TexCoord = aTexCoord;

    // Calcula la posición final del vértice 
    gl_Position = projection * view * worldPosition;
}
