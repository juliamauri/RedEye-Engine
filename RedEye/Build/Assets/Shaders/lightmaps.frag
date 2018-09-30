#version 330 core
out vec4 FragColor;

struct Material {
    sampler2D diffuse;
	sampler2D specular;
    float     shininess;
}; 

struct Light {
	//Spotlight
    vec3  SPosition;
    vec3  SDirection;
	float cutOff;
	float outerCutOff;
	
	// .w == 0, direction | .w == 1, position
    vec4 direction;  
  
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
	
    float constant;
    float linear;
    float quadratic;
}; 
  
uniform Material material;
uniform Light light;
uniform vec3 viewPos;
uniform bool spotlight;

in vec2 TexCoords;
in vec3 FragPos;  
in vec3 Normal;  

void main()
{	
if(spotlight)
{
    // ambient
    vec3 ambient = light.ambient * texture(material.diffuse, TexCoords).rgb;
    
    // diffuse 
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(light.SPosition - FragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = light.diffuse * diff * texture(material.diffuse, TexCoords).rgb;  
    
    // specular
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);  
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    vec3 specular = light.specular * spec * texture(material.specular, TexCoords).rgb;  
    
    // spotlight (soft edges)
    float theta = dot(lightDir, normalize(-light.SDirection)); 
    float epsilon = (light.cutOff - light.outerCutOff);
    float intensity = clamp((theta - light.outerCutOff) / epsilon, 0.0, 1.0);
    diffuse  *= intensity;
    specular *= intensity;
    
    // attenuation
    float distance    = length(light.SPosition - FragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));    
    ambient  *= attenuation; 
    diffuse   *= attenuation;
    specular *= attenuation;   
        
    vec3 result = ambient + diffuse + specular;
    FragColor = vec4(result, 1.0);
}
else{
    // ambient
    vec3 ambient = light.ambient * texture(material.diffuse, TexCoords).rgb;

	
	// diffuse 
    vec3 norm = normalize(Normal);
	
	vec3 lightDirValues;
		lightDirValues.x = light.direction.x;
		lightDirValues.y = light.direction.y;
		lightDirValues.z = light.direction.z;
	vec3 lightDir = vec3(0.0f);
	if(light.direction.w == 0.0)
		lightDir = normalize(-lightDirValues);
	else if(light.direction.w == 1.0)
		lightDir = normalize(lightDirValues - FragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = light.diffuse * diff * texture(material.diffuse, TexCoords).rgb;   
	
    // specular
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);  
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    vec3 specular = light.specular * spec * texture(material.specular, TexCoords).rgb;  
	
	//attenuation
	if(light.direction.w == 1.0)
	{
		float distance    = length(lightDirValues - FragPos);
		float attenuation = 1.0 / (light.constant + light.linear * distance + 
						light.quadratic * (distance * distance)); 
		ambient  *= attenuation; 
		diffuse  *= attenuation;
		specular *= attenuation;   
	}
	
	FragColor = vec4(ambient + diffuse + specular, 1.0);   
	}
}