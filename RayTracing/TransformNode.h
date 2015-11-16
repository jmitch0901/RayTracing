#ifndef _TRANSFORMNODE_H_
#define _TRANSFORMNODE_H_


#include "Node.h"
#include "GroupNode.h"
#include "Ray.h"
#include "HitRecord.h"
#define GLM_SWIZZLE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>


class TransformNode : public Node
{
public:

	TransformNode(Scenegraph *graph,string name="")
		:Node(graph,name)
	{
		transform = glm::mat4(1.0);
		animation_transform = glm::mat4(1.0);
		child = NULL;
		
	}

	~TransformNode(void)
	{
		if (child!=NULL)
			delete child;
	}

	virtual Node *clone()
	{
		Node *newchild;

		if (child!=NULL)
		{
			newchild = child->clone();
		}
		else
		{
			newchild = NULL;
		}

		TransformNode *newtransform = new TransformNode(scenegraph,name);
		newtransform->setTransform(transform);
		newtransform->setAnimationTransform(animation_transform);

		if (newchild!=NULL)
		{
			newtransform->setChild(newchild);
		}		
		return newtransform;
	}

	virtual graphics::Object * getInstanceOf(string name){
		//cout<<this->name<<endl;

		if(this->name == name){
			//cout<<this->name<<endl;
			//cout<<"Returning you my child's object"<<endl;
			return child->getInstanceOf(name);
		}

		GroupNode *n = dynamic_cast<GroupNode *>(child);
		
		if(n!=NULL){
			return child->getInstanceOf(name);
		}
		return NULL;
	}


	virtual vector<graphics::Light> grabLightingObjects(){

		vector<graphics::Light> returnMe = child->grabLightingObjects();

		for(int i = 0; i < lighting.size(); i++){
			//WE NEED DIRECTION AS WELL!
			
			returnMe.push_back(lighting[i]);
		}


		//Now apply transformations
		for(int i = 0; i < returnMe.size(); i++){
			//returnMe[i].setSpotDirection(glm::vec3(returnMe[i].getSpotDirection() * animation_transform*this->transform ));
			//returnMe[i].setPosition(glm::vec3(returnMe[i].getPosition() * animation_transform *this->transform));

			//returnMe[i].setSpotDirection(glm::vec3(returnMe[i].getSpotDirection() * this->transform * animation_transform ));
			returnMe[i].setPosition(glm::vec3(animation_transform * this->transform * returnMe[i].getPosition()));
		}
		
		return returnMe;
	}


	/*
		Should I apply the transforms to the light's Im storing
		inside the transform node?

	*/
	

	virtual Node *getNode(string name)
	{
		if (Node::getNode(name)!=NULL)
			return Node::getNode(name);

		if (child!=NULL)
		{
			return child->getNode(name);
		}

		return NULL;
	}

	void setChild(Node *child)
	{
		this->child = child;
		this->child->setParent(this);

	}

	virtual void draw(stack<glm::mat4> &modelView)
    {
        modelView.push(modelView.top());
        modelView.top() = modelView.top() * animation_transform * transform;
        if (child!=NULL)
			child->draw(modelView);
        modelView.pop();
    }

	virtual bool intersect(Ray R, stack<glm::mat4> &modelview,HitRecord &hr){

		bool returnBool = false;
		modelview.push(modelview.top());
		modelview.top() = glm::inverse(glm::transpose(modelview.top()));

		if(child!=NULL){
			returnBool = child->intersect(R,modelview,hr);
		}

		modelview.pop();

		return returnBool;

	}

	virtual void drawBB(stack<glm::mat4>& modelView)
	{
		
		/*if (bbDraw)
		{
			glm::mat4 bbTransform;

			bbTransform = glm::translate(glm::mat4(1.0),0.5f*(minBounds+maxBounds)) * glm::scale(glm::mat4(1.0),maxBounds-minBounds);
			glm::vec4 color = glm::vec4(1,1,1,1);
			//set the color for all vertices to be drawn for this object
			glUniform3fv(scenegraph->objectColorLocation,1,glm::value_ptr(color));
			glUniformMatrix4fv(scenegraph->modelviewLocation,1,GL_FALSE,glm::value_ptr(modelView.top() * bbTransform));
			scenegraph->getInstance("box")->draw(GL_TRIANGLES);        		
		}
		modelView.push(modelView.top());
        modelView.top() = modelView.top() * animation_transform * transform;
        
		if (child!=NULL)
			child->drawBB(modelView);
        modelView.pop();*/
	}

	virtual void updateBB()
	{
		glm::vec4 points[8];
		glm::vec3 bounds[2];
		int i,j,k,l;
		float x,y,z;

		if (child==NULL)
			return;

		child->updateBB();

		bounds[0] = child->getMinBounds();
		bounds[1] = child->getMaxBounds();

		l=0;
		for (i=0;i<2;i++)
		{
			x = bounds[i][0];
			for (j=0;j<2;j++)
			{
				y = bounds[j][1];
				for (k=0;k<2;k++)
				{
					z = bounds[k][2];
					points[l] = glm::vec4(x,y,z,1);
					l++;
				}
			}
		}

		//transform
		for (i=0;i<8;i++)
		{
			points[i] = animation_transform * transform * points[i];
		}

		minBounds = maxBounds = glm::vec3(points[0].x,points[0].y,points[0].z);

		for (i=1;i<8;i++)
		{
			for (j=0;j<3;j++)
			{
				if (points[i][j]<minBounds[j])
					minBounds[j] = points[i][j];
				if (points[i][j]>maxBounds[j])
					maxBounds[j] = points[i][j];
			}
		}
	}

	void setTransform(glm::mat4 obj)
    {
        transform = obj;
    }

    void setAnimationTransform(glm::mat4 mat)
    {
        animation_transform = mat;
    }

    glm::mat4 getTransform()
    {
        return transform;
    }

    glm::mat4 getAnimationTransform()
    {
        return animation_transform;
    }

	void setScenegraph(Scenegraph *graph)
	{
		Node::setScenegraph(graph);
		if (child!=NULL)
		{
			child->setScenegraph(graph);
		}
	}

protected:
	glm::mat4 transform,animation_transform;
	Node *child;
};

#endif