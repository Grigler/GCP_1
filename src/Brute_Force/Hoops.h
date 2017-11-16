#ifndef __HOOPS__
#define __HOOPS__

#include <math.h>
#include <limits>

namespace Hoops
{
  static float epsilon = std::numeric_limits<float>::epsilon();

  struct Vec3
  {
    float x;
    float y;
    float z;

#pragma region "Constructors"
    Vec3()
    {
      x = 0;
      y = 0; 
      z = 0;
    }
    Vec3(float _s)
    {
      x = _s;
      y = _s;
      z = _s;
    }
    Vec3(float _x, float _y, float _z)
    {
      x = _x;
      y = _y;
      z = _z;
    }
    Vec3(Vec3& _v)
    {
      x = _v.x;
      y = _v.y;
      z = _v.z;
    }
#pragma endregion

    float Dot(Vec3 &_r)
    {
      float rtn;
      rtn = x*_r.x + y*_r.y + z*_r.z;
      return rtn;
    }
    Vec3 Cross(Vec3 &_r)
    {
      Vec3 rtn;
      rtn.x = y*_r.z - z*_r.y;
      rtn.y = z*_r.x - x*_r.z;
      rtn.z = x*_r.y - y*_r.x;
      return rtn;
    }
    Vec3 Normalize()
    {
      //As Dot(_v,_v) gives (||_v||)^2
      *this = sqrtf(Dot(*this));
    }
    Vec3 Normalized()
    {
      //As Dot(_v,_v) gives (||_v||)^2
      return sqrtf(Dot(*this));
    }

#pragma region "Operator Overloads"
    friend Vec3 operator*(Vec3 _l, const float& _op)
    {
      _l.x *= _op;
      _l.y *= _op;
      _l.z *= _op;
      return _l;
    }
    Vec3& operator*=(const float& _op)
    {
      x *= _op;
      y *= _op;
      z *= _op;
      return *this;
    }
    friend Vec3 operator/(Vec3 _l, const float& _op)
    {
      Vec3 r = _l;
      r.x /= _op;
      r.y /= _op;
      r.z /= _op;
      return r;
    }
    Vec3& operator/=(const float& _op)
    {
      x /= _op;
      y /= _op;
      z /= _op;
    }
    Vec3& operator+=(const Vec3& _v)
    {
      x += _v.x;
      y += _v.y;
      z += _v.z;
      return *this;
    }
    Vec3& operator-=(const Vec3& _v)
    {
      x -= _v.x;
      y -= _v.y;
      z -= _v.z;
      return *this;
    }
    friend Vec3 operator+(Vec3 _l, const Vec3& _r)
    {
      _l.x += _r.x;
      _l.y += _r.y;
      _l.z += _r.z;
      return _l;
    }
    friend Vec3 operator-(Vec3 _l, const Vec3& _r)
    {
      _l.x -= _r.x;
      _l.y -= _r.y;
      _l.z -= _r.z;
      return _l;
    }
#pragma endregion
  };

  inline float Dot(Vec3 &_l, Vec3 &_r)
  {
    float rtn;
    rtn = _l.x*_r.x + _l.y*_r.y + _l.z*_r.z;
    return rtn;
  }
  inline Vec3 Cross(Vec3 &_l, Vec3 &_r)
  {
    Vec3 rtn;
    rtn.x = _l.y*_r.z - _l.z*_r.y;
    rtn.y = _l.z*_r.x - _l.x*_r.z;
    rtn.z = _l.x*_r.y - _l.y*_r.x;
    return rtn;
  }
  inline Vec3 Normalize(Vec3 &_v)
  {
    //As Dot(_v,_v) gives (||_v||)^2
    return _v / sqrtf(Dot(_v, _v));
  }

  inline float min(float &_l, float &_r)
  {
    float r;
    _l < _r ? r = _l : r = _r;
    return r;
  }
  inline float max(float &_l, float &_r)
  {
    float r;
    _l > _r ? r = _l : r =_r;
    return r;
  }
  inline float clamp(float _val, float _min, float _max)
  {
    float rtn;
    _val < _min ? rtn = _min : rtn = _val;
    _val > _max ? rtn = _max : rtn = _val;
    return rtn;
  }
}

#endif