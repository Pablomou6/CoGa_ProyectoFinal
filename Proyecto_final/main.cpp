#include <iostream>
#include <stdio.h>

#include "glad.h"  
#include <glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "lecturaShader_0_9.h"
#include "esfera.h"

//Declaramos algunas constantes
#define TRIANGULOS GL_TRIANGLES
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
	float angulo_trans;      // ángulo de giro
	float velocidad;         // velocidad de movimiento
	float sx, sy, sz;        // escalado
	unsigned int VAO;        // Vertex Array Object
	int texture;             // textura del objeto
	glm::mat4 transform;     // matriz de transformación
	glm::vec3 color;         // color del objeto

	//Constructor por defecto
	Estructura() {
		px = 0;
		py = 0;
		pz = 0;
		angulo_trans = 0;
		velocidad = 0;
		sx = 1;
		sy = 1;
		sz = 1;
		VAO = 0;
		texture = 0;
		transform = glm::mat4(1.0f);
		color = glm::vec3(1.0f);
	}

	//Constructor con parámetros. En caso de no recibir textura, se le asigna 0, pero permite cargarla
	Estructura(float px, float py, float pz, float angulo_trans, float velocidad,
		float sx, float sy, float sz, unsigned int VAO, glm::vec3 color = glm::vec3(1.0f),
		GLuint texture = 0) {
		this->px = px;
		this->py = py;
		this->pz = pz;
		this->angulo_trans = angulo_trans;
		this->velocidad = velocidad;
		this->sx = sx;
		this->sy = sy;
		this->sz = sz;
		this->VAO = VAO;
		this->color = color;
		this->texture = texture;
		this->transform = glm::mat4(1.0f);
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

//Declaramos los suelos de forma global
Estructura SueloPasillo(0, 0, 0, 0.0f, 0.0f, 33.0f, 1.0f, 100.0f, 0, glm::vec3(1, 1, 1));

Estructura SueloIntro(0, 0, 0, 0.0f, 0.0f, 33.3f, 1.0f, 33.3f, 0, glm::vec3(.1f, .1f, .1f));
Estructura SueloModelado(0, 0, 0, 0.0f, 0.0f, 33.3f, 1.0f, 33.3f, 0, glm::vec3(.2f, .2f, .2f));
Estructura SueloTransformaciones(0, 0, 0, 0.0f, 0.0f, 33.3f, 1.0f, 33.3f, 0, glm::vec3(.3f, .3f, .3f));

Estructura SueloCamara(0, 0, 0, 0.0f, 0.0f, 33.3f, 1.0f, 33.3f, 0, glm::vec3(.4f, .4f, .4f));

Estructura SueloIluminacion(0, 0, 0, 0.0f, 0.0f, 33.3f, 1.0f, 33.3f, 0, glm::vec3(.5f, .5f, .5f));
Estructura SueloTexturas(0, 0, 0, 0.0f, 0.0f, 33.3f, 1.0f, 33.3f, 0, glm::vec3(.6f, .6f, .6f));
Estructura SueloColisiones(0, 0, 0, 0.0f, 0.0f, 33.3f, 1.0f, 33.3f, 0, glm::vec3(.7f, .7f, .7f));

Estructura ParedIntroModelado = Estructura(.0f, 0.0f, .0f, 0.0f, 0.0f, 33.3f, 5.0f, 1.0f, 0, glm::vec3(0.5f, 0.5f, 0.5f));
Estructura ParedModeladoTransform = Estructura(.0f, 0.0f, .0f, 0.0f, 0.0f, 33.3f, 5.0f, 1.0f, 0, glm::vec3(0.5f, 0.5f, 0.5f));
Estructura ParedTransformIluminacion = Estructura(0.0f, 0.0f, -50.0f, 0.0f, 0.0f, 1.0f, 5.0f, 33.3f, 0, glm::vec3(0.5f, 0.5f, 0.5f));
Estructura ParedIluminacionTexturas = Estructura(16.65f, 0.0f, -33.3f, 0.0f, 0.0f, 1.0f, 5.0f, 33.3f, 0, glm::vec3(0.5f, 0.5f, 0.5f));


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

void ParedVAO(unsigned int* VAOPared) {
	unsigned int VBO;

	// Vértices de una pared (rectángulo vertical en el eje YZ)
	float vertices[] = {
		//     Posición          Color
		-0.5f, 0.0f,  0.0f,     0.5f, 0.5f, 0.5f,  // Inferior izquierda
		 0.5f, 0.0f,  0.0f,     0.5f, 0.5f, 0.5f,  // Inferior derecha
		 0.5f, 5.0f,  0.0f,     0.5f, 0.5f, 0.5f,  // Superior derecha
		-0.5f, 5.0f,  0.0f,     0.5f, 0.5f, 0.5f,  // Superior izquierda
	};

	unsigned int indices[] = {
		0, 1, 2,  // Triángulo inferior
		2, 3, 0   // Triángulo superior
	};

	glGenVertexArrays(1, VAOPared);
	glGenBuffers(1, &VBO);
	unsigned int EBO;
	glGenBuffers(1, &EBO);

	glBindVertexArray(*VAOPared);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	// Posición
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	// Color
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
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

void dibujarSuelos(unsigned int transformLoc, unsigned int colorLoc) {
	//Dibujamos el Suelo del pasillo con las transformaciones necesarias
	SueloPasillo.transform = glm::mat4(1.0f);
	SueloPasillo.transform = glm::translate(SueloPasillo.transform, glm::vec3(0.0f, 0.0f, 0.0f)); // Mantener en el centro
	SueloPasillo.transform = glm::scale(SueloPasillo.transform, glm::vec3(SueloPasillo.sx, SueloPasillo.sy, SueloPasillo.sz));
	SueloPasillo.dibujarObjeto(transformLoc, SueloPasillo.transform, colorLoc, TRIANGULOS, 6);

	//Dibujamos el Suelo de la intro con las transformaciones necesarias
	SueloIntro.transform = glm::mat4(1.0f);
	SueloIntro.transform = glm::translate(SueloIntro.transform, glm::vec3(-33.f, 0.0f, 33.3f));
	SueloIntro.transform = glm::scale(SueloIntro.transform, glm::vec3(SueloIntro.sx, SueloIntro.sy, SueloIntro.sz));
	SueloIntro.dibujarObjeto(transformLoc, SueloIntro.transform, colorLoc, TRIANGULOS, 6);

	//Dibujamos el Suelo de los modelados con las transformaciones necesarias
	SueloModelado.transform = glm::mat4(1.0f);
	SueloModelado.transform = glm::translate(SueloModelado.transform, glm::vec3(-33.f, 0.0f, 0.0f));
	SueloModelado.transform = glm::scale(SueloModelado.transform, glm::vec3(SueloModelado.sx, SueloModelado.sy, SueloModelado.sz));
	SueloModelado.dibujarObjeto(transformLoc, SueloModelado.transform, colorLoc, TRIANGULOS, 6);

	//Dibujamos el Suelo de las transformaciones con las transformaciones necesarias
	SueloTransformaciones.transform = glm::mat4(1.0f);
	SueloTransformaciones.transform = glm::translate(SueloTransformaciones.transform, glm::vec3(-33.f, 0.0f, -33.3f));
	SueloTransformaciones.transform = glm::scale(SueloTransformaciones.transform, glm::vec3(SueloTransformaciones.sx, SueloTransformaciones.sy, SueloTransformaciones.sz));
	SueloTransformaciones.dibujarObjeto(transformLoc, SueloTransformaciones.transform, colorLoc, TRIANGULOS, 6);

	//Dibujamos el Suelo de la cámara con las transformaciones necesarias
	SueloCamara.transform = glm::mat4(1.0f);
	SueloCamara.transform = glm::translate(SueloCamara.transform, glm::vec3(0.0f, 0.0f, -66.6f));
	SueloCamara.transform = glm::scale(SueloCamara.transform, glm::vec3(SueloCamara.sx, SueloCamara.sy, SueloCamara.sz));
	SueloCamara.dibujarObjeto(transformLoc, SueloCamara.transform, colorLoc, TRIANGULOS, 6);

	//Dibujamos el Suelo de la iluminación con las transformaciones necesarias
	SueloIluminacion.transform = glm::mat4(1.0f);
	SueloIluminacion.transform = glm::translate(SueloIluminacion.transform, glm::vec3(33.f, 0.0f, -33.3f));
	SueloIluminacion.transform = glm::scale(SueloIluminacion.transform, glm::vec3(SueloIluminacion.sx, SueloIluminacion.sy, SueloIluminacion.sz));
	SueloIluminacion.dibujarObjeto(transformLoc, SueloIluminacion.transform, colorLoc, TRIANGULOS, 6);

	//Dibujamos el Suelo de las texturas con las transformaciones necesarias
	SueloTexturas.transform = glm::mat4(1.0f);
	SueloTexturas.transform = glm::translate(SueloTexturas.transform, glm::vec3(33.f, 0.0f, 0.0f));
	SueloTexturas.transform = glm::scale(SueloTexturas.transform, glm::vec3(SueloTexturas.sx, SueloTexturas.sy, SueloTexturas.sz));
	SueloTexturas.dibujarObjeto(transformLoc, SueloTexturas.transform, colorLoc, TRIANGULOS, 6);

	//Dibujamos el Suelo de las colisiones con las transformaciones necesarias
	SueloColisiones.transform = glm::mat4(1.0f);
	SueloColisiones.transform = glm::translate(SueloColisiones.transform, glm::vec3(33.f, 0.0f, 33.3f));
	SueloColisiones.transform = glm::scale(SueloColisiones.transform, glm::vec3(SueloColisiones.sx, SueloColisiones.sy, SueloColisiones.sz));
	SueloColisiones.dibujarObjeto(transformLoc, SueloColisiones.transform, colorLoc, TRIANGULOS, 6);
}

void Display() {
	//Usamos el shader
	glUseProgram(shaderProgram);
	//Buscamos en el shader la matriz de transformación y el color.
	unsigned int transformLoc = glGetUniformLocation(shaderProgram, "model");
	unsigned int colorLoc = glGetUniformLocation(shaderProgram, "ourColor");

	//Limpiamos el buffer de color y el de profundidad
	glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	//Configuramos la cámara
	myCamara();
	
	//Dibujamos el suelo
	dibujarSuelos(transformLoc, colorLoc);

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}

int main() {
	//Devlaramos las variables
	unsigned int VAOSuelo, VAOPared;

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

	ParedVAO(&VAOPared);
	ParedIntroModelado.VAO = ParedModeladoTransform.VAO = ParedTransformIluminacion.VAO = ParedIluminacionTexturas.VAO = VAOPared;

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