#ifndef _LEAF_H_
#define _LEAF_H_
#include "node.h"
#include "utils/Object.h"
#include "utils/Material.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "utils/Texture.h"
#include "Ray.h"
#include "HitRecord.h"
#include <limits>

using namespace std;


class LeafNode : public Node
{

	
private:
	graphics::Texture *texture;

protected:
	graphics::Object *instanceOf;
	graphics::Material material;

public:

	LeafNode(graphics::Object *instanceOf,Scenegraph *graph,string name="")
		:Node(graph,name)
	{
		this->instanceOf = instanceOf;
		//default material
		material.setAmbient(0.0f,0.0f,0.0f);
		material.setDiffuse(0.0f,0.0f,0.0f);
		material.setSpecular(0.0f,0.0f,0.0f);
		material.setShininess(1);

		this->instanceOf->setMaterial(material);

		texture=NULL;
	}

	~LeafNode(void)
	{
	}

	Node *clone()
	{
		LeafNode *newclone = new LeafNode(instanceOf,scenegraph,name);
		newclone->setMaterial(material);
		newclone->setTexture(texture);
		return newclone;
	}

	virtual graphics::Object * getInstanceOf(string name){
		//cout<<"I FOUND YOU!"<<endl;
		//cout<<instanceOf->getName()<<endl;
		return instanceOf;
	}


	//DIRECTION AS WELL
	virtual vector<graphics::Light> grabLightingObjects(){
		return lighting;
	}
	

	virtual void draw(stack<glm::mat4> &modelView)
    {

		//GLuint a;
        if (instanceOf!=NULL)
		{
			//START DRAW HERE!

			 //get the color
            //glm::vec4 color = material.getAmbient();

			instanceOf->setMaterial(material);


			//The total transformation is whatever was passed to it, with its own transformation
			glUniformMatrix4fv(scenegraph->modelviewLocation,1,GL_FALSE,glm::value_ptr(modelView.top()*instanceOf->getTransform()));
			glUniformMatrix4fv(scenegraph->normalMatrixLocation,1,GL_FALSE,glm::value_ptr(glm::transpose(glm::inverse(modelView.top()*instanceOf->getTransform()))));

			glUniform3fv(scenegraph->mat_ambientLocation,1,glm::value_ptr(material.getAmbient()));
			glUniform3fv(scenegraph->mat_diffuseLocation,1,glm::value_ptr(material.getDiffuse()));
			glUniform3fv(scenegraph->mat_specularLocation,1,glm::value_ptr(material.getSpecular()));
			glUniform1f(scenegraph->mat_shininessLocation,material.getShininess());
			
			glUniformMatrix4fv(scenegraph->modelviewLocation,1,GL_FALSE,glm::value_ptr(modelView.top()));			
			
			if(texture!=NULL){
				
				glEnable(GL_TEXTURE_2D);//Tell openGL don't ignore my texture mapping commands
				glActiveTexture(GL_TEXTURE0);//Starts at 0 -> 8, can use to layer more textures


				glBindTexture(GL_TEXTURE_2D,texture->getTextureID());//this texture is associated with texzture 0
				glUniform1i(scenegraph->textureLocation,0); //bind GL_TEXTURE0 to sampler2D (whatever is bound to GL_TEXTURE0)



				if(texture->getName() == "floor"){
					//cout<<"FLOOR HIT"<<endl;
					//instanceOf->setTextureTransform(glm::scale(glm::mat4(1.0),glm::vec3(80,80,80)));
					glUniformMatrix4fv(scenegraph->texturematrixLocation,1,GL_FALSE,glm::value_ptr(glm::scale(glm::mat4(1.0f),glm::vec3(80,80,80))*instanceOf->getTextureTransform()));
				} else {
					glUniformMatrix4fv(scenegraph->texturematrixLocation,1,GL_FALSE,glm::value_ptr(glm::scale(glm::mat4(1.0f),glm::vec3(2,2,2))*instanceOf->getTextureTransform()));
				}
				
			} else {
				//cout<<"THERE WAS NO TEXTURE, TARD"<<endl;
			}
		}

		instanceOf->draw(GL_TRIANGLES);	
    }

