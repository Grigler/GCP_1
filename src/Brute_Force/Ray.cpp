#include "Ray.h"

#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

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
  float div = glm::dot(norm, _dir);

  float t;
  if (div != 0.0f)
    t = (k - glm::dot(norm, _origin)) / glm::dot(norm, _dir);
  else 
    return false;

  //Doesn't intersect with plane if < 0
  if (t < 0) return false;

  //Intersection point with plane
  glm::vec3 intPoint = _origin + (t*_dir);

  glm::vec3 edge = b - a;
  glm::vec3 hitEdge = intPoint - a;
  glm::vec3 hitNorm = glm::cross(edge, hitEdge);
  if (glm::dot(norm, hitNorm) <= 0.0f) return false;

  edge = c - b;
  hitEdge = intPoint - b;
  hitNorm = glm::cross(edge, hitEdge);
  if (glm::dot(norm, hitNorm) <= 0.0f) return false;

  edge = a - c;
  hitEdge = intPoint - c;
  hitNorm = glm::cross(edge, hitEdge);
  if (glm::dot(norm, hitNorm) <= 0.0f) return false;

  return true;
}

bool Ray::FailSafeTest(glm::vec3 _dir, glm::vec3 _origin,
  glm::vec3 _tri[3], glm::vec3 *_hitPoint)
{
  glm::vec3 edge1, edge2;
  glm::vec3 tVec, pVec, qVec;
  float det, invDet;

  float u, v;

  edge1 = _tri[1] - _tri[0];
  edge2 = _tri[2] - _tri[0];

  pVec = glm::cross(_dir, edge2);
  det = glm::dot(edge1, pVec);

  if (det > glm::epsilon<float>())
  {
    tVec = _origin - _tri[0];
    u = glm::dot(tVec, pVec);
    if (u < 0.0f || u > det)
      return false;

    qVec = glm::cross(tVec, edge1);

    v = glm::dot(_dir, qVec);
    if (v < 0.0f || u + v > det)
      return false;
  }
  else if (det < -glm::epsilon<float>())
  {
    tVec = _origin - _tri[0];
    u = glm::dot(tVec, pVec);
    if (u > 0.0f || u < det)
      return false;

    qVec = glm::cross(tVec, edge1);

    v = glm::dot(_dir, qVec);
    if (v > 0.0f || u + v < det)
      return false;
  }
  else
    return false;

  invDet = 1.0f / det;
  float t = glm::dot(edge2, qVec) * invDet;
  u *= invDet;
  v *= invDet;

  return true;
}