#include <iostream>
#include <stdio.h>

#include "glad.h"  
#include <glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "lecturaShader_0_9.h"
#include "esfera.h"
#include <vector>
#include <random>
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

//Declaramos constantes
#define TRIANGULOS GL_TRIANGLES
#define sphereVertexCount 1080
int SCR_WIDTH = 800;	//Declara el ancho de la ventana
int SCR_HEIGHT = 800;	//Declara la altura de la ventana

//Declaramos los shaders
extern GLuint setShaders(const char* nVertx, const char* nFrag);
GLuint shaderProgram; 


//////Declaramos variables globales para la c�mara y su movimiento con el rat�n//////

// Posici�n inicial de la c�mara en el espacio 3D  
glm::vec3 camPos = glm::vec3(.0f, 5.0f, 33.0f);  
// Direcci�n hacia la que apunta la c�mara (normalizada)  
glm::vec3 camFront = glm::normalize(glm::vec3(0.0f, 0.0f, -1.0f));  
// Vector que define la direcci�n "arriba" de la c�mara  
glm::vec3 camUp = glm::vec3(0.0f, 1.0f, 0.0f);  
// Direcci�n horizontal de la c�mara, ignorando el componente vertical  
glm::vec3 horizontalFront = glm::normalize(glm::vec3(camFront.x, 0.0f, camFront.z));  
// Campo de visi�n de la c�mara en grados  
float fovCamara = 45.0f;  
// Velocidad de movimiento de la c�mara  
float velCamara = 0.1f;  
// �ngulo de rotaci�n horizontal de la c�mara (en grados)  
float angRotHoriz = -90.0f;  
// �ngulo de rotaci�n vertical de la c�mara (en grados)  
float angRotVert = 0.0f;  
// Indica si es la primera vez que se mueve el rat�n (para evitar saltos al generar la c�mara)  
bool primerMovCam = true;  
// �ltima posici�n X del rat�n en la ventana  
float ultimoX = SCR_WIDTH / 2.0;  
// �ltima posici�n Y del rat�n en la ventana  
float ultimoY = SCR_HEIGHT / 2.0;  
// Sensibilidad del movimiento del rat�n  
float sensibilidad = 0.05f;  
// Indica si la ventana est� en modo pantalla completa  
bool esPantCompleta = false;  
// Indica si "p" fue presionada (para alternar pantalla completa)  
bool pPulsado = false;  
// Coordenadas y dimensiones de la ventana en modo ventana (no pantalla completa)  
int ventanaX, ventanaY, anchoVentana, altoVentana;

//////Variables para ej maneja de la c�mara del cubo y recuperaci�n de la c�mara normal//////
// Indica si se la c�mara est� observando al cubo
int camaraCuboActiva = 0;
// Posici�n (�ltima) de la c�mara en modo libre  
glm::vec3 camPosLibre = camPos;
// Direcci�n hacia donde apuntaba (la �ltima vez) la c�mara en modo libre  
glm::vec3 camFrontLibre = camFront; 
// Vector que define la direcci�n "arriba" de la c�mara en modo libre  
glm::vec3 camUpLibre = camUp; 
// �ngulo de rotaci�n horizontal (�ltimo) de la c�mara en modo libre  
float angRotHorizLibre = angRotHoriz; 
// �ngulo de rotaci�n vertical (�ltimo) de la c�mara en modo libre  
float angRotVertLibre = angRotVert; 
// Indica si la c�mara est� bloqueada (fija, en el cubo) o en modo libre
bool camaraBloqueada = false; 

//////Variables para controlar la iluminaci�n//////
// Controla si la iluminaci�n est� activa o no
bool luzIluminacionActiva = false;
// Controla el pulsado de la tecla 7
bool tecla7Presionada = false;

// Declaramos la clase con la que crearemos los diferentes componentes estructurales
class Estructura {
public:
	// Atributos p�blicos de los objetos
	float px, py, pz;        // posici�n inicial
	float sx, sy, sz;        // escalado
	unsigned int VAO;        // Vertex Array Object
	int texture;             // textura del objeto
	glm::mat4 transform;     // matriz de transformaci�n
	glm::vec3 color;         // color del objeto
	glm::mat4 posicion; // matriz de posici�n

	// Constructor por defecto
	Estructura() {
		px = 0;
		py = 0;
		pz = 0;
		sx = 1;
		sy = 1;
		sz = 1;
		VAO = 0;
		texture = 0;
		transform = glm::mat4(1.0f);
		color = glm::vec3(1.0f);
	}

	// Constructor con par�metros. En caso de no recibir textura, se le asigna 0, pero permite cargarla
	Estructura(float px, float py, float pz, float sx, float sy, float sz, unsigned int VAO, glm::vec3 color = glm::vec3(1.0f),
		GLuint texture = 0) {
		this->px = px;
		this->py = py;
		this->pz = pz;
		this->sx = sx;
		this->sy = sy;
		this->sz = sz;
		this->VAO = VAO;
		this->color = color;
		this->texture = texture;
		this->transform = glm::mat4(1.0f);
		this->posicion = glm::mat4(1.0f);
	}

	// M�todo que devuelve la matriz de transformaci�n
	glm::mat4 getTransform() const {
		return transform;
	}

	// M�todo para dibujar el objeto (Solo para emplear una vez se hacen las transformaciones, ya que transform se va modificando)
	void dibujarObjeto(unsigned int transformLoc, glm::mat4 transform, unsigned int colorLoc, GLenum primitiva, unsigned int numElem) {
		// Configurar la matriz de transformaci�n y el color
		glUniformMatrix4fv(transformLoc, 1, GL_FALSE, glm::value_ptr(transform));
		glUniform3fv(colorLoc, 1, glm::value_ptr(color));

		// Configurar las texturas
		if (texture != 0) {
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, texture);
			glUniform1i(glGetUniformLocation(shaderProgram, "ourTexture"), 0);
		}

		// Dibujar el objeto
		glBindVertexArray(VAO);
		glDrawArrays(primitiva, 0, numElem);
		glBindVertexArray(0);

		// Restaurar configuraci�n de texturas
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, 0);
	}
};

//////CREACI�N DEL ENTORNO///////
// Declaramos los suelos de forma global
Estructura SueloPasillo(0, 0, 0, 33.0f, 1.0f, 100.0f, 0, glm::vec3(1, 1, 1));

Estructura SueloIntro(0, 0, 0, 33.3f, 1.0f, 33.3f, 0, glm::vec3(.1f, .1f, .1f));
Estructura SueloModelado(0, 0, 0, 33.3f, 1.0f, 33.3f, 0, glm::vec3(.2f, .2f, .2f));
Estructura SueloTransformaciones(0, 0, 0, 33.3f, 1.0f, 33.3f, 0, glm::vec3(.3f, .3f, .3f));

Estructura SueloCamara(0, 0, 0, 33.3f, 1.0f, 33.3f, 0, glm::vec3(.4f, .4f, .4f));

Estructura SueloIluminacion(0, 0, 0, 33.3f, 1.0f, 33.3f, 0, glm::vec3(.5f, .5f, .5f));
Estructura SueloTexturas(0, 0, 0, 33.3f, 1.0f, 33.3f, 0, glm::vec3(.6f, .6f, .6f));
Estructura SueloColisiones(0, 0, 0, 33.3f, 1.0f, 33.3f, 0, glm::vec3(.7f, .7f, .7f));


