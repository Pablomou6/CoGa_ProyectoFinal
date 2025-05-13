#version 330 core
in vec3 vertexColor;
in vec3 fragNormal;
in vec3 fragPos;
in vec2 TexCoord; // Añadido para texturas

out vec4 FragColor;

uniform vec3 ourColor;
uniform bool useVertexColor;
uniform bool useLighting;

// Textura
uniform sampler2D ourTexture;      // Añadido para texturas
uniform bool useTexture;           // Añadido para activar/desactivar textura

// Luz direccional
uniform vec3 lightDir;
uniform vec3 lightColor;

// Luz puntual encima de la esfera de iluminación
uniform bool luzIluminacionActiva;
uniform vec3 luzIluminacionPos;
uniform vec3 viewPos;

void main() {
    vec3 baseColor;
    if (useTexture) {
        baseColor = texture(ourTexture, TexCoord).rgb;
    } else {
        baseColor = useVertexColor ? vertexColor : ourColor;
    }
    vec3 normal = normalize(fragNormal);
    vec3 result = baseColor;

    if (useLighting) {
        // Luz direccional
        vec3 lightDirection = normalize(-lightDir);
        float diff = max(dot(normal, lightDirection), 0.0);
        vec3 diffuse = diff * lightColor;
        vec3 ambient = 0.2 * lightColor;
        result = (ambient + diffuse) * baseColor;
    }

    // Luz puntual encima de la esfera de iluminación (solo si está activa)
    if (luzIluminacionActiva) {
        vec3 lightPos = luzIluminacionPos;
        vec3 lightDirPuntual = normalize(lightPos - fragPos);
        float diffPuntual = max(dot(normal, lightDirPuntual), 0.0);

        // Atenuación por distancia
        float distance = length(lightPos - fragPos);
        float attenuation = 1.0 / (1.0 + 0.02 * distance + 0.01 * (distance * distance));

        vec3 colorLuz = vec3(1.0, 1.0, 0.8); // Luz cálida
        vec3 diffusePuntual = diffPuntual * colorLuz * attenuation * 2;
        vec3 ambientPuntual = 0.2 * colorLuz * attenuation * 2;

        // Suma la luz puntual a lo que ya había
        result += (ambientPuntual + diffusePuntual) * baseColor;
    }

    FragColor = vec4(result, 1.0);
}
