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

//Declaramos algunas constantes
#define TRIANGULOS GL_TRIANGLES
#define sphereVertexCount 1080
int SCR_WIDTH = 800;	//Declara el ancho de la ventana
int SCR_HEIGHT = 800;	//Declara la altura de la ventana

//Declaramos los shaders
extern GLuint setShaders(const char* nVertx, const char* nFrag);
GLuint shaderProgram; 

/*			^-Z
*			|
*			|
*			|			
   -X <----   ----> +X
*			|
*			|
*			|
* 		    v+Z
*/

//Declaramos variables globales para la cámara
glm::vec3 camPos = glm::vec3(.0f, 5.0f, 33.0f); // Posición inicial de la cámara  
glm::vec3 camFront = glm::normalize(glm::vec3(0.0f, 0.0f, -1.0f)); // Dirección hacia +Z
glm::vec3 camUp = glm::vec3(0.0f, 1.0f, 0.0f); // Vector hacia arriba
glm::vec3 horizontalFront = glm::normalize(glm::vec3(camFront.x, 0.0f, camFront.z));
float fovCamara = 45.0f;
float cameraSpeed = 0.1f;
float yaw = -90.0f;
float pitch = 0.0f;
bool firstMouse = true;
float lastX = SCR_WIDTH / 2.0;
float lastY = SCR_HEIGHT / 2.0;
float sensitivity = 0.05f;
bool isFullscreen = false;
bool pWasPressed = false;
int windowedX, windowedY, windowedWidth, windowedHeight;

//Declaramos la clase con la que crearemos los diferentes suelos
class Estructura {
public:
	//Atributos públicos de los objetos
	float px, py, pz;        // posición inicial
	float sx, sy, sz;        // escalado
	unsigned int VAO;        // Vertex Array Object
	int texture;             // textura del objeto
	glm::mat4 transform;     // matriz de transformación
	glm::vec3 color;         // color del objeto
	glm::mat4 posicion; // matriz de posición

	//Constructor por defecto
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

	//Constructor con parámetros. En caso de no recibir textura, se le asigna 0, pero permite cargarla
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

	//Método que devuelve la matriz de transformación
	glm::mat4 getTransform() const {
		return transform;
	}

	// Método para dibujar el objeto (Solo para emplear una vez se hacen las transformaciones, ya que transform se va modificando)
	void dibujarObjeto(unsigned int transformLoc, glm::mat4 transform, unsigned int colorLoc, GLenum primitiva, unsigned int numElem) {
		//Configurar la matriz de transformación y el color
		glUniformMatrix4fv(transformLoc, 1, GL_FALSE, glm::value_ptr(transform));
		glUniform3fv(colorLoc, 1, glm::value_ptr(color));

		//Configurar las texturas
		/*if (texture != 0) {
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, texture);
			glUniform1i(glGetUniformLocation(shaderProgram, "ourTexture"), 0);
		}*/

		//Dibujar el objeto
		glBindVertexArray(VAO);
		glDrawArrays(primitiva, 0, numElem);
		glBindVertexArray(0);

		//Restaurar configuración de texturas
		/*glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, 0);*/
	}
};

//////CREACIÓN DEL ENTORNO///////
//Declaramos los suelos de forma global
Estructura SueloPasillo(0, 0, 0, 33.0f, 1.0f, 100.0f, 0, glm::vec3(1, 1, 1));

Estructura SueloIntro(0, 0, 0, 33.3f, 1.0f, 33.3f, 0, glm::vec3(.1f, .1f, .1f));
Estructura SueloModelado(0, 0, 0, 33.3f, 1.0f, 33.3f, 0, glm::vec3(.2f, .2f, .2f));
Estructura SueloTransformaciones(0, 0, 0, 33.3f, 1.0f, 33.3f, 0, glm::vec3(.3f, .3f, .3f));

Estructura SueloCamara(0, 0, 0, 33.3f, 1.0f, 33.3f, 0, glm::vec3(.4f, .4f, .4f));

