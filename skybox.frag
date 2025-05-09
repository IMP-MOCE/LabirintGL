#version 330 core
out vec4 FragColor;

in vec3 TexCoords;

uniform samplerCube skybox;
uniform bool ActiveCubeMap;

void main()
{    
    if (ActiveCubeMap) {
    FragColor = texture(skybox, TexCoords);
    }
    else {
    FragColor = vec4(0.0);
    }
}