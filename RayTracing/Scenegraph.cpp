#include "Scenegraph.h"
#include <stack>
#include "TransformNode.h"
#include "Ray.h"
#include "HitRecord.h"
using namespace std;
#include <glm/gtc/matrix_transform.hpp>
#include <cmath>
#include "utils/Light.h"

Scenegraph::Scenegraph()
{
    root = NULL;
}

void Scenegraph::makeScenegraph(Node *r)
{
    if (root!=NULL)
    {
        delete root;
        root = NULL;
    }
    this->root = r;

	if (this->root!=NULL)
		this->root->setScenegraph(this);


	initAnimate();
	vector<graphics::Light> allLights = gatherLightingObjects();
	for(int i = 0; i < allLights.size(); i++){
		cout << "allLights[" << i <<"]: " << endl;
	}
}


Scenegraph::~Scenegraph()
{
    if (root!=NULL)
    {
        delete root;
        root = NULL;
    }

	for (map<string,graphics::Texture *>::iterator it=textures.begin();it!=textures.end();it++)
	{
		delete it->second;
	}
}

Node * Scenegraph::getNode(string name){


	return root->getNode(name);


}

void Scenegraph::initShaderProgram(GLint program)
{
	modelviewLocation = glGetUniformLocation(program,"modelview");
    normalMatrixLocation = glGetUniformLocation(program,"normalmatrix");
    numLightsLocation = glGetUniformLocation(program,"numLights");


    mat_ambientLocation = glGetUniformLocation(program,"material.ambient");
    mat_diffuseLocation = glGetUniformLocation(program,"material.diffuse");
    mat_specularLocation = glGetUniformLocation(program,"material.specular");
    mat_shininessLocation = glGetUniformLocation(program,"material.shininess");

	texturematrixLocation = glGetUniformLocation(program,"texturematrix");
	textureLocation = glGetUniformLocation(program,"image");

}

float* Scenegraph::raytrace(int width, int height, stack<glm::mat4> &modelview){
	
	//Load world to view in modelview
	int count = 0;
	float* pixels = new float[width * height * 4];

	for(int y = 0; y < height; y++){
		for(int x = 0; x < width; x++){
			//Create ray R from camera through pixel (x,y)
			glm::vec4 cam = glm::vec4(0,0,0,1);				//Camera always at 0,0,0 in view coordinates.
			float z = -.5f * height / tan(60.0f*3.14159f/180.0f);															  //Check range of tan() ?
			glm::vec4 dir = glm::vec4(x-width/2,y-height/2,z,0);			//Near plane located .1f away from camera			Pixels not centered possibly?
			Ray pixelRay(cam, dir);
			//pixelRay.printRayReport();

			//color <- raycast(R,modelview)
			//float* color;

			glm::vec4 color(raycast(pixelRay, modelview));

			//pixel(x,y) <- color
			pixels[count] = color.x;
			pixels[count + 1] = color.y;
			pixels[count + 2] = color.z;
			pixels[count + 3] = color.w;			//Could be color.a ?

			//delete[] color;
			//color = NULL;

			count += 4;
		}
	}

	return pixels;
}

glm::vec4 Scenegraph::raycast(Ray R, stack<glm::mat4>& modelview){
	HitRecord hr;
	glm::vec4 color;
	bool result = closestIntersection(R, modelview, hr);

	if(result){
		color = (shade(R, hr, allLights));
		//color = glm::vec4(.5,.5,.5,1);
		/*color[0] = 1;
		color[1] = 1;
		color[2] = 1;
		color[3] = 1;*/
	} else {
		color = glm::vec4(0,0,0,1);
	}
	return color;
}

bool Scenegraph::closestIntersection(Ray R, stack<glm::mat4> &modelview, HitRecord &hr){
	
	return root->intersect(R, modelview, hr);
}

glm::vec4 Scenegraph::shade(Ray R, HitRecord &hr, vector<graphics::Light> lights){
	
	glm::vec3 lightVec,viewVec,reflectVec;
    glm::vec3 normalView = glm::vec3(hr.getNormal());
    glm::vec3 ambient(glm::vec3(hr.getMaterial().getAmbient()));
	glm::vec3 diffuse(glm::vec3(hr.getMaterial().getDiffuse()));
	glm::vec3 specular(glm::vec3(hr.getMaterial().getSpecular()));
    float nDotL,rDotV;

    glm::vec4 fColor(0,0,0,1);
	
    for (int i=0;i<lights.size();i++){
        if(lights[i].getPosition().w!=0){
            lightVec = glm::vec3(glm::normalize(lights[i].getPosition() - hr.getP()));
		} else{
            lightVec = glm::vec3(glm::normalize(-lights[i].getPosition()));
		}

        //glm::vec3 tNormal = glm::vec3(hr.getNormal());
        //normalView = glm::normalize(tNormal);
        nDotL = glm::dot(normalView,lightVec);

        viewVec = glm::vec3(-1*hr.getP().x,-1*hr.getP().y,-1*hr.getP().z);
        viewVec = glm::normalize(viewVec);

        reflectVec = glm::reflect(-lightVec,normalView);
        reflectVec = glm::normalize(reflectVec);

        rDotV = max(glm::dot(reflectVec,viewVec),0.0f);

        ambient = ambient * lights[i].getAmbient();
        diffuse = diffuse * lights[i].getDiffuse() * max(nDotL,0.0f);
        if(nDotL>0){
            specular = specular * lights[i].getSpecular() * pow(rDotV,hr.getMaterial().getShininess());
		} else{
            specular = glm::vec3(0,0,0);
		}

        fColor = fColor + glm::vec4(ambient+diffuse+specular,1.0);
    }
	
	return fColor;
}


vector<graphics::Light> Scenegraph::gatherLightingObjects(){

	if (root!=NULL)
    {
		return root->grabLightingObjects();  

    } else {
		cout<<"For some reason, root in the scene graph was NULL!"<<endl;
	}

	return vector<graphics::Light>();
}

void Scenegraph::draw(stack<glm::mat4>& modelView)
{


    if (root!=NULL)
    {
        root->draw(modelView);
    } else {
		cout<<"ROOT WAS NULL FOR DRAWING"<<endl;
	}

}

void Scenegraph::initAnimate(){



}


void Scenegraph::animate(float time)
{

	
	
}

