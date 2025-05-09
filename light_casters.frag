#version 330 core
out vec4 FragColor;

in VS_OUT {
    vec3 FragPos;
    vec2 TexCoords;
    vec3 TangentLightPos;
    vec3 TangentViewPos;
    vec3 TangentFragPos;
    vec3 TangentLightDir; 
    vec3 TangentPointLightPos[5]; 
} fs_in;

uniform sampler2D diffuseMap;
uniform sampler2D specularMap;
uniform sampler2D normalMap;

struct PointLight {
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    float constant;
    float linear;
    float quadratic;
};

struct SpotLight {
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    float constant;
    float linear;
    float quadratic;
    float cutOff;
    float outerCutOff;
    bool state;
};

uniform PointLight pointLights[5]; 
uniform SpotLight spotLight;

uniform sampler2D shadowMap;
uniform mat4 lightSpaceMatrix;

float ShadowCalculation(vec4 fragPosLightSpace, vec3 normal, vec3 lightDir) {
    
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    projCoords = projCoords * 0.5 + 0.5;

    float closestDepth = texture(shadowMap, projCoords.xy).r;

    float currentDepth = projCoords.z;

    float bias = max(0.05 * (1.0 - dot(normal, lightDir)), 0.005);
    float shadow = currentDepth - bias > closestDepth ? 1.0 : 0.0;

    return shadow;
}

vec3 calculatePointLight(PointLight light, vec3 lightPos, vec3 normal, vec3 fragPos, vec3 viewDir) {
    vec3 lightDir = normalize(lightPos - fragPos);
    float diff = max(dot(normal, lightDir), 0.0);
    vec3 diffuse = diff * texture(diffuseMap, fs_in.TexCoords).rgb * light.diffuse;
    vec3 reflectDir = reflect(-lightDir, normal);
    vec3 halfwayDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(normal, halfwayDir), 0.0), 32.0);
    vec3 specular = spec * texture(specularMap, fs_in.TexCoords).rgb * light.specular;
    float distance = length(lightPos - fragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));
    diffuse *= attenuation;
    specular *= attenuation;
    return (diffuse + specular);
}

vec3 calculateSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir, vec3 lightDir) {
    float theta = dot(lightDir, normalize(-fs_in.TangentLightDir));
    float epsilon = light.cutOff - light.outerCutOff;
    float intensity = clamp((theta - light.outerCutOff) / epsilon, 0.0, 1.0);

    float diff = max(dot(normal, lightDir), 0.0);
    vec3 diffuse = diff * texture(diffuseMap, fs_in.TexCoords).rgb * light.diffuse * intensity;

    vec3 reflectDir = reflect(-lightDir, normal);
    vec3 halfwayDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(normal, halfwayDir), 0.0), 32.0);
    vec3 specular = spec * texture(specularMap, fs_in.TexCoords).rgb * light.specular * intensity;

    float distance = length(fs_in.TangentLightPos - fragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));
    diffuse *= attenuation;
    specular *= attenuation;

    return (diffuse + specular);
}

void main() {

    vec3 viewDir = normalize(fs_in.TangentViewPos - fs_in.TangentFragPos);

    vec3 normal = texture(normalMap, fs_in.TexCoords).rgb;
    normal = normalize(normal * 2.0 - 1.0);

    vec3 color = texture(diffuseMap, fs_in.TexCoords).rgb;
    vec3 ambient = 0.01 * color;

    vec3 result = ambient;

    for (int i = 0; i < 5; i++) {
        result += calculatePointLight(pointLights[i], fs_in.TangentPointLightPos[i], normal, fs_in.TangentFragPos, viewDir);
    }

    vec3 lightDir = normalize(fs_in.TangentLightPos - fs_in.TangentFragPos);
    if (spotLight.state == true){
        vec4 fragPosLightSpace = lightSpaceMatrix * vec4(fs_in.FragPos, 1.0);

        float shadow = ShadowCalculation(fragPosLightSpace, normal, lightDir);

        vec3 spotLightResult = calculateSpotLight(spotLight, normal, fs_in.TangentFragPos, viewDir, lightDir);
        result += spotLightResult * (1.0 - shadow);
    }

    FragColor = vec4(result, 1.0);
}