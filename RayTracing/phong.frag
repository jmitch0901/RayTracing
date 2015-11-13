#version 400 core

struct MaterialProperties
{
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    float shininess;
};

in vec3 fNormal;
in vec4 fPosition;

uniform MaterialProperties material;
uniform vec3 light_ambient;
uniform vec3 light_diffuse;
uniform vec3 light_specular;
uniform vec4 light_position;
out vec4 fColor;

void main()
{
    vec3 lightVec,viewVec,reflectVec;
    vec3 normalView;
    vec3 ambient,diffuse,specular;
    float nDotL,rDotV;


    if (light_position.w!=0)
        lightVec = normalize(light_position.xyz - fPosition.xyz);
    else
        lightVec = normalize(-light_position.xyz);

    vec3 tNormal = fNormal;
    normalView = normalize(tNormal.xyz);
    nDotL = max(dot(normalView,lightVec),0.0);

    viewVec = -fPosition.xyz;
    viewVec = normalize(viewVec);

    reflectVec = reflect(-lightVec,normalView);
    reflectVec = normalize(reflectVec);

    rDotV = max(dot(reflectVec,viewVec),0.0);

    /*outColor = vZColor; */
    ambient = material.ambient * light_ambient;
    diffuse = material.diffuse * light_diffuse * nDotL;
    specular = material.specular * light_specular * pow(rDotV,material.shininess);
    fColor = vec4(ambient+diffuse+specular,1.0);
}
