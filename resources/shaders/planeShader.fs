#version 330 core
out vec4 FragColor;

struct DirLight {
    vec3 direction;
	
    vec3 ambient;
    vec3 diffuse;
};

struct SpotLight {
    vec3 position;
    vec3 direction;
    float cutOff;
    float outerCutOff;
  
    float constant;
    float linear;
    float quadratic;
  
    vec3 ambient;
    vec3 diffuse;     
};

uniform sampler2D texture_diffuse1;

uniform samplerCube depthMap;

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoords;

uniform vec3 viewPos;
uniform DirLight dirLight;
uniform SpotLight spotLight;

uniform vec3 lightPos;
uniform float far_plane;
uniform bool shadows;

// function prototypes
vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir, float shadow);
vec3 CalcSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir, float shadow);

float ShadowCalculation(vec3 fragPos)
{
    // get vector between fragment position and light position
    vec3 fragToLight = fragPos - lightPos;
    // ise the fragment to light vector to sample from the depth map    
    float closestDepth = texture(depthMap, fragToLight).r;
    // it is currently in linear range between [0,1], let's re-transform it back to original depth value
    closestDepth *= far_plane;
    // now get current linear depth as the length between the fragment and light position
    float currentDepth = length(fragToLight);
    // test for shadows
    float bias = 0.05; // we use a much larger bias since depth is now in [near_plane, far_plane] range
    float shadow = currentDepth -  bias > closestDepth ? 1.0 : 0.0;        
    // display closestDepth as debug (to visualize depth cubemap)
    // FragColor = vec4(vec3(closestDepth / far_plane), 1.0);    
        
    return shadow;
}

void main()
{    
    float shadow = shadows ? ShadowCalculation(FragPos) : 0.0;
    // properties
    vec3 norm = normalize(Normal);
    vec3 viewDir = normalize(viewPos - FragPos);
    
    //directional lighting
    vec3 result = CalcDirLight(dirLight, norm, viewDir, shadow);
 
    //spot light
    result += CalcSpotLight(spotLight, norm, FragPos, viewDir, shadow);    
    
    FragColor = vec4(result, 1.0);
   
}

// calculates the color when using a directional light.
vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir, float shadow)
{
    vec3 lightDir = normalize(-light.direction);
    // diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);

    // combine results
    vec3 ambient = light.ambient * vec3(texture(texture_diffuse1, TexCoords));
    vec3 diffuse = light.diffuse * diff * vec3(texture(texture_diffuse1, TexCoords)) * (1.0 - shadow);
    return (ambient + diffuse);
}

// calculates the color when using a spot light.
vec3 CalcSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir, float shadow)
{
    vec3 lightDir = normalize(light.position - fragPos);
    // diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);
    // attenuation
    float distance = length(light.position - fragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));    
    // spotlight intensity
    float theta = dot(lightDir, normalize(-light.direction)); 
    float epsilon = light.cutOff - light.outerCutOff;
    float intensity = clamp((theta - light.outerCutOff) / epsilon, 0.0, 1.0);
    // combine results
    vec3 ambient = light.ambient * vec3(texture(texture_diffuse1, TexCoords));
    vec3 diffuse = light.diffuse * diff * vec3(texture(texture_diffuse1, TexCoords));
    ambient *= attenuation * intensity;
    diffuse *= attenuation * intensity * (1-shadow);
    return (ambient + diffuse);
}