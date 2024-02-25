#version 410 core

in vec3 fNormal;
in vec4 fPosEye;
in vec2 fTexCoords;
in vec4 fragPosLightSpace;
in vec4 lanternEye;

out vec4 fColor;

//lighting
uniform	vec3 lightDir;
uniform	vec3 lightColor;
//uniform vec3 lanternPosition;
uniform vec3 lanternColor;

//texture
uniform sampler2D diffuseTexture;
uniform sampler2D specularTexture;
uniform sampler2D shadowMap;

// fog
uniform int fogEnable;
uniform float fogDensity;

vec3 ambient;
vec3 ambientLantern;
float ambientStrength = 0.2f;
float ambientStrengthLantern=0.007f;
vec3 diffuse;
vec3 diffuseLantern;
vec3 specular;
vec3 specularLantern;
float specularStrength = 0.5f;
float specularStrengthLantern = 0.01f;
float shininess = 32.0f;
float shadow;

float constant = 1.0f;
float linear = 0.001f;
float quadratic = 0.9f;

void computeLightComponents()
{		
	  vec3 cameraPosEye = vec3(0.0f); // in eye coordinates, the viewer is situated at the origin
    
    // transform normal
    vec3 normalEye = normalize(fNormal);    
    
    // compute light direction
    vec3 lightDirN = normalize(lightDir);
    
    // compute view direction 
    vec3 viewDirN = normalize(cameraPosEye - fPosEye.xyz);

    //compute distance to light
    float dist = length(lanternEye.xyz - fPosEye.xyz);
    //compute attenuation
    float att = 1.0f / (constant + linear * dist + quadratic * (dist * dist));
        
    // compute ambient light
    ambient = ambientStrength * lightColor;
    
    // compute diffuse light
    diffuse = max(dot(normalEye, lightDirN), 0.0f) * lightColor;

    // compute diffuse light from lantern
    vec3 lanternDir = normalize(lanternEye.xyz - fPosEye.xyz);
    float lanternDiffuse = max(dot(normalEye, lanternDir), 0.0f);
    diffuseLantern = att*lanternDiffuse * lanternColor;
    ambientLantern = att*ambientStrengthLantern * lanternColor;


    // compute specular light
    vec3 reflection = reflect(-lightDirN, normalEye);
    float specCoeff = pow(max(dot(viewDirN, reflection), 0.0f), shininess);
    float specCoeffLantern= pow(max(dot(normalEye, reflection), 0.0f), shininess);
    specular = specularStrength * specCoeff * lightColor;
    specularLantern = att*specularStrengthLantern * specCoeffLantern * lanternColor;
}
float computeShadow() {
	vec3 normalizedCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
	if(normalizedCoords.z > 1.0f)
        return 0.0f;
	normalizedCoords = normalizedCoords * 0.5 + 0.5;
	float closestDepth = texture(shadowMap, normalizedCoords.xy).r;
	float currentDepth = normalizedCoords.z;
	//float shadow = currentDepth > closestDepth ? 1.0 : 0.0;
 
	float bias=0.005f;
        //float bias = max(0.05 * (1.0 - dot(normal, lightDir)), 0.005);
	float shadow =currentDepth-bias>closestDepth ? 1.0f : 0.0f;
 
	return shadow;
 
}
float computeFog()
{
    float fragmentDistance = length(fPosEye.xyz);
    float fogFactor = exp(-pow(fragmentDistance * fogDensity, 2));

    return clamp(fogFactor, 0.0f, 1.0f);
}
void main() 
{
	computeLightComponents();
	
	//vec3 baseColor = vec3(0.9f, 0.35f, 0.0f); // orange
          vec3 baseColor = vec3(1.0f, 1.0f, 1.0f);
	
	ambient *= texture(diffuseTexture, fTexCoords).rgb;
	diffuse *= texture(diffuseTexture, fTexCoords).rgb;
	specular *= texture(specularTexture, fTexCoords).rgb;

        ambientLantern *= baseColor;
	diffuseLantern *= baseColor;
	specularLantern *= baseColor;

	// modulate with shadow
	shadow = computeShadow();
	vec3 color = min(((ambient+ambientLantern) + (1.0f - shadow) * (diffuse+diffuseLantern)) + (1.0f - shadow) * (specular+specularLantern), 1.0f);
    
    // compute final vertex color
    float fogFactor = computeFog(); 
    vec4 fogColor = vec4(0.5f, 0.5f, 0.5f, 1.0f);

    if (fogEnable == 1) {
        fColor = mix(fogColor, vec4(color, 1.0f), fogFactor);
    } else {
        fColor = vec4(color, 1.0f);
    }
}


