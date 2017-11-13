#ifndef __RAY__
#define __RAY__

#include <glm/vec3.hpp>

#define CONST_DIR glm::vec3(0, 0, 1)

/*
* Why bother taking the OO approach when the problem
* is inherently functional
*/

enum WindingOrder
{
  CW,
  CCW
};

class Ray
{
public:
  static bool RayTri(glm::vec3 _dir, glm::vec3 _origin,
    glm::vec3 _tri[3], WindingOrder _wo = CCW);

};

#endif