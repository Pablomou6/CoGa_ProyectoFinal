#version 330 core

// Entradas del v�rtice
layout(location = 0) in vec3 aPos;       // Posici�n del v�rtice (x, y, z)
layout(location = 1) in vec3 aColor;     // Color asociado al v�rtice (r, g, b)
layout(location = 2) in vec3 aNormal;    // Vector normal para iluminaci�n (x, y, z)
layout(location = 3) in vec2 aTexCoord;  // Coordenadas de textura (u, v)

// Variables de salida hacia el fragment shader
out vec3 vertexColor;  // Color 
out vec3 fragNormal;   // Normal transformada 
out vec3 fragPos;      // Posici�n del v�rtice 
out vec2 TexCoord;     // Coordenadas de textura 

// Matrices de transformaci�n pasadas como uniformes desde la aplicaci�n
uniform mat4 model;      // Matriz de transformaci�n
uniform mat4 view;       // Matriz de c�mara 
uniform mat4 projection; // Matriz de proyecci�n

void main() {
    // Calcula la posici�n del v�rtice 
    vec4 worldPosition = model * vec4(aPos, 1.0);
    fragPos = vec3(worldPosition); // Almacena la posici�n para usar en el fragment shader

    // Transforma la normal 
    fragNormal = mat3(transpose(inverse(model))) * aNormal;

    // Pasa el color del v�rtice directamente al fragment shader
    vertexColor = aColor;

    // Pasa las coordenadas de textura
    TexCoord = aTexCoord;

    // Calcula la posici�n final del v�rtice 
    gl_Position = projection * view * worldPosition;
}
