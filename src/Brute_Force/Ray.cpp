#include "Ray.h"

#include <glm/glm.hpp>

bool Ray::TestRay(glm::vec3 _dir, glm::vec3 _origin,
  glm::vec3 _tri[3], WindingOrder _wo = CW)
{
  /*
  *
  *   c .
  *
  * a .   .b
  * 
  */

  glm::vec3 a, b, c;
  if (_wo == CW)
  {
    a = _tri[0];
    b = _tri[1];
    c = _tri[2];
  }
  else
  {
    //TODO - Come back and check this
    a = _tri[0];
    b = _tri[2];
    c = _tri[1];
  }

  glm::vec3 planeNorm = glm::normalize(glm::cross(b - a, c - a));
  float k = glm::dot(planeNorm, a);

  float t = (k - glm::dot(planeNorm, _origin)) / glm::dot(planeNorm, _dir);

  glm::vec3 intPoint = _origin + (t*_dir);


  return false;
}