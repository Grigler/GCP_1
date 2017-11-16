#include "Ray.h"

//#include <glm/glm.hpp>
//#include <glm/gtc/constants.hpp>

#include "Hoops.h"

bool Ray::RayTri(Hoops::Vec3 _dir, Hoops::Vec3 _origin,
  Hoops::Vec3 _tri[3], WindingOrder _wo)
{
  /*
  *
  *   c .
  *
  * a .   .b
  * 
  */

  Hoops::Vec3 a, b, c;
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

  Hoops::Vec3 norm = Hoops::Normalize(Hoops::Cross(b - a, c - a));
  float k = Hoops::Dot(norm, a);
  float div = Hoops::Dot(norm, _dir);

  //Hoops::Dot(Hoops::Vec3(0, 0, 1), Hoops::Vec3(1, 0, 0));

  float t;
  if (div < 0.0f)
    t = (k - Hoops::Dot(norm, _origin)) /div;
  else 
    return false;

  //Doesn't intersect with plane if t < 0 as it is then behind it
  if (t < 0) return false;

  //Intersection point with plane
  Hoops::Vec3 intPoint = _origin + (_dir*t);

  Hoops::Vec3 edge = b - a;
  Hoops::Vec3 hitEdge = intPoint - a;
  Hoops::Vec3 hitNorm = Hoops::Cross(edge, hitEdge);
  if (Hoops::Dot(norm, hitNorm) < 0.0f) return false;

  edge = c - b;
  hitEdge = intPoint - b;
  hitNorm = Hoops::Cross(edge, hitEdge);
  if (Hoops::Dot(norm, hitNorm) < 0.0f) return false;

  edge = a - c;
  hitEdge = intPoint - c;
  hitNorm = Hoops::Cross(edge, hitEdge);
  if (Hoops::Dot(norm, hitNorm) < 0.0f) return false;

  return true;
}

bool Ray::RayTri(Hoops::Vec3 _dir, Hoops::Vec3 _origin,
  Hoops::Vec3 _tri[3], Hoops::Vec3 _triNorm, Hoops::Vec3 &_hitOut)
{
  Hoops::Vec3 a = _tri[0];
  Hoops::Vec3 b = _tri[1];
  Hoops::Vec3 c = _tri[2];

  float k = Hoops::Dot(_triNorm, a);
  float div = Hoops::Dot(_triNorm, _dir);

  //Hoops::Dot(Hoops::Vec3(0, 0, 1), Hoops::Vec3(1, 0, 0));

  float t;
  if (div < 0.0f)
    t = (k - Hoops::Dot(_triNorm, _origin)) / div;
  else
    return false;

  //Doesn't intersect with plane if t < 0 as it is then behind it
  if (t < 0) return false;

  //Intersection point with plane
  Hoops::Vec3 intPoint = _origin + (_dir*t);

  Hoops::Vec3 edge = b - a;
  Hoops::Vec3 hitEdge = intPoint - a;
  Hoops::Vec3 hitNorm = Hoops::Cross(edge, hitEdge);
  if (Hoops::Dot(_triNorm, hitNorm) < 0.0f) return false;

  edge = c - b;
  hitEdge = intPoint - b;
  hitNorm = Hoops::Cross(edge, hitEdge);
  if (Hoops::Dot(_triNorm, hitNorm) < 0.0f) return false;

  edge = a - c;
  hitEdge = intPoint - c;
  hitNorm = Hoops::Cross(edge, hitEdge);
  if (Hoops::Dot(_triNorm, hitNorm) < 0.0f) return false;

  _hitOut = intPoint;

  return true;
}

bool Ray::FailSafeTest(Hoops::Vec3 _dir, Hoops::Vec3 _origin,
  Hoops::Vec3 _tri[3], Hoops::Vec3 *_hitPoint)
{
  Hoops::Vec3 edge1, edge2;
  Hoops::Vec3 tVec, pVec, qVec;
  float det, invDet;

  float u, v;

  edge1 = _tri[1] - _tri[0];
  edge2 = _tri[2] - _tri[0];

  pVec = Hoops::Cross(_dir, edge2);
  det = Hoops::Dot(edge1, pVec);

  if (det > Hoops::epsilon)
  {
    tVec = _origin - _tri[0];
    u = Hoops::Dot(tVec, pVec);
    if (u < 0.0f || u > det)
      return false;

    qVec = Hoops::Cross(tVec, edge1);

    v = Hoops::Dot(_dir, qVec);
    if (v < 0.0f || u + v > det)
      return false;
  }
  else if (det < -Hoops::epsilon)
  {
    tVec = _origin - _tri[0];
    u = Hoops::Dot(tVec, pVec);
    if (u > 0.0f || u < det)
      return false;

    qVec = Hoops::Cross(tVec, edge1);

    v = Hoops::Dot(_dir, qVec);
    if (v > 0.0f || u + v < det)
      return false;
  }
  else
    return false;

  invDet = 1.0f / det;
  float t = Hoops::Dot(edge2, qVec) * invDet;
  u *= invDet;
  v *= invDet;

  return true;
}