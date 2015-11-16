#ifndef _RAY_
#define _RAY_

#include <glm/glm.hpp>

class Ray{
	public:
		Ray(glm::vec4 s, glm::vec4 v);
		Ray();
		~Ray();
		void setS(glm::vec4 s);
		void setV(glm::vec4 v);
		glm::vec4 getS();
		glm::vec4 getV();
		glm::vec4 point(float t);

	private:
		glm::vec4 s, v;


};
#endif