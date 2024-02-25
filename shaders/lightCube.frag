#version 410 core


in vec4 fragPosEye;
in vec3 fPosition;
in vec3 fNormal;
in vec2 fTexCoords;
in vec3 fragPos;

out vec4 fColor;

//matrices
uniform mat4 model;
uniform mat4 view;
uniform mat3 normalMatrix;

// textures
uniform sampler2D diffuseTexture;
uniform sampler2D specularTexture;

void main() 
{    
    fColor = vec4(texture(diffuseTexture, fTexCoords).rgb, 0.0f);
}
