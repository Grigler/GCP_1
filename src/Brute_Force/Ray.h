#ifndef __RAY__
#define __RAY__

//#include <glm/vec3.hpp>
#include "Hoops.h"

#define CONST_DIR Hoops::Vec3(1, 0, 0)

enum WindingOrder
{
  CW,
  CCW
};

//Implemented as static functions, as it is unecessary to increase memory footprint for no reason
class Ray
{
public:
  //Computes normal for given triangle verts
  static bool RayTri(Hoops::Vec3 _dir, Hoops::Vec3 _origin,
    Hoops::Vec3 _tri[3], WindingOrder _wo = CCW);
  //Takes a pre-compute triangle normal
  static bool RayTri(Hoops::Vec3 _dir, Hoops::Vec3 _origin,
    Hoops::Vec3 _tri[3], Hoops::Vec3 _triNorm, Hoops::Vec3 &_hitOut = Hoops::Vec3(0));

  static bool FailSafeTest(Hoops::Vec3 _dir, Hoops::Vec3 _origin,
    Hoops::Vec3 _tri[3], Hoops::Vec3 *_hitPoint = NULL);
};

#endif