// Declaramos las paredes de forma global
Estructura ParedXYTotal(0, 0, 0, 99.3f, 12.0f, 1.0f, 0, glm::vec3(.9f, .9f, .9f));
Estructura ParedXY(.0f, 0.0f, .0f, 33.3f, 12.0f, 1.0f, 0, glm::vec3(.9f, .9f, .9f));
Estructura ParedYZ(.0f, 0.0f, .0f, 1.0f, 12.0f, 33.3f, 0, glm::vec3(.9f, .9f, .9f));

// Declaramos un techo total y otro para la habitaci�n de la iluminaci�n (sobresale)
Estructura TechoTotal(0, 0, 0, 100.0f, 1.0f, 100.0f, 0, glm::vec3(.9f, .9f, .9f));
Estructura Techo(0, 0, 0, 33.3f, 1.0f, 33.3f, 0, glm::vec3(.9f, .9f, .9f));

////TRIANGULO PARA LA HABITACI�N INTRO//////
Estructura TrianguloIntro(0.0f, 0.0f, 0.0f, 7.0f, 7.0f, 7.0f, 0, glm::vec3(1.0f, 1.0f, 1.0f));

////ESFERA PARA LA HABITACI�N DEL MODELADO////
Estructura  EsferaModelado(0, 0, 0, 4.0, 4.0, 4.0, 0, glm::vec3(0.0, 0.0, 0.0));


//////CUBO PARA LA HABITACI�N DE LAS TRANSFORMACIONES//////
Estructura CuboTransformaciones(0, 0, 0, 2.0, 2.0, 2.0, 0, glm::vec3(1.0, 1.0, 1.0));

/////CUBO PARA LA HABITACI�N DE LA C�MARA//////
Estructura CuboCamara(0, 0, 0, 2.0, 2.0, 2.0, 0, glm::vec3(1.0, 1.0, 1.0));

/////ESFERA PARA LA HABITACI�N DE LA ILUMINACI�N//////
Estructura  EsferaIluminacion(0, 0, 0, 1, 1, 1, 0, glm::vec3(.7f, 0.7f, 0.7f));
Estructura CuboIluminacion(0, 0, 0, .75f, 1.50f, .75f, 0, glm::vec3(.2f, .2f, .2f));

/////HABITACI�N DE LAS TEXTURAS//////
//La habitaci�n de las texturas estar� vac�a, dado que las texturas ser�n aplicadas en todos las paredes y suelos

/////HABITACI�N DE COLISIONES////////
Estructura CuboColisiones(0, 0, 0, 3.0f, 8.0f, 3.0f, 0, glm::vec3(0, 0, 0));

// Funci�n encargada de cargar la textura
int myCargaTexturas(const char* nombre) {
	GLuint textura;

	// Cargamos la textura y la asignamos a un ID
	glGenTextures(1, &textura);
	glBindTexture(GL_TEXTURE_2D, textura);

	// Configuramos los par�metros de la textura
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	// Configuramos los par�metros de la textura
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	// Cargamos la imagen y la asignamos a la textura
	int width, height, nrChannels;
	unsigned char* data = stbi_load(nombre, &width, &height, &nrChannels, 0);

	// Comprobamos si la imagen se ha cargado correctamente
	if (data) {
		// Cargamos la textura en la GPU
		if (nrChannels == 3) {
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
		}
		// Comprobamos si la imagen tiene un canal alfa
		else if (nrChannels == 4) {
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
		}
	}
	else {
		std::cout << "Failed to load texture" << std::endl;
	}

	// Comprobamos si la textura se ha cargado correctamente
	stbi_image_free(data);
	stbi_set_flip_vertically_on_load(1);

	return textura;
}

// Funci�n para preparar el VAO del cuadrado en el plano XZ (suelo)
void CuadradoXZ(unsigned int* VAOSuelo) {
	unsigned int VBO;

	float vertices[] = {
		//    Posici�n         Color           Normal                TexCoords
		-0.5f, 0.0f,  0.5f,    0.0f, 1.0f, 0.0f,   0.0f, 1.0f, 0.0f,    0.0f, 0.0f,
		 0.5f, 0.0f,  0.5f,    0.0f, 1.0f, 0.0f,   0.0f, 1.0f, 0.0f,    1.0f, 0.0f,
		 0.5f, 0.0f, -0.5f,    0.0f, 1.0f, 0.0f,   0.0f, 1.0f, 0.0f,    1.0f, 1.0f,

		-0.5f, 0.0f,  0.5f,    0.0f, 1.0f, 0.0f,   0.0f, 1.0f, 0.0f,    0.0f, 0.0f,
		 0.5f, 0.0f, -0.5f,    0.0f, 1.0f, 0.0f,   0.0f, 1.0f, 0.0f,    1.0f, 1.0f,
		-0.5f, 0.0f, -0.5f,    0.0f, 1.0f, 0.0f,   0.0f, 1.0f, 0.0f,    0.0f, 1.0f
	};

	glGenVertexArrays(1, VAOSuelo);
	glGenBuffers(1, &VBO);
	glBindVertexArray(*VAOSuelo);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	// Posici�n
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	// Color
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);

	// Normal
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void*)(6 * sizeof(float)));
	glEnableVertexAttribArray(2);

	// Coordenadas de textura
	glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void*)(9 * sizeof(float)));
	glEnableVertexAttribArray(3);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

// Funci�n para preparar el VAO del cuadrado en el plano XZ (Techo)
void CuadradoXZTecho(unsigned int* VAOSuelo) {
	unsigned int VBO;

	float vertices[] = {
		//    Posici�n         Color           Normal                TexCoords
		-0.5f, 0.0f,  0.5f,    0.0f, 1.0f, 0.0f,   0.0f, -1.0f, 0.0f,    0.0f, 0.0f,
		-0.5f, 0.0f, -0.5f,    0.0f, 1.0f, 0.0f,   0.0f, -1.0f, 0.0f,    0.0f, 1.0f,
		 0.5f, 0.0f, -0.5f,    0.0f, 1.0f, 0.0f,   0.0f, -1.0f, 0.0f,    1.0f, 1.0f,

		-0.5f, 0.0f,  0.5f,    0.0f, 1.0f, 0.0f,   0.0f, -1.0f, 0.0f,    0.0f, 0.0f,
		 0.5f, 0.0f, -0.5f,    0.0f, 1.0f, 0.0f,   0.0f, -1.0f, 0.0f,    1.0f, 1.0f,
		 0.5f, 0.0f,  0.5f,    0.0f, 1.0f, 0.0f,   0.0f, -1.0f, 0.0f,    1.0f, 0.0f
	};

	glGenVertexArrays(1, VAOSuelo);
	glGenBuffers(1, &VBO);
	glBindVertexArray(*VAOSuelo);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	// Posici�n
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	// Color
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);

	// Normal
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void*)(6 * sizeof(float)));
	glEnableVertexAttribArray(2);

	// Coordenadas de textura
	glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void*)(9 * sizeof(float)));
	glEnableVertexAttribArray(3);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}


