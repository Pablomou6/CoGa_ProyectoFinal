#version 330 core

// Entradas desde el vertex shader
in vec3 vertexColor;  // Color por v�rtice
in vec3 fragNormal;   // Normal del fragmento, transformada
in vec3 fragPos;      // Posici�n del fragmento 
in vec2 TexCoord;     // Coordenadas de textura

out vec4 FragColor;   // Color final que se enviar� al framebuffer

// Uniformes generales
uniform vec3 ourColor;        // Color general si no se usa color por v�rtice
uniform bool useVertexColor;  // Indica si se usa color por v�rtice
uniform bool useLighting;     // Indica si se activa la iluminaci�n

// Uniformes de textura
uniform sampler2D ourTexture; // Textura 2D aplicada al objeto
uniform bool useTexture;      // Indica si se debe usar la textura

// Uniformes de luz direccional
uniform vec3 lightDir;        // Direcci�n de la luz (normalizada)
uniform vec3 lightColor;      // Color de la luz direccional

// Uniformes para luz puntual 
uniform bool luzIluminacionActiva; // Activa/desactiva la luz puntual
uniform vec3 luzIluminacionPos;    // Posici�n de la luz puntual

void main() {
    vec3 baseColor;

    // Determina el color base del fragmento
    if (useTexture) {
        baseColor = texture(ourTexture, TexCoord).rgb;
    } else {
        baseColor = useVertexColor ? vertexColor : ourColor;
    }

    // Asegura que la normal est� normalizada
    vec3 normal = normalize(fragNormal); 

    // Resultado inicial (sin iluminaci�n)
    vec3 result = baseColor; 

    // Si la iluminaci�n est� activada, se aplica la luz direccional
    if (useLighting) {
    // Direcci�n hacia la luz 
        vec3 lightDirection = normalize(-lightDir);
        // Intensidad difusa
        float diff = max(dot(normal, lightDirection), 0.0);
        // Componente difusa
        vec3 diffuse = diff * lightColor;
        // Componente ambiental (fija)
        vec3 ambient = 0.2 * lightColor;  
        // Iluminaci�n total aplicada al color base
        result = (ambient + diffuse) * baseColor; 
    }

    // Luz puntual encima de la esfera (si est� activada)
    if (luzIluminacionActiva) {
        vec3 lightPos = luzIluminacionPos;
        // Direcci�n desde el fragmento a la luz
        vec3 lightDirPuntual = normalize(lightPos - fragPos); 
        // Intensidad difusa puntual
        float diffPuntual = max(dot(normal, lightDirPuntual), 0.0); 

        // Atenuaci�n por distancia 
        float distance = length(lightPos - fragPos);
        float attenuation = 1.0 / (1.0 + 0.02 * distance + 0.01 * (distance * distance));
        
        // Color c�lido para la luz puntual
        vec3 colorLuz = vec3(1.0, 1.0, 0.8);
        // Componente difusa puntual
        vec3 diffusePuntual = diffPuntual * colorLuz * attenuation * 2; 
        // Componente ambiental puntual
        vec3 ambientPuntual = 0.2 * colorLuz * attenuation * 2;         

        // Se suma la luz puntual al resultado existente
        result += (ambientPuntual + diffusePuntual) * baseColor;
    }

    // Asigna el color final al fragmento
    FragColor = vec4(result, 1.0);
}
