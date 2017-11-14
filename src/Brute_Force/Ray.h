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
  //Computes normal for given triangle verts
  static bool RayTri(glm::vec3 _dir, glm::vec3 _origin,
    glm::vec3 _tri[3], WindingOrder _wo = CCW);
  //Takes a pre-compute triangle normal
  static bool Ray::RayTri(glm::vec3 _dir, glm::vec3 _origin,
    glm::vec3 _tri[3], glm::vec3 _triNorm, glm::vec3 &_hitOut = glm::vec3(0));

  static bool FailSafeTest(glm::vec3 _dir, glm::vec3 _origin,
    glm::vec3 _tri[3], glm::vec3 *_hitPoint = NULL);
};

#endif