// Funci�n para preparar el VAO de un cuadrado en el plano XY (paredes)
void CuadradoXY(unsigned int* VAOPared) {
	unsigned int VBO;

	float vertices[] = {
		//    Posici�n             Color             Normal                TexCoords
		-0.5f, -0.5f, 0.0f,     0.0f, 1.0f, 0.0f,   0.0f, 0.0f, 1.0f,     0.0f, 0.0f,
		 0.5f, -0.5f, 0.0f,     0.0f, 1.0f, 0.0f,   0.0f, 0.0f, 1.0f,     1.0f, 0.0f,
		 0.5f,  0.5f, 0.0f,     0.0f, 1.0f, 0.0f,   0.0f, 0.0f, 1.0f,     1.0f, 1.0f,

		-0.5f, -0.5f, 0.0f,     0.0f, 1.0f, 0.0f,   0.0f, 0.0f, 1.0f,     0.0f, 0.0f,
		 0.5f,  0.5f, 0.0f,     0.0f, 1.0f, 0.0f,   0.0f, 0.0f, 1.0f,     1.0f, 1.0f,
		-0.5f,  0.5f, 0.0f,     0.0f, 1.0f, 0.0f,   0.0f, 0.0f, 1.0f,     0.0f, 1.0f
	};

	glGenVertexArrays(1, VAOPared);
	glGenBuffers(1, &VBO);
	glBindVertexArray(*VAOPared);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	// Posici�n
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	// Color
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);

	// Normal
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void*)(6 * sizeof(float)));
	glEnableVertexAttribArray(2);

	// Coordenadas de textura
	glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void*)(9 * sizeof(float)));
	glEnableVertexAttribArray(3);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}


// Funci�n para preparar el VAO de un cuadrado en el plano YZ (paredes)
void CuadradoYZ(unsigned int* VAOPared) {
	unsigned int VBO;

	float vertices[] = {
		//    Posici�n           Color            Normal              TexCoords
		 0.0f, -0.5f, -0.5f,    0.0f, 1.0f, 0.0f,  1.0f, 0.0f, 0.0f,   0.0f, 0.0f,
		 0.0f, -0.5f,  0.5f,    0.0f, 1.0f, 0.0f,  1.0f, 0.0f, 0.0f,   1.0f, 0.0f,
		 0.0f,  0.5f,  0.5f,    0.0f, 1.0f, 0.0f,  1.0f, 0.0f, 0.0f,   1.0f, 1.0f,

		 0.0f, -0.5f, -0.5f,    0.0f, 1.0f, 0.0f,  1.0f, 0.0f, 0.0f,   0.0f, 0.0f,
		 0.0f,  0.5f,  0.5f,    0.0f, 1.0f, 0.0f,  1.0f, 0.0f, 0.0f,   1.0f, 1.0f,
		 0.0f,  0.5f, -0.5f,    0.0f, 1.0f, 0.0f,  1.0f, 0.0f, 0.0f,   0.0f, 1.0f
	};

	glGenVertexArrays(1, VAOPared);
	glGenBuffers(1, &VBO);
	glBindVertexArray(*VAOPared);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	// Posici�n
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	// Color
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);

	// Normal
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void*)(6 * sizeof(float)));
	glEnableVertexAttribArray(2);

	// Coordenadas de textura
	glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void*)(9 * sizeof(float)));
	glEnableVertexAttribArray(3);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

//Funci�n para dibujar el tri�ngulo de la parte INTRO (necesito un VAO para hacerlo objeto de la clase y moverlo en el espacio)
void Triangulo(unsigned int* VAO) {
	unsigned int VBO;

	float vertices[] = {
		// Posiciones         // Colores
		 0.0f,  0.5f, 0.0f,    1.0f, 0.0f, 0.0f, // V�rtice superior (rojo)
		-0.5f, -0.5f, 0.0f,    0.0f, 1.0f, 0.0f, // V�rtice inferior izquierdo (verde)
		 0.5f, -0.5f, 0.0f,    0.0f, 0.0f, 1.0f  // V�rtice inferior derecho (azul)
	};

	glGenVertexArrays(1, VAO);
	glGenBuffers(1, &VBO);

	glBindVertexArray(*VAO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	// Atributo posici�n
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	// Atributo color
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

// Funci�n para preparar el VAO de las esferas
void Esfera(unsigned int* VAO) {
	unsigned int VBO;
	//set up vertex data (and buffer(s)) and configure vertex attributes

	glGenVertexArrays(1, VAO);
	glGenBuffers(1, &VBO);

	glBindVertexArray(*VAO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(sphereAxis), sphereAxis, GL_STATIC_DRAW);

	// Normales
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(1);

	// texture
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(2);

	// vertices
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(5 * sizeof(float)));
	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
	glDeleteBuffers(1, &VBO);
}

// Funci�n que prepara el VAO para un cubo
void Cubo(unsigned int* VAO) {
	unsigned int VBO, EBO;

	float vertices[] = {
		-0.5f, -0.5f,  0.5f,   0.0f, 0.0f, 1.0f,
		 0.5f, -0.5f,  0.5f,   0.0f, 0.0f, 1.0f,
		 0.5f,  0.5f,  0.5f,   0.0f, 0.0f, 1.0f,
		 0.5f,  0.5f,  0.5f,   0.0f, 0.0f, 1.0f,
		-0.5f,  0.5f,  0.5f,   0.0f, 0.0f, 1.0f,
		-0.5f, -0.5f,  0.5f,   0.0f, 0.0f, 1.0f,

		-0.5f, -0.5f, -0.5f,   1.0f, 0.0f, 0.0f,
		 0.5f,  0.5f, -0.5f,   1.0f, 0.0f, 0.0f,
		 0.5f, -0.5f, -0.5f,   1.0f, 0.0f, 0.0f,
		 0.5f,  0.5f, -0.5f,   1.0f, 0.0f, 0.0f,
		-0.5f, -0.5f, -0.5f,   1.0f, 0.0f, 0.0f,
		-0.5f,  0.5f, -0.5f,   1.0f, 0.0f, 0.0f,

		-0.5f,  0.5f, -0.5f,   0.0f, 1.0f, 0.0f,
		-0.5f, -0.5f,  0.5f,   0.0f, 1.0f, 0.0f,
		-0.5f,  0.5f,  0.5f,   0.0f, 1.0f, 0.0f,
		-0.5f, -0.5f,  0.5f,   0.0f, 1.0f, 0.0f,
		-0.5f,  0.5f, -0.5f,   0.0f, 1.0f, 0.0f,
		-0.5f, -0.5f, -0.5f,   0.0f, 1.0f, 0.0f,

		 0.5f,  0.5f, -0.5f,   1.0f, 1.0f, 0.0f,
		 0.5f,  0.5f,  0.5f,   1.0f, 1.0f, 0.0f,
		 0.5f, -0.5f,  0.5f,   1.0f, 1.0f, 0.0f,
		 0.5f, -0.5f,  0.5f,   1.0f, 1.0f, 0.0f,
		 0.5f, -0.5f, -0.5f,   1.0f, 1.0f, 0.0f,
		 0.5f,  0.5f, -0.5f,   1.0f, 1.0f, 0.0f,

		 -0.5f,  0.5f,  0.5f,   1.0f, 0.0f, 1.0f,
		  0.5f,  0.5f,  0.5f,   1.0f, 0.0f, 1.0f,
		  0.5f,  0.5f, -0.5f,   1.0f, 0.0f, 1.0f,
		  0.5f,  0.5f, -0.5f,   1.0f, 0.0f, 1.0f,
		 -0.5f,  0.5f, -0.5f,   1.0f, 0.0f, 1.0f,
		 -0.5f,  0.5f,  0.5f,   1.0f, 0.0f, 1.0f,

		 -0.5f, -0.5f,  0.5f,   0.0f, 1.0f, 1.0f,
		  0.5f, -0.5f, -0.5f,   0.0f, 1.0f, 1.0f,
		  0.5f, -0.5f,  0.5f,   0.0f, 1.0f, 1.0f,
		  0.5f, -0.5f, -0.5f,   0.0f, 1.0f, 1.0f,
		 -0.5f, -0.5f,  0.5f,   0.0f, 1.0f, 1.0f,
		 -0.5f, -0.5f, -0.5f,   0.0f, 1.0f, 1.0f,
	};

	glGenVertexArrays(1, VAO);
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &EBO);

	glBindVertexArray(*VAO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	// Atributos Posicion
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	// Atributo Color
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
	glDeleteBuffers(1, &VBO);
}


// Inicializaciones de openGL
void openGlInit() {
	glClearDepth(1.0f);						// Ajustamos el buffer de profundidad
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);	// Ajustamos el color de fondo
	glEnable(GL_DEPTH_TEST);				// Activamos el test de profundidad, para procesar en 3D
	glEnable(GL_CULL_FACE);					// Activamos el culling, para eliminar caras no visibles
	glCullFace(GL_BACK);					// Eliminamos las caras traseras
	glLineWidth(1.0f);						// Ajustamos el grosor de las l�neas		
}


