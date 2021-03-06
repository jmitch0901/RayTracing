#include "Scenegraph.h"
#include <stack>
#include "TransformNode.h"
#include "Ray.h"
#include "HitRecord.h"
using namespace std;
#include <glm/gtc/matrix_transform.hpp>
#include <cmath>
#include "utils/Light.h"
#include <iostream>

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
	allLights = gatherLightingObjects();
	for(int i = 0;i < allLights.size(); i++){
		glm::vec4 tempLights = modelview.top() * allLights[i].getPosition();
		allLights[i].setPosition(tempLights.x, tempLights.y, tempLights.z);
	}

	for(int y = 0; y < height; y++){
		for(int x = 0; x < width; x++){
			//Create ray R from camera through pixel (x,y)
			glm::vec4 cam = glm::vec4(0,0,0,1);		//Camera always at 0,0,0 in view coordinates.
			float z = -.5f * height / tan(60.0f*3.14159f/180.0f);	//Check range of tan() ?
			glm::vec4 dir = glm::vec4(x-width/2,y-height/2,z,0); //Near plane located .1f away from camera... Pixels not centered possibly?
			Ray pixelRay(cam, dir);
			//pixelRay.printRayReport();

			//color <- raycast(R,modelview)
			//float* color;

			glm::vec4 color = raycast(pixelRay, modelview,0);

			//pixel(x,y) <- color
			pixels[count] = color.r;
			pixels[count + 1] = color.g;
			pixels[count + 2] = color.b;
			pixels[count + 3] = color.w;			//Could be color.a ?

			//delete[] color;
			//color = NULL;

			count += 4;
		}
	}

	return pixels;
}

glm::vec4 Scenegraph::raycast(Ray R, stack<glm::mat4>& modelview, int bounce){

	HitRecord hr;
	glm::vec4 color;
	bool result = closestIntersection(R, modelview, hr);

	if(result){
		color = shade(R, modelview,hr,bounce);
	} else {
		color = glm::vec4(0,0,0,1);
	}
	return color;
}

bool Scenegraph::closestIntersection(Ray R, stack<glm::mat4> &modelview, HitRecord &hr){
	return root->intersect(R, modelview, hr);
}