	virtual bool intersect(Ray R, stack<glm::mat4> &modelview,HitRecord &hr){

		if(instanceOf == NULL){
			cout<<"For Some reason, your leaf node had a null object associated with it!"<<endl;
			return false;
		}
		
		if(instanceOf->getName() == "sphere"){
		
			
			//cout<<"Got a sphere for ray tracing!"<<endl;
			float highT,lowT,currentT; //answers

			glm::mat4 inv = glm::inverse(modelview.top());
			R.setV(inv * R.getV());
			R.setS(inv * R.getS());

			float a = R.getV().x*R.getV().x + R.getV().y*R.getV().y + R.getV().z*R.getV().z;
			float b = 2*R.getS().x*R.getV().x + 2*R.getS().y*R.getV().y + 2*R.getS().z*R.getV().z;
			float c = R.getS().x*R.getS().x + R.getS().y*R.getS().y + R.getS().z*R.getS().z - 1;

			if(a==0){
				//cout<<"a==0, RETURN FALSE"<<endl;
				return false;
			}

			float determinant = b*b - 4*a*c;

			if(determinant<0){
				//cout<<"determinant<0, RETURN FALSE"<<endl;
				return false;
			}


			highT = (-b + sqrt(determinant))/(2*a);
			lowT = (-b - sqrt(determinant))/(2*a);

			if(lowT<0 && highT<0){
				//cout<<"lowT<0 && highT<0, RETURN FALSE"<<endl;
				return false;
			}
			
			if(lowT<0){
				currentT = highT;
			} else if(highT<0){
				currentT = lowT;
			} else {
				
				if(lowT < highT) 
					currentT = lowT;
				else 
					currentT = highT;

			}


			if(hr.getT() < 0 || hr.getT() > currentT){
				//hr.setNormal(glm::vec4(R.point(currentT).x,R.point(currentT).y,R.point(currentT).z,0));
				glm::vec4 normInObj(glm::vec4(R.point(currentT).x,R.point(currentT).y,R.point(currentT).z,0));
				glm::mat4 invTran = glm::transpose(glm::inverse(modelview.top()));

				hr.setNormal(glm::normalize(invTran * normInObj));

				glm::mat4 backToTheViewture = modelview.top();
				R.setS(backToTheViewture * R.getS());
				R.setV(backToTheViewture * R.getV());

				hr.setT(currentT);
				hr.setP(R.point(currentT));
				hr.setMaterial(material);

				//hr.setTexture(this->texture);

				return true;
			}

			return false;
		
		
		} else if(instanceOf->getName() == "box"){

			glm::mat4 inv = glm::inverse(modelview.top());
			R.setV(inv * R.getV());
			R.setS(inv * R.getS());

			float tMinx, tMaxx, tMiny, tMaxy, tMinz, tMaxz;
			bool Nxflag = false, Nyflag = false, Nzflag = false;

			//Check if Ray is parallel to any axis and if starting point is not within box bounds

			if((R.getV().x == 0 && (R.getS().x < -.5 || R.getS().x > .5)) ||
				(R.getV().y == 0 && (R.getS().y < -.5 || R.getS().y > .5)) ||
				(R.getV().z == 0 && (R.getS().z < -.5 || R.getS().z > .5))){
					return false;
			}

			if(/*R.getV().x == 0){*/abs(R.getV().x) < 0.0000001f){
				tMinx = -1 * numeric_limits<float>::max();
				tMaxx = numeric_limits<float>::max();
			} else{
				tMinx = (-.5f - R.getS().x) / R.getV().x;
				tMaxx = (.5f - R.getS().x) / R.getV().x;
				if(tMinx > tMaxx){
					swap(tMinx, tMaxx);
					Nxflag = true;
				}
			}

			if(/*R.getV().y == 0){*/abs(R.getV().y) < 0.0000001f){
				tMiny = -1 * numeric_limits<float>::max();
				tMaxy = numeric_limits<float>::max();
			} else{
				tMiny = (-.5f - R.getS().y) / R.getV().y;
				tMaxy = (.5f - R.getS().y) / R.getV().y;
				if(tMiny > tMaxy){
					swap(tMiny, tMaxy);
					Nyflag = true;
				}
			}

			if(/*R.getV().z == 0){*/abs(R.getV().z) < 0.0000001f){
				tMinz = -1 * numeric_limits<float>::max();
				tMaxz = numeric_limits<float>::max();
			} else{
				tMinz = (-.5f - R.getS().z) / R.getV().z;
				tMaxz = (.5f - R.getS().z) / R.getV().z;
				if(tMinz > tMaxz){
					swap(tMinz, tMaxz);
					Nzflag = true;
				}
			}

			float maxOfMins, minOfMaxs;

			maxOfMins = tMinx > tMiny ? tMinx : tMiny;
			maxOfMins = maxOfMins > tMinz ? maxOfMins : tMinz;

			minOfMaxs = tMaxx < tMaxy ? tMaxx : tMaxy;
			minOfMaxs = minOfMaxs < tMaxz ? minOfMaxs : tMaxz;


			//cout << "("<<maxOfMins<<","<<minOfMaxs<<")"<<endl;
			
			if(maxOfMins < minOfMaxs && maxOfMins > 0){



				if(hr.getT() < 0 || hr.getT() > maxOfMins){
					glm::mat4 backToTheViewture = modelview.top();
					R.setS(backToTheViewture * R.getS());
					R.setV(backToTheViewture * R.getV());

					hr.setT(maxOfMins);
					hr.setP(R.point(maxOfMins));
					hr.setMaterial(material);

					glm::mat4 invTran = glm::transpose(glm::inverse(modelview.top()));

					if(tMinx > tMiny){
						if(tMinx > tMinz){					//One of x planes are struck by ray
							if(Nxflag){
								//x = +.5 plane
								hr.setNormal(glm::normalize(invTran * glm::vec4(1,0,0,0)));
							} else {
								//x = -.5 plane							
								hr.setNormal(glm::normalize(invTran * glm::vec4(-1,0,0,0)));
							}
						} else{								//One of z planes are struck by ray
							if(Nzflag){
								//z = +.5 plane
								hr.setNormal(glm::normalize(invTran * glm::vec4(0,0,1,0)));
							} else{
								//z = -.5 plane
								hr.setNormal(glm::normalize(invTran * glm::vec4(0,0,-1,0)));
							}
						}
					} else{
						if(tMiny > tMinz){					//One of y planes are struck by ray
							if(Nyflag){
								//y = +.5 plane
								hr.setNormal(glm::normalize(invTran * glm::vec4(0,1,0,0)));
							} else{
								//y = -.5 plane
								hr.setNormal(glm::normalize(invTran * glm::vec4(0,-1,0,0)));
							}
						} else{								//One of z planes are struck by ray
							if(Nzflag){
								//z = +.5 plane
								hr.setNormal(glm::normalize(invTran * glm::vec4(0,0,1,0)));
							} else {
								//z = -.5 plane
								hr.setNormal(glm::normalize(invTran * glm::vec4(0,0,-1,0)));
							}
						}
					}

				
					//hr.setTexture(this->texture);
					
					return true;

				}

			}
		} else {

			cout<<"Couldn't ray-cast shape for: "<<instanceOf->getName()<<endl;
			return false;

		}

		return false;
	}


