#include "Ray.h"

#include <glm/glm.hpp>

bool Ray::RayTri(glm::vec3 _dir, glm::vec3 _origin,
  glm::vec3 _tri[3], WindingOrder _wo)
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
    b = _tri[2];
    c = _tri[1];
  }
  else
  {
    //TODO - Come back and check this
    a = _tri[0];
    b = _tri[1];
    c = _tri[2];
  }

  glm::vec3 norm = glm::normalize(glm::cross(b - a, c - a));
  float k = glm::dot(norm, a);

  float t = (k - glm::dot(norm, _origin)) / glm::dot(norm, _dir);

  //Doesn't intersect with plane if < 0
  if (t < 0) return false;

  //Intersection point with plane
  glm::vec3 intPoint = _origin + (t*_dir);

  glm::vec3 edge = b - a;
  glm::vec3 hitEdge = intPoint - a;
  glm::vec3 hitNorm = glm::cross(edge, hitEdge);
  if (glm::dot(norm, hitNorm) < 0) return false;

  edge = c - b;
  hitEdge = intPoint - b;
  hitNorm = glm::cross(edge, hitEdge);
  if (glm::dot(norm, hitNorm) < 0) return false;

  edge = a - c;
  hitEdge = intPoint - c;
  hitNorm = glm::cross(edge, hitEdge);
  if (glm::dot(norm, hitNorm) < 0) return false;

  return true;
}