// Funci�n que nos permite que se actualice el tama�o cuando cambiamos el tama�o de la ventana
void window_size_callback(GLFWwindow* ventana, int nuevoAncho, int nuevoAlto) {
	// Actualiza las variables de ancho y alto de la ventana con los nuevos valores
	SCR_WIDTH = nuevoAncho;
	SCR_HEIGHT = nuevoAlto;

	// Ajusta el viewport para que coincida con el nuevo tama�o de la ventana
	glViewport(0, 0, nuevoAncho, nuevoAlto);
}

// Funci�n que configura la c�mara para observar el cubo desde diferentes caras
void setCamaraCubo(int cara) {  
   // Si la cara es 0, se desactiva la c�mara fija y se restaura la c�mara libre  
   if (cara == 0) {  
       camaraCuboActiva = 0;  
       camaraBloqueada = false;  
       camPos = camPosLibre;  
       camFront = camFrontLibre;  
       camUp = camUpLibre;  
       angRotHoriz = angRotHorizLibre;  
       angRotVert = angRotVertLibre;  
       return;  
   }  

   // Si la c�mara no est� bloqueada, guarda la posici�n y orientaci�n actuales (de forma que luego podamos recuperar la c�mara libre) 
   if (!camaraBloqueada) {  
       camPosLibre = camPos;  
       camFrontLibre = camFront;  
       camUpLibre = camUp;  
       angRotHorizLibre = angRotHoriz;  
       angRotVertLibre = angRotVert;  
   }  

   // Activa la c�mara fija y establece la cara seleccionada  
   camaraCuboActiva = cara;  
   camaraBloqueada = true;  

   // Posici�n relativa del cubo sobre el suelo  
   glm::vec3 posicionRelativa(0.0f, ParedXY.sy / 2, 0.0f);  

   // Calcula el centro del cubo   
   glm::vec3 centro = glm::vec3(SueloCamara.posicion * glm::vec4(posicionRelativa, 1.0f));  

   // Distancia de la c�mara a la cara del cubo 
   float distancia = 8.0f;  

   // Configura la posici�n y orientaci�n de la c�mara seg�n la cara seleccionada  
   switch (cara) {  
   case 1: // Cara +X  
       camPos = centro + glm::vec3(distancia, 0.0f, 0.0f);  
       camUp = glm::vec3(0.0f, 1.0f, 0.0f);  
       break;  
   case 2: // Cara -X  
       camPos = centro + glm::vec3(-distancia, 0.0f, 0.0f);  
       camUp = glm::vec3(0.0f, 1.0f, 0.0f);  
       break;  
   case 3: // Cara +Y  
       camPos = centro + glm::vec3(0.0f, distancia, 0.0f);  
       camUp = glm::vec3(0.0f, 0.0f, -1.0f);  
       break;  
   case 4: // Cara -Y  
       camPos = centro + glm::vec3(0.0f, -distancia, 0.0f);  
       camUp = glm::vec3(0.0f, 0.0f, 1.0f);  
       break;  
   case 5: // Cara +Z  
       camPos = centro + glm::vec3(0.0f, 0.0f, distancia);  
       camUp = glm::vec3(0.0f, 1.0f, 0.0f);  
       break;  
   case 6: // Cara -Z  
       camPos = centro + glm::vec3(0.0f, 0.0f, -distancia);  
       camUp = glm::vec3(0.0f, 1.0f, 0.0f);  
       break;  
   default:  
       return;  
   }  

   // Calcula la direcci�n hacia la que apunta la c�mara  
   camFront = glm::normalize(centro - camPos);  
}



// Funci�n que configura la c�mara  
void myCamara() {  
   // Configura la matriz de proyecci�n utilizando una perspectiva.  
   // La perspectiva se define con el campo de visi�n (fovCamara),  
   // la relaci�n de aspecto (ancho/alto de la ventana) y los planos cercano y lejano.  
   glm::mat4 projection = glm::perspective(glm::radians(fovCamara),  
       (float)SCR_WIDTH / (float)SCR_HEIGHT,  
       0.1f, 200.0f);  

   // Obtiene la ubicaci�n de la variable "projection" en el shader activo.  
   unsigned int proyectionLoc = glGetUniformLocation(shaderProgram, "projection");  

   // Env�a la matriz de proyecci�n al shader.  
   glUniformMatrix4fv(proyectionLoc, 1, GL_FALSE, glm::value_ptr(projection));  

   // Configura la matriz de vista, que define la posici�n y orientaci�n de la c�mara.  
   glm::mat4 view;  
   if (camaraCuboActiva == 0) {  
       // Si la c�mara est� en modo libre, calcula la vista con la posici�n y direcci�n actuales.  
       view = glm::lookAt(camPos, camPos + camFront, camUp);  
   }  
   else {  
       // Si la c�mara est� fija (mirando al cubo), utiliza los valores ajustados previamente.  
       view = glm::lookAt(camPos, camPos + camFront, camUp);  
   }  

   // Obtiene la ubicaci�n de la variable "view" en el shader activo.  
   unsigned int viewLoc = glGetUniformLocation(shaderProgram, "view");  

   // Env�a la matriz de vista al shader.  
   glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));  
}

// Funci�n de callback para el movimiento del rat�n
void mouse_callback(GLFWwindow* window, double xpos, double ypos) {
	// No hacer nada si la c�mara est� fija (mirando al cubo)
	if (camaraBloqueada) return; 

	// Si es el primer movimiento, guardamos las posiciones y se desmarca
	if (primerMovCam) {
		ultimoX = xpos;
		ultimoY = ypos;
		primerMovCam = false;
	}

	// Calculamos cu�nto se ha movido el rat�n respecto a la �ltima posici�n
	float xoffset = xpos - ultimoX;
	float yoffset = ultimoY - ypos;
	ultimoX = xpos;
	ultimoY = ypos;

	// Aplicamos la sensibilidad a ese desplazamiento
	xoffset *= sensibilidad;
	yoffset *= sensibilidad;

	// Recalculamos los �ngulos de rotaci�n de la c�mara
	angRotHoriz += xoffset;
	angRotVert += yoffset;

	// Limitamos el �ngulo de rotaci�n del eje Y, si no produce que se mueva de forma invertida 
	if (angRotVert > 89.0f) angRotVert = 89.0f;
	if (angRotVert < -89.0f) angRotVert = -89.0f;

	// Calculamos la posici�n hacia donde apunta la c�mara
	glm::vec3 front;
	front.x = cos(glm::radians(angRotHoriz)) * cos(glm::radians(angRotVert));
	front.y = sin(glm::radians(angRotVert));
	front.z = sin(glm::radians(angRotHoriz)) * cos(glm::radians(angRotVert));
	camFront = glm::normalize(front);
}