	//CHANGE THIS DRAW AS WELL
	virtual void drawBB(stack<glm::mat4>& modelView)
	{
		/*if (bbDraw)
		{
			glm::mat4 bbTransform;

			bbTransform = glm::translate(glm::mat4(1.0),0.5f*(minBounds+maxBounds)) * glm::scale(glm::mat4(1.0),maxBounds-minBounds);
			glm::vec4 color = glm::vec4(1,1,1,1);
			//set the color for all vertices to be drawn for this object
			//glUniform3fv(scenegraph->objectColorLocation,1,glm::value_ptr(color));
			glUniformMatrix4fv(scenegraph->modelviewLocation,1,GL_FALSE,glm::value_ptr(modelView.top() * bbTransform));
			scenegraph->getInstance("box")->draw(GL_TRIANGLES);        		
		}*/
	}
	
	virtual void updateBB()
	{
		//minBounds = instanceOf->getMinimumWorldBounds().xyz();
		//maxBounds = instanceOf->getMaximumWorldBounds().xyz();
	}

	glm::vec4 getColor()
	{
		return material.getAmbient();
	}

	/*
		*Set the material of each vertex in this object
		*/
	virtual void setMaterial(graphics::Material& mat)
	{
		material = mat;
		instanceOf->setMaterial(mat);
	}

	/*
	* gets the material
	*/
	graphics::Material getMaterial()
	{
		return material;
	}


	/*
		UPDATE THIS WHEN WE NEED TEXTURES
	*/
	void setTexture(graphics::Texture *tex)
	{
		//cout << "Texture set to " << tex->getName() << endl;
		texture=tex;
		
	}




};
#endif
