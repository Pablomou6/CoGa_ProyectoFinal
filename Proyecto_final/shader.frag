#version 330 core
out vec4 FragColor;

uniform vec3 ourColor;

void main()
{
    //Asignamos el color seg�n la variable ourColor
    FragColor = vec4(ourColor, 1.0);
}