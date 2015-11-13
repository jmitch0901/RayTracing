#version 400 core



layout(location=0) in vec4 vPosition;
layout(location=1) in vec4 vNormal;
uniform mat4 projection;
uniform mat4 modelview;
uniform mat4 normalmatrix;
out vec4 fColor;

struct MaterialProperties
{
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    float shininess;
};

struct LightProperties
{
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    vec4 position;
};

const int MAXLIGHTS = 10;

uniform MaterialProperties material;
uniform LightProperties light[MAXLIGHTS];
uniform int numLights;

void main()
{
    vec3 lightVec,viewVec,reflectVec;
    vec3 normalView;
    vec3 ambient,diffuse,specular;
    float nDotL,rDotV;
	vec3 fNormal;
	vec4 fPosition;

    fPosition = modelview * vec4(vPosition.xyzw);
    gl_Position = projection * fPosition;


    vec4 tNormal = normalmatrix * vNormal;
    fNormal = normalize(tNormal.xyz);

	fColor = vec4(0,0,0,1);
	normalView = normalize(fNormal.xyz);
    viewVec = -fPosition.xyz;
    viewVec = normalize(viewVec);

    for (int i=0;i<numLights;i++)
    {

        if (light[i].position.w!=0)
            lightVec = normalize(light[i].position.xyz - fPosition.xyz);
        else
            lightVec = normalize(-light[i].position.xyz);

        
        nDotL = dot(normalView,lightVec);


        reflectVec = reflect(-lightVec,normalView);
        reflectVec = normalize(reflectVec);

        rDotV = max(dot(reflectVec,viewVec),0.0);

        ambient = material.ambient * light[i].ambient;
        diffuse = material.diffuse * light[i].diffuse * max(nDotL,0);
        if (nDotL>0)
            specular = material.specular * light[i].specular * pow(rDotV,material.shininess);
        else
            specular = vec3(0,0,0);
        fColor = fColor + vec4(ambient+diffuse+specular,1.0);
    }
}

