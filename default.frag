#version 330 core
out vec4 FragColor;

in vec3 color;
in vec2 texCoord;

in vec3 normal;
in vec3 crntPos;

uniform sampler2D tex1;
uniform sampler2D specmap;

uniform vec4 lightColor;
uniform vec3 lightPos;
uniform vec3 camPos;

void main()
{
	float ambient = 0.20f;

	vec3 normal1 = normalize(normal);
	vec3 lightDirection = normalize(lightPos - crntPos);

	float diffuse = max(dot(normal, lightDirection), 0.0f);

	float specularLight = 0.50f;
	vec3 viewDirection = normalize(camPos - crntPos);
	vec3 reflectionDirection = reflect(-lightDirection, normal);
	float specularAmount = pow(max(dot(viewDirection, reflectionDirection), 0.0f), 8);
	float specular = specularAmount * specularLight;

    FragColor = texture(tex1, texCoord) * lightColor * (diffuse + ambient) * texture(specmap, texCoord) + specular;
	//FragColor = texture(tex1, texCoord) * lightColor * (diffuse + ambient + specular);
}