// Funci�n auxiliar para dibujar un suelo individual
// Recibe una referencia a una estructura "suelo", una posici�n de destino y los uniformes de transformaci�n y color
void dibujarSuelo(Estructura& suelo, glm::vec3 posicion, unsigned int transformLoc, unsigned int colorLoc) {
	// Inicializa la matriz de transformaci�n como la identidad
	glm::mat4 transform = glm::mat4(1.0f);

	// Aplica una traslaci�n a la matriz de transformaci�n
	transform = glm::translate(transform, posicion);

	// Guarda la posici�n transformada en el objeto suelo (por si se necesita luego)
	suelo.posicion = transform;

	// Escala el suelo seg�n sus dimensiones
	transform = glm::scale(transform, glm::vec3(suelo.sx, suelo.sy, suelo.sz));

	// Dibuja el objeto suelo con la transformaci�n final, color y modo de dibujo (TRIANGULOS con 6 v�rtices)
	suelo.dibujarObjeto(transformLoc, transform, colorLoc, TRIANGULOS, 6);
}

// Funci�n principal para dibujar todos los suelos de la escena
void dibujarSuelos(unsigned int transformLoc, unsigned int colorLoc, unsigned int useTexture) {
	// Estructura auxiliar para almacenar un suelo junto con su posici�n deseada
	struct InfoSuelo {
		Estructura& suelo;
		glm::vec3 posicion;
	};

	// Vector que contiene todos los suelos con sus respectivas posiciones en el mundo
	std::vector<InfoSuelo> suelos = {
		{SueloPasillo, glm::vec3(0.0f, 0.0f, 0.0f)},
		{SueloIntro, glm::vec3(-33.f, 0.0f, 33.3f)},
		{SueloModelado, glm::vec3(-33.f, 0.0f, 0.0f)},
		{SueloTransformaciones, glm::vec3(-33.f, 0.0f, -33.3f)},
		{SueloCamara, glm::vec3(0.0f, 0.0f, -66.6f)},
		{SueloIluminacion, glm::vec3(33.f, 0.0f, -33.3f)},
		{SueloTexturas, glm::vec3(33.f, 0.0f, 0.0f)},
		{SueloColisiones, glm::vec3(33.f, 0.0f, 33.3f)}
	};

	// Activa el uso de la textura del VAO antes de dibujar los suelos
	glUniform1i(useTexture, true);

	// Recorre cada suelo del vector y lo dibuja usando la funci�n auxiliar
	for (const auto& info : suelos) {
		dibujarSuelo(info.suelo, info.posicion, transformLoc, colorLoc);
	}

	// Desactiva el uso de la textura para dejar el estado gr�fico limpio
	glUniform1i(useTexture, false);
}


// Funci�n auxiliar para dibujar una pared individual en dos direcciones
void dibujarPared(glm::vec3 posicion, Estructura& pared, unsigned int transformLoc, unsigned int colorLoc) {
	// Creamos una matriz de transformaci�n identidad
	glm::mat4 transform = glm::mat4(1.0f);

	// Aplicamos la traslaci�n para posicionar la pared
	transform = glm::translate(transform, posicion);

	// Escalamos la pared seg�n sus dimensiones
	transform = glm::scale(transform, glm::vec3(pared.sx, pared.sy, pared.sz));

	// Dibujamos la pared en la orientaci�n original
	pared.dibujarObjeto(transformLoc, transform, colorLoc, TRIANGULOS, 6);

	// Giramos la matriz 180 grados en el eje Y para dibujar la pared por el lado opuesto
	transform = glm::rotate(transform, glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f));

	// Dibujamos nuevamente la pared rotada
	pared.dibujarObjeto(transformLoc, transform, colorLoc, TRIANGULOS, 6);
}

// Funci�n principal para dibujar todas las paredes del entorno
void dibujarParedes(unsigned int transformLoc, unsigned int colorLoc, unsigned int useTexture) {
	// Activamos el uso de textura para las paredes
	glUniform1i(useTexture, true);

	// Dibujamos la pared central (ParedXYTotal) detr�s del suelo de introducci�n
	dibujarPared(glm::vec3(0.0f, ParedXYTotal.sy / 2, (SueloIntro.sz / 2) + 33.3f), ParedXYTotal, transformLoc, colorLoc);

	// Coordenada base en Z para ubicar las paredes XY alrededor del conjunto de suelos
	float zBase = SueloIntro.sz / 2;

	// Vector con las posiciones donde se colocar�n las paredes en el plano XY
	std::vector<glm::vec3> posicionesXY = {
		{-33.f, ParedXY.sy / 2, zBase},
		{-33.f, ParedXY.sy / 2, -zBase},
		{-33.f, ParedXY.sy / 2, -zBase - 33.3f},
		{ 0.f,  ParedXY.sy / 2, -SueloIntro.sz * 2.5f},
		{+33.f, ParedXY.sy / 2, zBase},
		{+33.f, ParedXY.sy / 2, -zBase},
		{+33.f, ParedXY.sy / 2, -zBase - 33.3f}
	};

	// Dibujamos todas las paredes XY en sus respectivas posiciones
	for (const auto& pos : posicionesXY) {
		dibujarPared(pos, ParedXY, transformLoc, colorLoc);
	}

	// Vector con las posiciones donde se colocar�n las paredes en el plano YZ (laterales)
	std::vector<glm::vec3> posicionesYZ = {
		{-(SueloPasillo.sx / 2) - 33.15f, ParedYZ.sy / 2, SueloIntro.sz},
		{-(SueloPasillo.sx / 2) - 33.15f, ParedYZ.sy / 2, 0.0f},
		{-(SueloPasillo.sx / 2) - 33.15f, ParedYZ.sy / 2, -SueloTransformaciones.sz},
		{-(SueloPasillo.sx / 2),         ParedYZ.sy / 2, -(SueloCamara.sz * 2)},
		{+(SueloPasillo.sx / 2),         ParedYZ.sy / 2, -(SueloCamara.sz * 2)},
		{+(SueloPasillo.sx / 2) + 33.15f, ParedYZ.sy / 2, SueloIntro.sz},
		{+(SueloPasillo.sx / 2) + 33.15f, ParedYZ.sy / 2, 0.0f},
		{+(SueloPasillo.sx / 2) + 33.15f, ParedYZ.sy / 2, -SueloTransformaciones.sz}
	};

	// Dibujamos las paredes YZ en sus posiciones correspondientes
	for (const auto& pos : posicionesYZ) {
		dibujarPared(pos, ParedYZ, transformLoc, colorLoc);
	}

	// Desactivamos el uso de la textura para dejar el estado OpenGL limpio
	glUniform1i(useTexture, false);
}

// Funci�n para dibujar un solo techo en una posici�n espec�fica
void dibujarTecho(Estructura& techo, glm::vec3 posicion, unsigned int transformLoc, unsigned int colorLoc) {
	// Creamos una matriz de transformaci�n identidad
	glm::mat4 transform = glm::mat4(1.0f);

	// Aplicamos una traslaci�n a la posici�n deseada
	transform = glm::translate(transform, posicion);

	// Guardamos esta transformaci�n en el atributo `posicion` del techo
	techo.posicion = transform;

	// Escalamos el objeto techo seg�n sus dimensiones sx, sy, sz
	transform = glm::scale(transform, glm::vec3(techo.sx, techo.sy, techo.sz));

	// Dibujamos el objeto usando la transformaci�n, el color y el modo de dibujo
	techo.dibujarObjeto(transformLoc, transform, colorLoc, TRIANGULOS, 6);
}

