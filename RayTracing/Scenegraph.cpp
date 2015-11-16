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
			glm::vec4 dir = glm::vec4(x-width/2,y-height/2,z,1);			//Near plane located .1f away from camera			Pixels not centered possibly?
			Ray pixelRay(cam, dir);
			//pixelRay.printRayReport();

			//color <- raycast(R,modelview)
			float* color;

			color = raycast(pixelRay, modelview);

			//pixel(x,y) <- color
			pixels[count] = color[0];
			pixels[count + 1] = color[1];
			pixels[count + 2] = color[2];
			pixels[count + 3] = color[3];

			delete[] color;
			color = NULL;

			count += 4;
		}
	}

	return pixels;
}

float* Scenegraph::raycast(Ray R, stack<glm::mat4>& modelview){
	HitRecord hr;
	float* color = new float[4];
	bool result = closestIntersection(R, modelview, hr);

	if(result){
		//color = shade(R, hr, allLights);
		color[0] = 1;
		color[1] = 1;
		color[2] = 1;
		color[3] = 1;
	} else {
		color[0] = 0;
		color[1] = 1;
		color[2] = 0;
		color[3] = 1;
	}
	return color;
}

bool Scenegraph::closestIntersection(Ray R, stack<glm::mat4> &modelview, HitRecord &hr){
	
	//bool flag = true;
	//float t = 0;
	/*while(flag){
		glm::vec4 point = R.point(t);
		if(point.z > 10000.0f){
			return false;
		}

		t += .1f;

	}*/
	return root->intersect(R, modelview, hr);
}

float* Scenegraph::shade(Ray R, HitRecord &hr, vector<graphics::Light> lights){
	return NULL;
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

