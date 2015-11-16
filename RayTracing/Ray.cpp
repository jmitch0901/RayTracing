#include "Ray.h"
using namespace std;


Ray::Ray(){
	s = glm::vec4(0,0,0,1);
	v = glm::vec4(0,0,1,1);
}

Ray::Ray(glm::vec4 s1, glm::vec4 v1){
	s = s1;
	v = v1;
}

Ray::~Ray(){

}

void Ray::setS(glm::vec4 s1){
	s = s1;
}

void Ray::setV(glm::vec4 v1){
	v = v1;
}

glm::vec4 Ray::getS(){
	return s;
}

glm::vec4 Ray::getV(){
	return v;
}

glm::vec4 Ray::point(float t){
	glm::vec4 tv = glm::vec4(t * v.x, t * v.y, t * v.z, 1);		//Last param???
	return s + tv;
}

void Ray::printRayReport(){
	cout<<"P = ("<<s.x<<","<<s.y<<","<<s.z<<") + t * ("<<v.x<<","<<v.y<<","<<v.x<<")"<<endl;
}