// Funci�n para dibujar todos los techos del entorno
void dibujarTechos(unsigned int transformLoc, unsigned int colorLoc, unsigned int useTextureLoc) {
	// Estructura auxiliar que asocia un techo con su posici�n
	struct InfoTecho {
		Estructura& techo;
		glm::vec3 posicion;
	};

	// Definimos una altura com�n para todos los techos: igual a la altura de una pared
	float alturaTecho = ParedXYTotal.sy;

	// Creamos un vector con los techos y sus posiciones
	std::vector<InfoTecho> techos = {
		// TechoTotal se coloca sobre el centro del conjunto de suelos del pasillo
		{TechoTotal, glm::vec3(0.0f, alturaTecho, 0.0f)},

		// Techo sobre la zona de la c�mara, desplazado en Z
		{Techo, glm::vec3(SueloCamara.px, alturaTecho, -100 + SueloCamara.sz)}
	};

	// Activamos el uso de texturas para los techos
	glUniform1i(useTextureLoc, true);

	// Dibujamos todos los techos definidos
	for (const auto& info : techos) {
		dibujarTecho(info.techo, info.posicion, transformLoc, colorLoc);
	}

	// Desactivamos el uso de texturas para dejar el estado limpio
	glUniform1i(useTextureLoc, false);
}

//Funci�n que dibuja el tri�ngulo de la introducci�n
void dibujarTrianguloIntro(unsigned int transformLoc, unsigned int colorLoc, unsigned int useVertexColorLoc) {   
   // Aplicamos una traslaci�n al tri�ngulo para posicionarlo en la escena  
   TrianguloIntro.transform = glm::translate(SueloIntro.posicion, glm::vec3(TrianguloIntro.px, TrianguloIntro.py + 5, TrianguloIntro.pz));  

   // Rotamos el tri�ngulo 90 grados en el eje Y  
   TrianguloIntro.transform = glm::rotate(TrianguloIntro.transform, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));  

   // Escalamos el tri�ngulo seg�n sus dimensiones  
   TrianguloIntro.transform = glm::scale(TrianguloIntro.transform, glm::vec3(TrianguloIntro.sx, TrianguloIntro.sy, TrianguloIntro.sz));  

   // Activamos el uso del color del VAO 
   glUniform1i(useVertexColorLoc, true); 

   // Dibujamos el tri�ngulo con la transformaci�n aplicada  
   TrianguloIntro.dibujarObjeto(transformLoc, TrianguloIntro.transform, colorLoc, GL_TRIANGLES, 3);  

   // Desactivamos el uso del color del VAO
   glUniform1i(useVertexColorLoc, false);   
}


// Funci�n que dibuja un conjunto de esferas con distintas posiciones y configuraciones de color por v�rtice
void dibujarEsferas(unsigned int transformLoc, unsigned int colorLoc, unsigned int useVertexColorLoc) {

	// Estructura para almacenar los datos necesarios para dibujar cada esfera
	struct EsferaData {
		Estructura* esfera;              // Puntero a la estructura de la esfera a dibujar
		glm::mat4 posicionSuelo;         // Matriz de transformaci�n del suelo sobre el que se coloca
		glm::vec3 posicionRelativa;      // Desplazamiento respecto a la posici�n del suelo
		bool usaColorVertice;            // Indica si se usa el color por v�rtice
	};

	// Definimos un array de esferas a dibujar con sus datos correspondientes
	EsferaData esferas[] = {
		// Esfera en la zona de modelado, a 5 unidades por encima del suelo
		{ &EsferaModelado, SueloModelado.posicion,   glm::vec3(0.0f, 5.0f, 0.0f),          false },

		// Esfera en la zona de iluminaci�n, a media altura de la pared
		{ &EsferaIluminacion, SueloIluminacion.posicion, glm::vec3(0.0f, ParedXY.sy / 2, 0.0f), false }
	};

	// Recorremos cada esfera para calcular su transformaci�n y dibujarla
	for (const auto& e : esferas) {
		// Calculamos la posici�n final sumando la posici�n del suelo y el desplazamiento relativo
		glm::mat4 transform = glm::translate(e.posicionSuelo, e.posicionRelativa);

		// Escalamos la esfera con sus dimensiones individuales
		transform = glm::scale(transform, glm::vec3(e.esfera->sx, e.esfera->sy, e.esfera->sz));

		// Guardamos esta transformaci�n en la estructura de la esfera (por si se necesita m�s adelante)
		e.esfera->transform = transform;

		// Activamos o desactivamos el uso de color por v�rtice para esta esfera
		glUniform1i(useVertexColorLoc, e.usaColorVertice);

		// Dibujamos la esfera con la transformaci�n calculada
		e.esfera->dibujarObjeto(transformLoc, e.esfera->transform, colorLoc, GL_TRIANGLES, sphereVertexCount);
	}

	// Restauramos el uso de color por v�rtice a desactivado tras terminar
	glUniform1i(useVertexColorLoc, false);
}

// Funci�n para posicionar la luz de iluminaci�n en la escena  
void posicionarLuzIluminacion(GLuint shaderProgram) {  
   // Calcula la posici�n de la luz, coloc�ndola justo encima de la esfera de iluminaci�n  
   glm::vec3 luzPos = glm::vec3(SueloIluminacion.posicion * glm::vec4(0, 0, 0, 1)) + glm::vec3(0.0f, ParedXY.sy / 2 + 3, 0.0f);  

   // Env�a al shader si la iluminaci�n est� activa o no  
   glUniform1i(glGetUniformLocation(shaderProgram, "luzIluminacionActiva"), luzIluminacionActiva ? 1 : 0);  

   // Env�a la posici�n de la luz al shader  
   glUniform3fv(glGetUniformLocation(shaderProgram, "luzIluminacionPos"), 1, glm::value_ptr(luzPos));  

   // Env�a la posici�n de la c�mara al shader para c�lculos de iluminaci�n  
   glUniform3fv(glGetUniformLocation(shaderProgram, "viewPos"), 1, glm::value_ptr(camPos));  
}


// Funci�n para dibujar y mover el cubo aleatoriamente dentro de la habitaci�n de transformaciones
void dibujarCubo(unsigned int transformLoc, unsigned int colorLoc, unsigned int useVertexColorLoc) {
	// Calculamos los l�mites del espacio dentro del cual el cubo puede moverse,
	// teniendo en cuenta el tama�o del suelo y del propio cubo, para evitar que lo atraviese.
	float minX = -SueloTransformaciones.sx / 2.0f + CuboTransformaciones.sx / 2.0f;
	float maxX = SueloTransformaciones.sx / 2.0f - CuboTransformaciones.sx / 2.0f;
	float minZ = -SueloTransformaciones.sz / 2.0f + CuboTransformaciones.sz / 2.0f;
	float maxZ = SueloTransformaciones.sz / 2.0f - CuboTransformaciones.sz / 2.0f;
	float minY = 0.0f + CuboTransformaciones.sy / 2.0f; // altura m�nima (suelo)
	float maxY = ParedXY.sy - CuboTransformaciones.sy / 2.0f; // altura m�xima (techo)

	// Variables est�ticas para mantener la posici�n y direcci�n entre llamadas sucesivas (simula "estado")
	static glm::vec3 pos = glm::vec3(0.0f, minY, 0.0f); // posici�n inicial
	static glm::vec3 dir = glm::normalize(glm::vec3(0.5f, 0.2f, 0.3f)); // direcci�n de movimiento normalizada
	static std::mt19937 rng(std::random_device{}()); // generador de n�meros aleatorios
	static std::uniform_real_distribution<float> dist(-1.0f, 1.0f); // distribuci�n para desv�o aleatorio

	float velocidad = 0.2f; // Velocidad constante

	// Actualiza la posici�n del cubo con la direcci�n y velocidad actual
	pos += dir * velocidad;

	// Detectamos colisiones con los l�mites y reflejamos la direcci�n si es necesario
	bool rebote = false;
	if (pos.x < minX) { pos.x = minX; dir.x = -dir.x; rebote = true; }
	if (pos.x > maxX) { pos.x = maxX; dir.x = -dir.x; rebote = true; }
	if (pos.z < minZ) { pos.z = minZ; dir.z = -dir.z; rebote = true; }
	if (pos.z > maxZ) { pos.z = maxZ; dir.z = -dir.z; rebote = true; }
	if (pos.y < minY) { pos.y = minY; dir.y = -dir.y; rebote = true; }
	if (pos.y > maxY) { pos.y = maxY; dir.y = -dir.y; rebote = true; }

	// Si el cubo rebota, cambia un poco su direcci�n para hacer el movimiento m�s natural
	if (rebote) {
		dir.x += dist(rng) * 0.2f;
		dir.y += dist(rng) * 0.2f;
		dir.z += dist(rng) * 0.2f;
		dir = glm::normalize(dir); // Re-normalizamos la direcci�n tras el cambio
	}

	// Calculamos la matriz de transformaci�n
	CuboTransformaciones.transform = glm::translate(SueloTransformaciones.posicion, glm::vec3(pos.x, pos.y, pos.z));
	CuboTransformaciones.transform = glm::rotate(CuboTransformaciones.transform, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f)); // rotaci�n en Y
	CuboTransformaciones.transform = glm::scale(CuboTransformaciones.transform, glm::vec3(CuboTransformaciones.sx, CuboTransformaciones.sy, CuboTransformaciones.sz));

	// Activamos color por v�rtice y dibujamos el cubo
	glUniform1i(useVertexColorLoc, true);
	CuboTransformaciones.dibujarObjeto(transformLoc, CuboTransformaciones.transform, colorLoc, GL_TRIANGLES, 36);
	glUniform1i(useVertexColorLoc, false); // Restauramos el estado por defecto
}

