#ifndef GROUPNODE_H
#define GROUPNODE_H

#include <GL/gl.h>
#include <vector>
using namespace std;
#include "Node.h"
#include "Ray.h"
#include "HitRecord.h"
#include <glm/gtc/matrix_transform.hpp>


class GroupNode: public Node
{
protected:
    vector<Node *> children;

public:
    GroupNode(Scenegraph *graph,string name="")
        :Node(graph,name)
    {

    }

    ~GroupNode()
    {
        for (unsigned int i=0;i<children.size();i++)
            delete children[i];
    }

	virtual Node *clone()
	{
		vector<Node *> newc;

		for (int i=0;i<children.size();i++)
		{
			newc.push_back(children[i]->clone());
		}

		GroupNode * newgroup = new GroupNode(scenegraph,name);

		for (int i=0;i<children.size();i++)
		{
			newgroup->addChild(newc[i]);
		}		
		return newgroup;

	}


	virtual vector<graphics::Light> grabLightingObjects(){
		vector<graphics::Light> returnMe;

		for(int i = 0; i < lighting.size(); i++){
			returnMe.push_back(lighting[i]);
		}
		
		for (int i=0;i<children.size();i++)
		{
			vector<graphics::Light> childLight = children[i]->grabLightingObjects();

			for(int j = 0; j < childLight.size(); j++){
				returnMe.push_back(childLight[j]);
			}		
		}		

		
		return returnMe;
	}

	virtual graphics::Object * getInstanceOf(string name){

		for(int i = 0; i < children.size(); i++){

			graphics::Object *o = children[i]->getInstanceOf(name);

			if(o!=NULL){
				return o;
			}
		}
		
		return NULL;
	}

	virtual Node* getNode(string name)
	{
		if (Node::getNode(name)!=NULL)
		{
			return Node::getNode(name);
		}

		int i=0;
		Node *answer = NULL;

		while ((i<children.size()) && (answer == NULL))
		{
			answer = children[i]->getNode(name);
			i++;
		}
		return answer;
	}


	//THIS SHOULDN'T CHANGE
    virtual void draw(stack<glm::mat4> &modelView)
    {
        for (int i=0;i<children.size();i++)
        {
            children[i]->draw(modelView);
        }
    }

	virtual bool intersect(Ray R, stack<glm::mat4> &modelview,HitRecord &hr){

		bool returnBool = false;

		for(int i = 0; i < children.size(); i++){

			returnBool |= children[i]->intersect(R,modelview,hr);

		}

		return returnBool;

	}

	void setScenegraph(Scenegraph * graph)
	{
		Node::setScenegraph(graph);
		for (int i=0;i<children.size();i++)
		{
			children[i]->setScenegraph(graph);
		}
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
		for (int i=0;i<children.size();i++)
        {
            children[i]->drawBB(modelView);
        }*/
	}

	virtual void updateBB()
	{
		int i,j;

		if (children.size()==0)
			return;

		for (i=0;i<children.size();i++)
		{
			children[i]->updateBB();
		}

		minBounds = children[0]->getMinBounds();
		maxBounds = children[0]->getMaxBounds();

		for (i=1;i<children.size();i++)
		{
			for (j=0;j<3;j++)
			{
				if (children[i]->getMinBounds()[j]<minBounds[j])
					minBounds[j] = children[i]->getMinBounds()[j];

				if (children[i]->getMaxBounds()[j]>maxBounds[j])
					maxBounds[j] = children[i]->getMaxBounds()[j];
			}
		}
	}

    void addChild(Node *child)
    {
        children.push_back(child);
		child->setParent(this);
    }

    const vector<Node *> getChildren() const
    {
        return children;
    }
};

#endif // GROUP_H
