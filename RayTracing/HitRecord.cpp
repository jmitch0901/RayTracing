#include "HitRecord.h"


HitRecord::HitRecord(){
	t = -1;
	p = glm::vec4(0,0,0,1);
	normal = glm::vec4(0,0,0,1);
	texture = NULL;
	sText = -1;
	tText = -1;
}

HitRecord::HitRecord(float t1, glm::vec4 p1, glm::vec4 normal1, graphics::Material material1){
	t = t1;
	p = p1;
	normal = normal1;
	material = material1;
	texture = NULL;
	sText = -1;
	tText = -1;
}

HitRecord::HitRecord(float t1, glm::vec4 p1, glm::vec4 normal1, graphics::Material material1, int sText1, int tText1, graphics::Texture *texture1){
	t = t1;
	p = p1;
	normal = normal1;
	material = material1;
	texture = texture1;
	sText = sText1;
	tText = tText1;
}

HitRecord::~HitRecord(){

}

void HitRecord::setT(float t1){
	t = t1;
}

void HitRecord::setP(glm::vec4 p1){
	p = p1;
}

void HitRecord::setNormal(glm::vec4 normal1){
	normal = normal1;
}

void HitRecord::setMaterial(graphics::Material material1){
	material = material1;
}

void HitRecord::setTextCoords(int sText1, int tText1){
	sText = sText1;
	tText = tText1;
}

void HitRecord::setTexture(graphics::Texture *texture1){
	texture = texture1;
}

float HitRecord::getT(){
	return t;
}

glm::vec4 HitRecord::getP(){
	return p;
}

glm::vec4 HitRecord::getNormal(){
	return normal;
}

graphics::Material HitRecord::getMaterial(){
	return material;
}

int HitRecord::getSTextCoord(){
	return sText;
}

int HitRecord::getTTextCoord(){
	return tText;
}

graphics::Texture* HitRecord::getTexture(){
	return texture;
}

