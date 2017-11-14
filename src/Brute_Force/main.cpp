#include <iostream>
#include <chrono>

#include <glm/glm.hpp>
//For lerp function
#include <glm/gtx/compatibility.hpp>

#include "tiny_obj_loader.h"
#include "lodepng.h"

#include "Ray.h"

//TODO - Take these define values as command line arguments
#define UNIFORM_VAL 512
#define MAX_X UNIFORM_VAL
#define MAX_Y UNIFORM_VAL
#define MAX_Z UNIFORM_VAL
#define MODEL_SCALE 150.0f

//#define DEBUG_OUTPUT

//Simple struct to holds the 3 vertex points
//and the normal of the triangle
struct Tri
{
  glm::vec3 v[3] = { glm::vec3(0) };
  glm::vec3 n = glm::vec3(0);
  //Assumes correct Vertex values
  void CalculateNorm()
  {
    n = glm::normalize(glm::cross(v[1] - v[0], v[2] - v[0]));
  }
};
//Abstraction of the tiny_obj_loader API
struct Obj
{
  tinyobj::attrib_t attrib;
  std::vector<tinyobj::shape_t> shapes;
  std::vector<tinyobj::material_t> materials;

  Obj(const char *_path)
  {
    std::string err;
    bool ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &err, _path, NULL, true);

    if (!err.empty())
    {
      std::cerr << err << std::endl;
      //getchar();
    }

    if (!ret)
    {
      std::cerr << "! Failed: Path likely doesn't exist" << std::endl;
      getchar();
    }
  }
};

void ExportImage(std::vector<unsigned char> &_raw, std::string _name);
void ScrubRawImage(std::vector<unsigned char> &_raw);
//Takes vector representation of a raw image (assumes correct size)
//and populates with
void BruteForceTrace(std::vector<unsigned char> &_imageVec, Obj *_obj);

int main(int argc, char **argv)
{
  //Parse argv to filename here or launch demo mode

  //Load objects here
  Obj object("C:\\Users\\i7465070\\GCP_1\\models\\gourd.obj");

  std::vector<unsigned char> image;
  image.resize(MAX_X * MAX_Y * 4);
  for (auto i = image.begin(); i < image.end(); i++) (*i) = 128;

/*
  for (int x = 0; x < MAX_X; x++)
  {
    for (int y = 0; y < MAX_Y; y++)
    {
#ifdef DEBUG_OUTPUT
      std::cout << x << " " << y << std::endl;
#endif
      glm::vec3 rayPos = glm::vec3(x, y, 0);
      for (size_t s = 0; s < shapes.size(); s++)
      {
        //std::cout << "Shape " << s << ": " << shapes.at(s).name << std::endl;
        size_t indexOffset = 0;
        for (size_t f = 0; f < shapes.at(s).mesh.num_face_vertices.size(); f++)
        {
          int fv = shapes.at(s).mesh.num_face_vertices.at(f);

          Tri tri;
          for (size_t v = 0; v < fv; v++)
          {
            tinyobj::index_t i = shapes.at(s).mesh.indices.at(indexOffset + v);
            tri.v[v].x = MODEL_SCALE * attrib.vertices.at(3 * i.vertex_index + 0) + (MAX_X / 2.0f);
            tri.v[v].y = MODEL_SCALE * attrib.vertices.at(3 * i.vertex_index + 1) + (MAX_Y / 2.0f);
            tri.v[v].z = MODEL_SCALE * attrib.vertices.at(3 * i.vertex_index + 2) + (MAX_Z / 2.0f);
          }
          indexOffset += fv;

          //bool isHit = Ray::FailSafeTest(CONST_DIR, rayPos, tri.v);
          bool isHit = Ray::RayTri(CONST_DIR, rayPos, tri.v, CCW);
          if (isHit)
          {
#ifdef DEBUG_OUTPUT
            std::cout << "\tHIT" << std::endl;
#endif
            image[x *(MAX_X * 4) + 4 * y + 0] = (tri.v[0].x + tri.v[1].x + tri.v[2].x) / 3.0f;
            image[x *(MAX_X * 4) + 4 * y + 1] = (tri.v[0].y + tri.v[1].y + tri.v[2].y) / 3.0f;
            image[x *(MAX_X * 4) + 4 * y + 2] = (tri.v[0].z + tri.v[1].z + tri.v[2].z) / 3.0f;
            image[x *(MAX_X * 4) + 4 * y + 3] = 255;

            break;
          }
          else
          {
            //std::cout << std::endl;
          }
        }
      }
    }
  }
  */

  BruteForceTrace(image, &object);
  ExportImage(image, "BruteForce.png");

  return 0;
}

void ExportImage(std::vector<unsigned char> &_raw, std::string _name)
{
  std::vector<unsigned char> out;
  unsigned int errVal = lodepng::encode(out, _raw, MAX_X, MAX_Y);
  if (errVal)
  {
    std::cout << "Err " << errVal << ": " << lodepng_error_text(errVal) << std::endl;
    getchar();
  }
  lodepng::save_file(out, _name);
}

void BruteForceTrace(std::vector<unsigned char> &_imageVec, Obj *_obj)
{
  //Each object in obj file
  for (size_t s = 0; s < _obj->shapes.size(); s++)
  {
    size_t indexOffset = 0;
    //Each face in this object
    for (int f = 0; f < _obj->shapes.at(s).mesh.num_face_vertices.size(); f++)
    {
      int fv = _obj->shapes.at(s).mesh.num_face_vertices.at(f);

      Tri tri;
      for (size_t v = 0; v < fv; v++)
      {
        tinyobj::index_t i = _obj->shapes.at(s).mesh.indices.at(indexOffset + v);
        tri.v[v].x = MODEL_SCALE * _obj->attrib.vertices.at(3 * i.vertex_index + 0) + (MAX_X / 2.0f);
        tri.v[v].y = MODEL_SCALE * _obj->attrib.vertices.at(3 * i.vertex_index + 1) + (MAX_Y / 2.0f);
        tri.v[v].z = MODEL_SCALE * _obj->attrib.vertices.at(3 * i.vertex_index + 2) + (MAX_Z / 2.0f);
        //Calculating normal once per-triangle
        //rather than once per-ray
        tri.CalculateNorm();
      }
      indexOffset += fv;

      for (int x = 0; x < MAX_X; x++)
      {
#pragma omp parallel for
        for (int y = 0; y < MAX_Y; y++)
        {
          glm::vec3 hitPoint;
          bool isHit = Ray::RayTri(CONST_DIR, glm::vec3(x, y, 0), tri.v, tri.n, hitPoint);
          if (isHit)
          {
            float facingRatio = glm::max(glm::min(-glm::dot(CONST_DIR, tri.n), 1.0f), 0.4f) * 0.5f;

            _imageVec[x *(MAX_X * 4) + 4 * y + 0] = facingRatio * 128;
            _imageVec[x *(MAX_X * 4) + 4 * y + 1] = facingRatio * 255;
            _imageVec[x *(MAX_X * 4) + 4 * y + 2] = facingRatio * 128;
            _imageVec[x *(MAX_X * 4) + 4 * y + 3] = 255;
            //break;
          }
        }
      }
    }
  }
}

void BVTrace(std::vector<unsigned char> &_imageVec, Obj *_obj)
{
  //Create Bounding Volume From _obj then trace
}