Estructura SueloIluminacion(0, 0, 0, 33.3f, 1.0f, 33.3f, 0, glm::vec3(.5f, .5f, .5f));
Estructura SueloTexturas(0, 0, 0, 33.3f, 1.0f, 33.3f, 0, glm::vec3(.6f, .6f, .6f));
Estructura SueloColisiones(0, 0, 0, 33.3f, 1.0f, 33.3f, 0, glm::vec3(.7f, .7f, .7f));

Estructura ParedXYTotal(0, 0, 0, 99.3f, 12.0f, 1.0f, 0, glm::vec3(.9f, .9f, .9f));
Estructura ParedXY(.0f, 0.0f, .0f, 33.3f, 12.0f, 1.0f, 0, glm::vec3(.9f, .9f, .9f));
Estructura ParedYZ(.0f, 0.0f, .0f, 1.0f, 12.0f, 33.3f, 0, glm::vec3(.9f, .9f, .9f));

////TRIANGULO PARA LA HABITACIÓN INTRO//////
Estructura TrianguloIntro(0.0f, 0.0f, 0.0f, 7.0f, 7.0f, 7.0f, 0, glm::vec3(1.0f, 1.0f, 1.0f));

////ESFERA PARA LA HABITACIÓN DEL MODELADO////
Estructura  EsferaModelado(0, 0, 0, 4.0, 4.0, 4.0, 0, glm::vec3(0.0, 0.0, 0.0));


//////CUBO PARA LA HABITACIÓN DE LAS TRANSFORMACIONES//////
Estructura CuboTransformaciones(0, 0, 0, 2.0, 2.0, 2.0, 0, glm::vec3(1.0, 1.0, 1.0));

