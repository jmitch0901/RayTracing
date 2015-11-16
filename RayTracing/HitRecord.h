#ifndef _HIT_RECORD_
#define _HIT_RECORD_


#include <GL/glew.h>
#include <cstdlib>
#include "utils/Material.h"
#include "utils/Texture.h"
#include "utils/Object.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "Ray.h"


class HitRecord{
	public:
		HitRecord();
		HitRecord(float t1, glm::vec4 p1, glm::vec4 normal1, graphics::Material material1);
		HitRecord(float t1, glm::vec4 p1, glm::vec4 normal1, graphics::Material material1, int sText1, int tText1, graphics::Texture *texture1);
		~HitRecord();
		void setT(float t1);
		void setP(glm::vec4 p1);
		void setNormal(glm::vec4 normal1);
		void setMaterial(graphics::Material material1);
		void setTextCoords(int sText1, int tText1);
		void setTexture(graphics::Texture *texture1);
		float getT();
		glm::vec4 getP();
		glm::vec4 getNormal();
		graphics::Material getMaterial();
		int getSTextCoord();
		int getTTextCoord();
		graphics::Texture* getTexture();

	private:
		float t;
		glm::vec4 p;
		glm::vec4 normal;
		graphics::Material material;
		int sText;
		int tText;
		graphics::Texture *texture;
};
#endif