glm::vec4 Scenegraph::shade(Ray R, stack<glm::mat4> &modelview,HitRecord &hr,int bounce){
	
	glm::vec3 lightVec,viewVec,reflectVec;
    
    float nDotL,rDotV;

    glm::vec4 fColor(0,0,0,1);


	/*
		First, determine if the pixel can even see a light
	*/
	bool intersects = true;
	//vector<bool> lightVisible;

	for(int i = 0; i < allLights.size(); i++){

		glm::vec4 tempLightVec= allLights[i].getPosition();

		//Make a new ray from the pixel location to the light position.

		Ray shadowRay(R.point(hr.getT()-0.001f),tempLightVec);
		HitRecord shadowHitRecord;

		//glm::vec4 lightT = (tempLightVec - shadowRay.getS())/shadowRay.getV();

		//float d = -(tempLightVec.x + tempLightVec.y + tempLightVec.z);
		//float t = -(shadowRay.getS().x + shadowRay.getS().y + shadowRay.getS().z + d)/(tempLightVec.x + tempLightVec.y + tempLightVec.z);

	



		//intersects  &=  (this->closestIntersection(shadowRay,modelview,shadowHitRecord) && t > hr.getT());
		intersects &= (this->closestIntersection(shadowRay,modelview,shadowHitRecord) /*&& t<=hr.getT()*/);

		
		

		/*if((t < shadowHitRecord.getT() && hit) || !hit){

			lightVisible.push_back(true);

		} else{

			lightVisible.push_back(false);
		}*/

		//If it does, let canSeeLight remain false, otherwise set it to true and break out of loop.
		//if(intersects) break;
	}

	//At this point, if there is no light, we should stop here and return the background color.
	//Otherwise, continue to determine what the color is...

	if(intersects){
		return glm::vec4(0,0,0,1);
	}
	

	for (int i=0;i<allLights.size();i++){
		//if(!lightVisible[i]) continue;

        if(allLights[i].getPosition().w!=0){
            lightVec = glm::vec3(glm::normalize(allLights[i].getPosition() - hr.getP()));
		} else{
            lightVec = glm::vec3(glm::normalize(-allLights[i].getPosition()));
		}
		
        glm::vec3 tNormal = glm::vec3(hr.getNormal());
        glm::vec3 normalView = glm::normalize(tNormal);
        nDotL = glm::dot(normalView,lightVec);


		//Modelview.top() multiply here in vertex shader
		viewVec = -glm::vec3(hr.getP());//-glm::vec3(hr.getP().x,hr.getP().y,hr.getP().z);

        viewVec = glm::normalize(viewVec);

        reflectVec = glm::reflect(-lightVec,normalView);
        reflectVec = glm::normalize(reflectVec);

        rDotV = max(glm::dot(reflectVec,viewVec),0.0f);

		glm::vec3 ambient = glm::vec3(hr.getMaterial().getAmbient());
		glm::vec3 diffuse = glm::vec3(hr.getMaterial().getDiffuse());
		glm::vec3 specular = glm::vec3(hr.getMaterial().getSpecular());

        ambient = ambient * allLights[i].getAmbient();
        diffuse = diffuse * allLights[i].getDiffuse() * max(nDotL,0.0f);

		//cout<<allLights[i].getSpecular().x<<", "<<allLights[i].getSpecular().y<<", "<<allLights[i].getSpecular().z<<endl;
		
        if(nDotL>0){

            specular = specular * allLights[i].getSpecular() * glm::pow(rDotV,hr.getMaterial().getShininess());
		} else{

            specular = glm::vec3(0,0,0);
		}

		glm::vec4 Ca(ambient+diffuse+specular,1.0f);

		Ca.x = max(min(Ca.x, 1.0f), 0.0f);
		Ca.y = max(min(Ca.y, 1.0f), 0.0f);
		Ca.z = max(min(Ca.z, 1.0f), 0.0f);

		fColor = fColor + Ca;

		fColor.x = max(min(fColor.x, 1.0f), 0.0f);
		fColor.y = max(min(fColor.y, 1.0f), 0.0f);
		fColor.z = max(min(fColor.z, 1.0f), 0.0f);

		//cout<<"fColor"<<fColor.x<<", "<<fColor.y<<", "<<fColor.z<<", "<<fColor.w<<endl;
    }

	fColor = fColor * hr.getMaterial().getAbsorption();

	//Reflections
	if(bounce < 5 && hr.getMaterial().getReflection()>0){

		//It's not reflecting exactly how it should
		glm::vec4 reflectVec =  glm::reflect(glm::normalize(R.getV()),glm::normalize(hr.getNormal()));
		reflectVec.z = -1.0f * reflectVec.z;//THIS
		reflectVec.w = 0.0f;//THIS

		glm::vec4 reflectP = hr.getP();
		
		glm::vec4 fudgeVec = reflectVec * 0.01f;
		fudgeVec.w = 0.0f;//THIS

		
		
		
		
		Ray reflectRay(reflectP + fudgeVec,reflectVec);

		
		
		//reflectRay.printRayReport();

		glm::vec4 rayCastedColor = raycast(reflectRay,modelview,bounce+1);
		
		fColor = fColor + rayCastedColor * hr.getMaterial().getReflection();///*fColor +*/ (raycast(reflectRay,modelview,bounce+1)); /* hr.getMaterial().getReflection())*/
		

		//cout<<"Reflected Color: "<<fColor.r<<", "<<fColor.g<<", "<<fColor.b<<endl;
	}

	fColor.x = max(min(fColor.x, 1.0f), 0.0f);
	fColor.y = max(min(fColor.y, 1.0f), 0.0f);
	fColor.z = max(min(fColor.z, 1.0f), 0.0f);


	

	/*
		IMPLEMENT TEXTURES LATER
	*/
	//Need to implement getSText and getTText
	graphics::Texture *t = hr.getTexture();

	if(t!=NULL){

		float r,g,b = 0.0f;

		t->lookup(hr.getSTextCoord(),hr.getTTextCoord(),r,g,b);

		fColor.r = (fColor.r * r);
		fColor.g = (fColor.g * g);
		fColor.b = (fColor.b * b);

		fColor.x = max(min(fColor.x, 1.0f), 0.0f);
		fColor.y = max(min(fColor.y, 1.0f), 0.0f);
		fColor.z = max(min(fColor.z, 1.0f), 0.0f);
		
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