//Función para preparar el VAO del cuadrado en el plano XZ
void CuadradoXZ(unsigned int* VAOSuelo) {
	unsigned int VBO;

	float vertices[] = {
		//     Posición         Color
		-.5, 0.0f,  .5f,     0.0, 1.0, 0.0,
		 .5, 0.0f,  .5f,     0.0, 1.0, 0.0,
		 .5, 0.0f, -.5f,     0.0, 1.0, 0.0,

		-.5, 0.0f,  .5f,     0.0, 1.0, 0.0,
		 .5, 0.0f, -.5f,     0.0, 1.0, 0.0,
		-.5, 0.0f, -.5f,     0.0, 1.0, 0.0
	};

	glGenVertexArrays(1, VAOSuelo);
	glGenBuffers(1, &VBO);
	glBindVertexArray(*VAOSuelo);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	// Posición
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	// Color
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

// Función para preparar el VAO de un cuadrado en el plano XY (ideal para paredes)
void CuadradoXY(unsigned int* VAOPared) {
	unsigned int VBO;

	float vertices[] = {
		//     Posición         Color
		-0.5f, -0.5f, 0.0f,     0.0f, 1.0f, 0.0f,
		 0.5f, -0.5f, 0.0f,     0.0f, 1.0f, 0.0f,
		 0.5f,  0.5f, 0.0f,     0.0f, 1.0f, 0.0f,

		-0.5f, -0.5f, 0.0f,     0.0f, 1.0f, 0.0f,
		 0.5f,  0.5f, 0.0f,     0.0f, 1.0f, 0.0f,
		-0.5f,  0.5f, 0.0f,     0.0f, 1.0f, 0.0f
	};

	glGenVertexArrays(1, VAOPared);
	glGenBuffers(1, &VBO);
	glBindVertexArray(*VAOPared);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	// Posición
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	// Color
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

// Función para preparar el VAO de un cuadrado en el plano YZ (ideal para paredes)
void CuadradoYZ(unsigned int* VAOPared) {
	unsigned int VBO;

	float vertices[] = {
		//     Posición             Color
		 0.0f, -0.5f, -0.5f,       0.0f, 1.0f, 0.0f,
		 0.0f, -0.5f,  0.5f,       0.0f, 1.0f, 0.0f,
		 0.0f,  0.5f,  0.5f,       0.0f, 1.0f, 0.0f,

		 0.0f, -0.5f, -0.5f,       0.0f, 1.0f, 0.0f,
		 0.0f,  0.5f,  0.5f,       0.0f, 1.0f, 0.0f,
		 0.0f,  0.5f, -0.5f,       0.0f, 1.0f, 0.0f
	};

	glGenVertexArrays(1, VAOPared);
	glGenBuffers(1, &VBO);
	glBindVertexArray(*VAOPared);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	// Posición
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	// Color
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

//Función para dibujar el triángulo de la parte INTRO (necesito un VAO para hacerlo objeto de la clase y moverlo en el espacio)
void Triangulo(unsigned int* VAO) {
	unsigned int VBO;

	float vertices[] = {
		// Posiciones         // Colores
		 0.0f,  0.5f, 0.0f,    1.0f, 0.0f, 0.0f, // Vértice superior (rojo)
		-0.5f, -0.5f, 0.0f,    0.0f, 1.0f, 0.0f, // Vértice inferior izquierdo (verde)
		 0.5f, -0.5f, 0.0f,    0.0f, 0.0f, 1.0f  // Vértice inferior derecho (azul)
	};

	glGenVertexArrays(1, VAO);
	glGenBuffers(1, &VBO);

	glBindVertexArray(*VAO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	// Atributo posición
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	// Atributo color
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

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

void Cubo(unsigned int* VAO) {
	unsigned int VBO, EBO;

	float vertices[] = {
		-.5f, -0.5f, .5f,    1.0, 0.0, 0.0,    //0.0f, 0.0f, 1.0f,	0.0f, 0.0f,
		.5f, -0.5f, .5f,     1.0, 0.0, 0.0,    //0.0f, 0.0f, 1.0f,	1.0f, 0.0f,
		.5f, 0.5f, .5f,      1.0, 0.0, 0.0,    //0.0f, 0.0f, 1.0f,	1.0f, 1.0f,
		.5f, 0.5f, .5f,      1.0, 0.0, 0.0,    //0.0f, 0.0f, 1.0f,	1.0f, 1.0f,
		-.5f, 0.5f, .5f,     1.0, 0.0, 0.0,    //0.0f, 0.0f, 1.0f,	0.0f, 1.0f,
		-.5f, -0.5f, .5f,    1.0, 0.0, 0.0,    //0.0f, 0.0f, 1.0f,	0.0f, 0.0f,


		-.5f, -0.5f, -.5f,   1.0, 0.0, 0.0,    //0.0f, 0.0f, -1.0f,	0.0f, 0.0f,
		0.5f, 0.5f, -.5f,    1.0, 0.0, 0.0,    //0.0f, 0.0f, -1.0f,	1.0f, 0.0f,
		0.5f, -0.5f, -.5f,   1.0, 0.0, 0.0,    //0.0f, 0.0f, -1.0f,	1.0f, 1.0f,
		0.5f, 0.5f, -.5f,    1.0, 0.0, 0.0,    //0.0f, 0.0f, -1.0f,	1.0f, 1.0f,
		-.5f, -0.5f, -.5f,   1.0, 0.0, 0.0,    //0.0f, 0.0f, -1.0f,	0.0f, 1.0f,
		-.5f, 0.5f, -.5f,    1.0, 0.0, 0.0,    //0.0f, 0.0f, -1.0f,	0.0f, 0.0f,


		-.5f, 0.5f, -.5f,    1.0, 0.0, 0.0,    //-1.0f, 0.0f, 0.0f,	0.0f, 0.0f,
		-.5f, -.5f, .5f,     1.0, 0.0, 0.0,    //-1.0f, 0.0f, 0.0f,	1.0f, 0.0f,
		-0.5f, 0.5f, 0.5f,   1.0, 0.0, 0.0,    //-1.0f, 0.0f, 0.0f,	1.0f, 1.0f,
		-.5f, -.5f, .5f,     1.0, 0.0, 0.0,    //-1.0f, 0.0f, 0.0f,	1.0f, 1.0f,
		-.5f, 0.5f, -.5f,    1.0, 0.0, 0.0,    //-1.0f, 0.0f, 0.0f,	0.0f, 1.0f,
		-.5f, -.5f, -.5f,    1.0, 0.0, 0.0,    //-1.0f, 0.0f, 0.0f,	0.0f, 0.0f,


		.5f, 0.5f, -.5f,     1.0, 0.0, 0.0,    //1.0f, 0.0f, 0.0f,	0.0f, 0.0f,
		0.5f, 0.5f, .5f,     1.0, 0.0, 0.0,    //1.0f, 0.0f, 0.0f,	1.0f, 0.0f,
		0.5f, -0.5f, .5f,    1.0, 0.0, 0.0,    //1.0f, 0.0f, 0.0f,	1.0f, 1.0f,
		0.5f, -.5f, 0.5f,    1.0, 0.0, 0.0,    //1.0f, 0.0f, 0.0f,	1.0f, 1.0f,
		0.5f, -.5f, -.5f,    1.0, 0.0, 0.0,    //1.0f, 0.0f, 0.0f,	0.0f, 1.0f,
		0.5f, 0.5f, -.5f,    1.0, 0.0, 0.0,    //1.0f, 0.0f, 0.0f,	0.0f, 0.0f,


		-.5f, -.5f, .5f,     1.0, 0.0, 0.0,    //0.0f, -1.0f, 0.0f,	0.0f, 0.0f,
		0.5f, -.5f, -.5f,    1.0, 0.0, 0.0,    //0.0f, -1.0f, 0.0f,	1.0f, 0.0f,
		0.5f, -.5f, .5f,     1.0, 0.0, 0.0,    //0.0f, -1.0f, 0.0f,	1.0f, 1.0f,
		0.5f, -.5f, -.5f,    1.0, 0.0, 0.0,    //0.0f, -1.0f, 0.0f,	1.0f, 1.0f,
		-.5f, -.5f, .5f,     1.0, 0.0, 0.0,    //0.0f, -1.0f, 0.0f,	0.0f, 1.0f,
		-.5f, -.5f, -.5f,    1.0, 0.0, 0.0,    //0.0f, -1.0f, 0.0f,	0.0f, 0.0f,


		-.5f, 0.5f, .5f,     1.0, 0.0, 0.0,    //0.0f, 1.0f, 0.0f,	0.0f, 0.0f,
		0.5f, 0.5f, .5f,     1.0, 0.0, 0.0,    //0.0f, 1.0f, 0.0f,	1.0f, 0.0f,
		0.5f, 0.5f, -.5f,    1.0, 0.0, 0.0,    //0.0f, 1.0f, 0.0f,	1.0f, 1.0f,
		0.5f, 0.5f, -.5f,    1.0, 0.0, 0.0,    //0.0f, 1.0f, 0.0f,	1.0f, 1.0f,
		-.5f, 0.5f, -.5f,    1.0, 0.0, 0.0,    //0.0f, 1.0f, 0.0f,	0.0f, 1.0f,
		-.5f, 0.5f, .5f,     1.0, 0.0, 0.0,    //0.0f, 1.0f, 0.0f,	0.0f, 0.0f,
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

	// Atributo normal
	//glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void*)(6 * sizeof(float)));
	//glEnableVertexAttribArray(2);


	// Coordenadas de textura
	//glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void*)(9 * sizeof(float)));
	//glEnableVertexAttribArray(3);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
	glDeleteBuffers(1, &VBO);
}


//Inicializaciones de openGL
void openGlInit() {
	glClearDepth(1.0f);						//Ajustamos el buffer de profundidad
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);	//Ajustamos el color de fondo
	glEnable(GL_DEPTH_TEST);				//Activamos el test de profundidad, para procesar en 3D
	glEnable(GL_CULL_FACE);					//Activamos el culling, para eliminar caras no visibles
	glCullFace(GL_BACK);					//Eliminamos las caras traseras
	glLineWidth(1.0f);						//Ajustamos el grosor de las líneas		
}


//Función que nos permite que se actualice el tamaño cuando cambiamos el tamaño de la ventana
void window_size_callback(GLFWwindow* ventana, int nuevoAncho, int nuevoAlto) {
	//Actualiza las variables de ancho y alto de la ventana con los nuevos valores
	SCR_WIDTH = nuevoAncho;
	SCR_HEIGHT = nuevoAlto;

	//Ajusta el viewport para que coincida con el nuevo tamaño de la ventana
	glViewport(0, 0, nuevoAncho, nuevoAlto);
}

//Función para el control del programa
void processInput(GLFWwindow* window) {
	GLFWmonitor* monitor = glfwGetPrimaryMonitor();
	const GLFWvidmode* mode = glfwGetVideoMode(monitor);

	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
		// ESC para cerrar la ventana
		glfwSetWindowShouldClose(window, true);
	}
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		camPos += cameraSpeed * camFront; // Mover hacia adelante en la dirección de camFront
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		camPos -= cameraSpeed * camFront; // Mover hacia atrás en la dirección de camFront
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		camPos -= glm::normalize(glm::cross(camFront, camUp)) * cameraSpeed; // Mover hacia la izquierda
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		camPos += glm::normalize(glm::cross(camFront, camUp)) * cameraSpeed; // Mover hacia la derecha
	camPos.y = 5.0f;
	if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS) {
		if (!pWasPressed) {
			isFullscreen = !isFullscreen;

			// Guardar posición y tamaño actuales si se va a fullscreen
			if (isFullscreen) {
				glfwGetWindowPos(window, &windowedX, &windowedY);
				glfwGetWindowSize(window, &windowedWidth, &windowedHeight);
				glfwSetWindowMonitor(window, monitor, 0, 0,
					mode->width, mode->height,
					mode->refreshRate);
			}
			else {
				glfwSetWindowMonitor(window, nullptr,
					windowedX, windowedY,
					windowedWidth, windowedHeight,
					0);
			}
		}
		pWasPressed = true;
	}
	else {
		pWasPressed = false;
	}
}


//Función que configura la cámara
void myCamara() {
	// Matriz de proyección
	glm::mat4 projection = glm::perspective(glm::radians(fovCamara),
		(float)SCR_WIDTH / (float)SCR_HEIGHT,
		0.1f, 200.0f);
	unsigned int proyectionLoc = glGetUniformLocation(shaderProgram, "projection");
	glUniformMatrix4fv(proyectionLoc, 1, GL_FALSE, glm::value_ptr(projection));

	// Matriz de vista con cámara libre
	glm::mat4 view = glm::lookAt(camPos, camPos + camFront, camUp);
	unsigned int viewLoc = glGetUniformLocation(shaderProgram, "view");
	glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos) {
	if (firstMouse) {
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}

	float xoffset = xpos - lastX;
	float yoffset = lastY - ypos;
	lastX = xpos;
	lastY = ypos;

	xoffset *= sensitivity;
	yoffset *= sensitivity;

	yaw += xoffset;
	pitch += yoffset;

	if (pitch > 89.0f) pitch = 89.0f;
	if (pitch < -89.0f) pitch = -89.0f;

	glm::vec3 front;
	front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
	front.y = sin(glm::radians(pitch));
	front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
	camFront = glm::normalize(front);
}

// Función auxiliar para dibujar un suelo individual
void dibujarSuelo(Estructura& suelo, glm::vec3 posicion, unsigned int transformLoc, unsigned int colorLoc) {
	glm::mat4 transform = glm::mat4(1.0f);
	transform = glm::translate(transform, posicion);
	suelo.posicion = transform;
	transform = glm::scale(transform, glm::vec3(suelo.sx, suelo.sy, suelo.sz));
	suelo.dibujarObjeto(transformLoc, transform, colorLoc, TRIANGULOS, 6);
}

// Función principal para dibujar todos los suelos
void dibujarSuelos(unsigned int transformLoc, unsigned int colorLoc) {
	// Definimos los suelos y sus posiciones en un vector de pares
	struct InfoSuelo {
		Estructura& suelo;
		glm::vec3 posicion;
	};

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

	// Dibujamos cada suelo con su posición correspondiente
	for (const auto& info : suelos) {
		dibujarSuelo(info.suelo, info.posicion, transformLoc, colorLoc);
	}
}



void dibujarPared(glm::vec3 posicion, Estructura& pared, unsigned int transformLoc, unsigned int colorLoc) {
	glm::mat4 transform = glm::mat4(1.0f);
	transform = glm::translate(transform, posicion);
	transform = glm::scale(transform, glm::vec3(pared.sx, pared.sy, pared.sz));
	pared.dibujarObjeto(transformLoc, transform, colorLoc, TRIANGULOS, 6);
	// Giramos 180 grados y volvemos a dibujar
	transform = glm::rotate(transform, glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	pared.dibujarObjeto(transformLoc, transform, colorLoc, TRIANGULOS, 6);
}

void dibujarParedes(unsigned int transformLoc, unsigned int colorLoc) {
	// ParedXYTotal (centro)
	dibujarPared(glm::vec3(0.0f, ParedXYTotal.sy / 2, (SueloIntro.sz / 2) + 33.3f), ParedXYTotal, transformLoc, colorLoc);

	// Coordenadas de ParedXY alrededor del suelo
	float zBase = SueloIntro.sz / 2;
	std::vector<glm::vec3> posicionesXY = {
		{-33.f, ParedXY.sy / 2, zBase},
		{-33.f, ParedXY.sy / 2, -zBase},
		{-33.f, ParedXY.sy / 2, -zBase - 33.3f},
		{0.f,   ParedXY.sy / 2, -SueloIntro.sz * 2.5f},
		{+33.f, ParedXY.sy / 2, zBase},
		{+33.f, ParedXY.sy / 2, -zBase},
		{+33.f, ParedXY.sy / 2, -zBase - 33.3f}
	};
	for (const auto& pos : posicionesXY) {
		dibujarPared(pos, ParedXY, transformLoc, colorLoc);
	}

    // ParedYZ (laterales)  
    std::vector<glm::vec3> posicionesYZ = {  
		{-(SueloPasillo.sx / 2) - 33.15f, ParedYZ.sy / 2, SueloIntro.sz},  
		{-(SueloPasillo.sx / 2) - 33.15f, ParedYZ.sy / 2, 0.0f},  
		{-(SueloPasillo.sx / 2) - 33.15f, ParedYZ.sy / 2, -SueloTransformaciones.sz},  
		{-(SueloPasillo.sx / 2), ParedYZ.sy / 2, -(SueloCamara.sz * 2)},  
		{+(SueloPasillo.sx / 2), ParedYZ.sy / 2, -(SueloCamara.sz * 2)},  
		{+(SueloPasillo.sx / 2) + 33.15f, ParedYZ.sy / 2, SueloIntro.sz},  
		{+(SueloPasillo.sx / 2) + 33.15f, ParedYZ.sy / 2, 0.0f},  
		{+(SueloPasillo.sx / 2) + 33.15f, ParedYZ.sy / 2, -SueloTransformaciones.sz}  
    };  
    for (const auto& pos : posicionesYZ) {  
					dibujarPared(pos, ParedYZ, transformLoc, colorLoc);  
    }
	for (const auto& pos : posicionesYZ) {
		dibujarPared(pos, ParedYZ, transformLoc, colorLoc);
	}
}


// Dibuja la esfera en la habitación de modelado, usando como base SueloModelado
void dibujarEsfera(unsigned int transformLoc, unsigned int colorLoc, unsigned int useVertexColorLoc) {
	// Posición relativa de la esfera respecto al centro del suelo
	glm::vec3 posicionRelativa(0.0f, +5.0f, 0.0f);

	// Calcula la transformación: parte de la matriz de posición del suelo
	EsferaModelado.transform = glm::translate(SueloModelado.posicion, posicionRelativa);
	EsferaModelado.transform = glm::scale(EsferaModelado.transform, glm::vec3(EsferaModelado.sx, EsferaModelado.sy, EsferaModelado.sz));

	// Dibuja la esfera (usa color uniforme, no color por vértice)
	glUniform1i(useVertexColorLoc, false);
	EsferaModelado.dibujarObjeto(transformLoc, EsferaModelado.transform, colorLoc, GL_TRIANGLES, sphereVertexCount); // sphereVertexCount: número de vértices de la esfera
}


// Función para dibujar y mover el cubo aleatoriamente dentro de la habitación de transformaciones
void dibujarCubo(unsigned int transformLoc, unsigned int colorLoc, unsigned int useVertexColorLoc) {
	// Límites de la habitación de transformaciones
	float minX = -SueloTransformaciones.sx / 2.0f + CuboTransformaciones.sx / 2.0f;
	float maxX = SueloTransformaciones.sx / 2.0f - CuboTransformaciones.sx / 2.0f;
	float minZ = -SueloTransformaciones.sz / 2.0f + CuboTransformaciones.sz / 2.0f;
	float maxZ = SueloTransformaciones.sz / 2.0f - CuboTransformaciones.sz / 2.0f;
	float minY = 0.0f + CuboTransformaciones.sy / 2.0f;
	float maxY = ParedXY.sy - CuboTransformaciones.sy / 2.0f;

	// Variables estáticas para la posición y dirección del cubo
	static glm::vec3 pos = glm::vec3(0.0f, minY, 0.0f);
	static glm::vec3 dir = glm::normalize(glm::vec3(0.5f, 0.2f, 0.3f));
	static std::mt19937 rng(std::random_device{}());
	static std::uniform_real_distribution<float> dist(-1.0f, 1.0f);

	// Velocidad del cubo
	float velocidad = 0.2f;

	// Actualiza la posición
	pos += dir * velocidad;

	// Comprueba los límites y rebota o cambia dirección aleatoriamente
	bool rebote = false;
	if (pos.x < minX) { pos.x = minX; dir.x = -dir.x; rebote = true; }
	if (pos.x > maxX) { pos.x = maxX; dir.x = -dir.x; rebote = true; }
	if (pos.z < minZ) { pos.z = minZ; dir.z = -dir.z; rebote = true; }
	if (pos.z > maxZ) { pos.z = maxZ; dir.z = -dir.z; rebote = true; }
	if (pos.y < minY) { pos.y = minY; dir.y = -dir.y; rebote = true; }
	if (pos.y > maxY) { pos.y = maxY; dir.y = -dir.y; rebote = true; }

	// Si rebota, cambia ligeramente la dirección aleatoriamente
	if (rebote) {
		dir.x += dist(rng) * 0.2f;
		dir.y += dist(rng) * 0.2f;
		dir.z += dist(rng) * 0.2f;
		dir = glm::normalize(dir);
	}

	// Calcula la transformación del cubo
	CuboTransformaciones.transform = glm::translate(SueloTransformaciones.posicion, glm::vec3(pos.x, pos.y, pos.z));
	CuboTransformaciones.transform = glm::rotate(CuboTransformaciones.transform, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	CuboTransformaciones.transform = glm::scale(CuboTransformaciones.transform, glm::vec3(CuboTransformaciones.sx, CuboTransformaciones.sy, CuboTransformaciones.sz));

	// Dibuja el cubo usando color por vértice
	glUniform1i(useVertexColorLoc, true);
	CuboTransformaciones.dibujarObjeto(transformLoc, CuboTransformaciones.transform, colorLoc, GL_TRIANGLES, 36);
	glUniform1i(useVertexColorLoc, false);
}


void Display() {
	//Usamos el shader
	glUseProgram(shaderProgram);
	//Buscamos en el shader la matriz de transformación y el color.
	unsigned int transformLoc = glGetUniformLocation(shaderProgram, "model");
	unsigned int colorLoc = glGetUniformLocation(shaderProgram, "ourColor");
	unsigned int useVertexColorLoc = glGetUniformLocation(shaderProgram, "useVertexColor");

	//Limpiamos el buffer de color y el de profundidad
	glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	//Configuramos la cámara
	myCamara();
	
	//Dibujamos el suelo
	dibujarSuelos(transformLoc, colorLoc);

	//Dibujamos las paredes
	dibujarParedes(transformLoc, colorLoc);

	//Dibujamos el triángulo de la parte INTRO
	glUniform1i(useVertexColorLoc, true); //Activamos que queremos usar el color del VAO
    TrianguloIntro.transform = glm::translate(SueloIntro.posicion, glm::vec3(TrianguloIntro.px, TrianguloIntro.py + 5, TrianguloIntro.pz + 5));  
    TrianguloIntro.transform = glm::rotate(TrianguloIntro.transform, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));  
    TrianguloIntro.transform = glm::scale(TrianguloIntro.transform, glm::vec3(TrianguloIntro.sx, TrianguloIntro.sy, TrianguloIntro.sz));  
    TrianguloIntro.dibujarObjeto(transformLoc, TrianguloIntro.transform, colorLoc, GL_TRIANGLES, 3);
	//Desactivamos el uso del color del VAO
	glUniform1i(useVertexColorLoc, false); 

	//Dibujamos la esfera
	dibujarEsfera(transformLoc, colorLoc, useVertexColorLoc);

	//Dibujamos el cubo
	dibujarCubo(transformLoc, colorLoc, useVertexColorLoc);

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}

int main() {
	//Devlaramos las variables
	unsigned int VAOSuelo, VAOPared, VAOTriangulo, VAOEsfera, VAOCubo;

	//Inicializamos GLFW
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	//Creo la ventana
	GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Grua", NULL, NULL);
	if (window == NULL) {
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}

	//Establezco la ventana como contexto
	glfwMakeContextCurrent(window);
	//Establezco la función para el cambio de tamaño de la ventana
	glfwSetWindowSizeCallback(window, window_size_callback);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED); // Para ocultar cursor

	//Inicializo GLAD
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}

	openGlInit();

	//Cargamos los shaders
	shaderProgram = setShaders("shader.vert", "shader.frag");

	//Preparamos los VAO's
	CuadradoXZ(&VAOSuelo);
	SueloPasillo.VAO = SueloIntro.VAO = SueloModelado.VAO = SueloTransformaciones.VAO = SueloCamara.VAO = SueloIluminacion.VAO = SueloTexturas.VAO = SueloColisiones.VAO = VAOSuelo;

	CuadradoXY(&VAOPared);
	ParedXY.VAO = ParedXYTotal.VAO = VAOPared;

	CuadradoYZ(&VAOPared);
	ParedYZ.VAO = VAOPared;

	Triangulo(&VAOTriangulo);
	TrianguloIntro.VAO = VAOTriangulo;

	Esfera(&VAOEsfera);
	EsferaModelado.VAO = VAOEsfera;

	Cubo(&VAOCubo);
	CuboTransformaciones.VAO = VAOCubo;
	

	//Lazo de la ventana mientras no la cierre
	while (!glfwWindowShouldClose(window)) {

		//Función para leer los inputs del teclado
		processInput(window);

		//Función que se encargará de dibujar los objetos
		Display();

		//Swap buffers y poll events
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	//Liberamos los recursos
	//glDeleteVertexArrays(1, &VAO);
	glfwTerminate();
	return 0;
}