void dibujarCubos(unsigned int transformLoc, unsigned int colorLoc, unsigned int useVertexColorLoc) {
	// Estructura para guardar la informaci�n necesaria de cada cubo
	struct CuboData {
		Estructura* cubo;             // Puntero al cubo que se va a dibujar
		glm::mat4 posicionSuelo;      // Matriz de posici�n base (suelo) del cubo
		glm::vec3 posicionRelativa;   // Posici�n relativa sobre ese suelo
		bool usaColorVertice;         // Indica si se debe usar color por v�rtice
	};

	// Inicializamos un array de cubos con sus datos
	CuboData cubos[] = {
		// Cubo de la c�mara: centrado en el suelo, flotando en mitad de la altura de la pared
		{ &CuboCamara, SueloCamara.posicion, glm::vec3(0.0f, ParedXY.sy / 2, 0.0f), true },

		// Cubo de iluminaci�n: a media altura menos un desplazamiento
		{ &CuboIluminacion, SueloIluminacion.posicion, glm::vec3(0.0f, ParedXY.sy / 2 - 0.75f, 0.0f), false },

		// Cubo de colisiones: apoyado directamente sobre el suelo
		{ &CuboColisiones, SueloColisiones.posicion, glm::vec3(0.0f, CuboColisiones.sy / 2, 0.0f), true }
	};

	// Recorremos cada cubo y aplicamos las transformaciones
	for (const auto& c : cubos) {
		// Aplicamos traslaci�n desde la matriz de posici�n base del suelo
		glm::mat4 transform = glm::translate(c.posicionSuelo, c.posicionRelativa);

		// Aplicamos escalado para adaptar el tama�o real del cubo
		transform = glm::scale(transform, glm::vec3(c.cubo->sx, c.cubo->sy, c.cubo->sz));

		// Guardamos la transformaci�n resultante en el objeto
		c.cubo->transform = transform;

		// Indicamos si se debe usar color por v�rtice
		glUniform1i(useVertexColorLoc, c.usaColorVertice);

		// Dibujamos el cubo con su transformaci�n
		c.cubo->dibujarObjeto(transformLoc, c.cubo->transform, colorLoc, GL_TRIANGLES, 36);
	}

	// Restauramos el estado por defecto (sin color por v�rtice)
	glUniform1i(useVertexColorLoc, false);
}


// Funci�n que verifica si la c�mara colisiona con un cubo espec�fico  
bool camaraColisionaConCubo(const glm::vec3& camPos, const Estructura& cubo) {
	// Calcula el centro global del cubo utilizando la posici�n del suelo asociado  
	glm::vec3 centroSuelo = glm::vec3(SueloColisiones.posicion * glm::vec4(0, 0, 0, 1));
	glm::vec3 centroCubo = centroSuelo + glm::vec3(0.0f, cubo.sy / 2.0f, 0.0f);

	// Calcula los extremos del cubo (m�nimo y m�ximo)
	glm::vec3 minCubo = centroCubo - glm::vec3(cubo.sx, cubo.sy, cubo.sz) * 0.5f;
	glm::vec3 maxCubo = centroCubo + glm::vec3(cubo.sx, cubo.sy, cubo.sz) * 0.5f;

	// Realiza la comprobaci�n de colisi�n usando AABB
	return (camPos.x >= minCubo.x && camPos.x <= maxCubo.x) &&
		(camPos.y >= minCubo.y && camPos.y <= maxCubo.y) &&
		(camPos.z >= minCubo.z && camPos.z <= maxCubo.z);
}

// Funci�n que genera una posici�n aleatoria en el suelo del pasillo  
// El suelo est� centrado en (0,0,0), con tama�o SueloPasillo.sx x SueloPasillo.sz  
glm::vec3 posicionAleatoriaEnSueloPasillo() {  
   // Calculamos los l�mites del suelo en los ejes X y Z, dejando un margen de 1.0f  
   float minX = -SueloPasillo.sx / 2.0f + 1.0f;  
   float maxX = SueloPasillo.sx / 2.0f - 1.0f;  
   float minZ = -SueloPasillo.sz / 2.0f + 1.0f;  
   float maxZ = SueloPasillo.sz / 2.0f - 1.0f;  

   // Generador de n�meros aleatorios para las coordenadas X y Z  
   static std::mt19937 rng(std::random_device{}());  
   std::uniform_real_distribution<float> distX(minX, maxX);  
   std::uniform_real_distribution<float> distZ(minZ, maxZ);  

   // Generamos las coordenadas aleatorias dentro de los l�mites calculados  
   float x = distX(rng);  
   float z = distZ(rng);  

   // Retornamos la posici�n generada como un vector 3D  
   return glm::vec3(x, 5.0f, z);  
}

