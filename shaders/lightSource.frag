#version 410 core

out vec4 fColor;
uniform vec3 lightColor;

in vec2 fTexCoords;

void main() 
{    
    fColor = vec4(lightColor, 1.0f);
}