// Funci�n para el control del programa
void processInput(GLFWwindow* window) {
	// Obtiene el monitor principal y su modo de video
	GLFWmonitor* monitor = glfwGetPrimaryMonitor();
	const GLFWvidmode* mode = glfwGetVideoMode(monitor);

	// Solo permite mover la c�mara si est� libre (no est� bloqueada)
	if (!camaraBloqueada) {
		// Si se presiona ESC, se marca la ventana para cerrarse
		if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
			glfwSetWindowShouldClose(window, true);
		}

		// Movimiento hacia adelante (W)
		if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
			camPos += velCamara * camFront;

		// Movimiento hacia atr�s (S)
		if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
			camPos -= velCamara * camFront;

		// Movimiento a la izquierda (A): vector perpendicular a la direcci�n frontal y hacia arriba
		if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
			camPos -= glm::normalize(glm::cross(camFront, camUp)) * velCamara;

		// Movimiento a la derecha (D)
		if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
			camPos += glm::normalize(glm::cross(camFront, camUp)) * velCamara;

		// Fijar la altura (Y) de la c�mara para evitar que suba o baje
		camPos.y = 5.0f;
	}

	// Alternar entre pantalla completa y modo ventana con la tecla P
	if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS) {
		if (!pPulsado) { // Solo se ejecuta una vez por pulsaci�n
			esPantCompleta = !esPantCompleta; 

			if (esPantCompleta) {
				// Guarda la posici�n y tama�o de la ventana actual
				glfwGetWindowPos(window, &ventanaX, &ventanaY);
				glfwGetWindowSize(window, &anchoVentana, &altoVentana);

				// Pone la ventana en modo pantalla completa
				glfwSetWindowMonitor(window, monitor, 0, 0,
					mode->width, mode->height,
					mode->refreshRate);
			}
			else {
				// Restaura la ventana a su posici�n y tama�o anterior
				glfwSetWindowMonitor(window, nullptr,
					ventanaX, ventanaY,
					anchoVentana, altoVentana,
					0);
			}
		}
		// Marca que la tecla P est� presionada
		pPulsado = true; 
	}
	else {
		// Permite volver a detectar la tecla cuando se suelte
		pPulsado = false; 
	}

	// Selecci�n de c�mara con teclas 0 a 6
	if (glfwGetKey(window, GLFW_KEY_0) == GLFW_PRESS) setCamaraCubo(0);
	if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS) setCamaraCubo(1);
	if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS) setCamaraCubo(2);
	if (glfwGetKey(window, GLFW_KEY_3) == GLFW_PRESS) setCamaraCubo(3);
	if (glfwGetKey(window, GLFW_KEY_4) == GLFW_PRESS) setCamaraCubo(4);
	if (glfwGetKey(window, GLFW_KEY_5) == GLFW_PRESS) setCamaraCubo(5);
	if (glfwGetKey(window, GLFW_KEY_6) == GLFW_PRESS) setCamaraCubo(6);

	// Alternar encendido/apagado de la luz con la tecla 7
	if (glfwGetKey(window, GLFW_KEY_7) == GLFW_PRESS) {
		if (!tecla7Presionada) { 
			luzIluminacionActiva = !luzIluminacionActiva; 
			tecla7Presionada = true;
		}
	}
	else {
		tecla7Presionada = false; 
	}
}

// Funci�n que se encarga de dibujar los objetos en la escena
void Display() {
	// Usamos el shader
	glUseProgram(shaderProgram);
	// Buscamos en el shader las variables uniformes
	unsigned int transformLoc = glGetUniformLocation(shaderProgram, "model");
	unsigned int colorLoc = glGetUniformLocation(shaderProgram, "ourColor");
	unsigned int useVertexColorLoc = glGetUniformLocation(shaderProgram, "useVertexColor");
	signed int luzActivaLoc = glGetUniformLocation(shaderProgram, "luzIluminacionActiva");
	unsigned int luzPosLoc = glGetUniformLocation(shaderProgram, "luzIluminacionPos");
    unsigned int useTextureLoc = glGetUniformLocation(shaderProgram, "useTexture");


	// Limpiamos el buffer de color y el de profundidad
	glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Configuramos la c�mara
	myCamara();
	
	// Dibujamos los suelos
	dibujarSuelos(transformLoc, colorLoc, useTextureLoc);

	// Dibujamos las paredes
	dibujarParedes(transformLoc, colorLoc, useTextureLoc);

	// Dibujamos los techos
	dibujarTechos(transformLoc, colorLoc, useTextureLoc);

	// Dibujamos el tri�ngulo de la parte INTRO
	dibujarTrianguloIntro(transformLoc, colorLoc, useVertexColorLoc);

	// Dibujamos las esferas que usamos
	dibujarEsferas(transformLoc, colorLoc, useVertexColorLoc);

	// Dibujamos los cubos que usamos (est�ticos)
	dibujarCubos(transformLoc, colorLoc, useVertexColorLoc);

	// Dibujamos el cubo (movimiento aleatorio)
	dibujarCubo(transformLoc, colorLoc, useVertexColorLoc);

	// Posicionamos la luz donde queremos
	posicionarLuzIluminacion(shaderProgram);

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}

int main() {
	// Devlaramos las variables
	unsigned int VAOSuelo, VAOPared, VAOTecho, VAOTriangulo, VAOEsfera, VAOCubo, VAOCuboCamara, VAOEsferaLuces, VAOCuboLuces, VAOCuboColisiones;

	// Inicializamos GLFW
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	// Creo la ventana
	GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Grua", NULL, NULL);
	if (window == NULL) {
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}

	// Establezco la ventana como contexto
	glfwMakeContextCurrent(window);
	// Establezco la funci�n para el cambio de tama�o de la ventana
	glfwSetWindowSizeCallback(window, window_size_callback);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

	glfwSetCursorPosCallback(window, mouse_callback);
	// Se oculta el curosr
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED); 

	// Inicializo GLAD
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}

	openGlInit();

	// Cargamos los shaders
	shaderProgram = setShaders("shader.vert", "shader.frag");

	// Preparamos los VAO's y las texturas donde las usemos
	CuadradoXZ(&VAOSuelo);
	SueloPasillo.VAO = SueloIntro.VAO = SueloModelado.VAO = SueloTransformaciones.VAO = SueloCamara.VAO = SueloIluminacion.VAO = SueloTexturas.VAO = SueloColisiones.VAO = VAOSuelo;
	SueloPasillo.texture = myCargaTexturas("marmol.jpg");
	SueloIntro.texture = SueloModelado.texture = SueloTransformaciones.texture = SueloCamara.texture = SueloIluminacion.texture = SueloTexturas.texture = SueloColisiones.texture = myCargaTexturas("marmol.jpg");
	
	CuadradoXZTecho(&VAOTecho);
	TechoTotal.VAO = Techo.VAO = VAOTecho;
	TechoTotal.texture = Techo.texture = myCargaTexturas("marmol.jpg");

	CuadradoXY(&VAOPared);
	ParedXY.VAO = ParedXYTotal.VAO = VAOPared;
	ParedXY.texture = ParedXYTotal.texture = myCargaTexturas("marmol.jpg");

	CuadradoYZ(&VAOPared);
	ParedYZ.VAO = VAOPared;
	ParedYZ.texture = myCargaTexturas("marmol.jpg");

	Triangulo(&VAOTriangulo);
	TrianguloIntro.VAO = VAOTriangulo;

	Esfera(&VAOEsfera);
	EsferaModelado.VAO = VAOEsfera;

	Cubo(&VAOCubo);
	CuboTransformaciones.VAO = VAOCubo;

	Cubo(&VAOCuboCamara);
	CuboCamara.VAO = VAOCuboCamara;

	Esfera(&VAOEsferaLuces);
	EsferaIluminacion.VAO = VAOEsferaLuces;
	Cubo(&VAOCuboLuces);
	CuboIluminacion.VAO = VAOCuboLuces;

	Cubo(&VAOCuboColisiones);
	CuboColisiones.VAO = VAOCuboColisiones;

	

	// Lazo de la ventana mientras no la cierre
	while (!glfwWindowShouldClose(window)) {

		// Funci�n para leer los inputs del teclado
		processInput(window);

		// Funci�n que se encargar� de dibujar los objetos
		Display();

		// Comprobar colisi�n de la c�mara con el cubo de colisiones
		if (camaraColisionaConCubo(camPos, CuboColisiones)) {
			camPos = posicionAleatoriaEnSueloPasillo();
		}

		// Swap buffers y poll events
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	// Liberamos los recursos
	// glDeleteVertexArrays(1, &VAO);
	glfwTerminate();
	return 0;
}