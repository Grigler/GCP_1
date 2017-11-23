#include <iostream>
#include <chrono>
#include <sstream>
#include <fstream>


#include <glm/glm.hpp>
//For lerp function
#include <glm/gtx/compatibility.hpp>


#include "Hoops.h"

#include "tiny_obj_loader.h"
#include "lodepng.h"

#include "Ray.h"

//TODO - Take these define values as command line arguments
#define UNIFORM_VAL 256
#define MAX_X UNIFORM_VAL
#define MAX_Y UNIFORM_VAL
#define MAX_Z UNIFORM_VAL
#define MODEL_SCALE 0.065f

//Simple struct to holds the 3 vertex points
//and the normal of the triangle
struct Tri
{
  Hoops::Vec3 v[3] = { Hoops::Vec3(0) };
  Hoops::Vec3 n = Hoops::Vec3(0);
  Hoops::Vec3 min;
  Hoops::Vec3 max;
  //Assumes correct Vertex values
  void CalculateNorm()
  {
    n = Hoops::Normalize(Hoops::Cross(v[1] - v[0], v[2] - v[0]));
  }
  void CalculateBV()
  {
    for (size_t i = 0; i < 3; i++)
    {
      //Min values
      min.x = Hoops::min(min.x, v[i].x);
      min.y = Hoops::min(min.y, v[i].y);
      min.z = Hoops::min(min.z, v[i].z);
      //Max values
      max.x = Hoops::max(max.x, v[i].x);
      max.y = Hoops::max(max.y, v[i].y);
      max.z = Hoops::max(max.z, v[i].z);
    }

    
  }
};
//Abstraction of the tiny_obj_loader API
struct Obj
{
  tinyobj::attrib_t attrib;
  std::vector<tinyobj::shape_t> shapes;
  std::vector<tinyobj::material_t> materials;

  Hoops::Vec3 min;
  Hoops::Vec3 max;

  Obj(std::string _path)
  {
    std::string err;
    bool ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &err, _path.c_str(), NULL, true);
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

    //Arbritrary values drastically outside the Viewing Volume
    min = Hoops::Vec3(256);
    max = Hoops::Vec3(-256);
  }

  void BuildBV()
  {
    for (size_t s = 0; s < shapes.size(); s++)
    {
      size_t indexOffset = 0;
      //Each face in this object
      for (int f = 0; f < shapes.at(s).mesh.num_face_vertices.size(); f++)
      {
        int fv = shapes.at(s).mesh.num_face_vertices.at(f);
        for (size_t v = 0; v < fv; v++)
        {
          tinyobj::index_t i = shapes.at(s).mesh.indices.at(indexOffset + v);
          Hoops::Vec3 vert;
          vert.x = attrib.vertices.at(3 * i.vertex_index + 0);
          vert.y = attrib.vertices.at(3 * i.vertex_index + 1);
          vert.z = attrib.vertices.at(3 * i.vertex_index + 2);

          //Min values
          min.x = Hoops::min(min.x, vert.x);
          min.y = Hoops::min(min.y, vert.y);
          min.z = Hoops::min(min.z, vert.z);
          //Max values
          max.x = Hoops::max(max.x, vert.x);
          max.y = Hoops::max(max.y, vert.y);
          max.z = Hoops::max(max.z, vert.z);
        }
        indexOffset += fv;
      }
    }
    //Scaling+offsetting the mins and maxs TODO - do per xyz component
    min = min * MODEL_SCALE + (UNIFORM_VAL / 2.0f);
    max = max * MODEL_SCALE + (UNIFORM_VAL / 2.0f);
  }
};

void ParseArgs(int argc, char **argv);
void DumpData(std::stringstream &_ss, std::string _path);

void ExportImage(std::vector<unsigned char> &_raw, std::string _name);
void ScrubRawImage(std::vector<unsigned char> &_raw);
//Takes vector representation of a raw image (assumes correct size)
//and populates with
void BruteForceTrace(std::vector<unsigned char> &_imageVec, Obj *_obj);
void BVTrace(std::vector<unsigned char> &_imageVec, Obj *_obj);

bool g_visualDrawing = false;
std::string g_inputFilePath = "C:\\Users\\i7465070\\GCP_1\\models\\ManScanLow.obj";

int main(int argc, char **argv)
{
  //Parse argv to filename here or launch demo mode
  if (argc > 1)
  {
    ParseArgs(argc, argv);
  }

  //Load objects here
  Obj object(g_inputFilePath);

  std::vector<unsigned char> image;
  image.resize(MAX_X * MAX_Y * 4);
  ScrubRawImage(image);

  std::chrono::steady_clock clock;
  std::stringstream ss;

  std::stringstream dataCSV;

  /********************************************
  *               Brute Force                 *
  *********************************************/

  printf("BF:\n");
  dataCSV << "Iteration,run-time(ms)" << std::endl;
  for (size_t i = 0; i < 32; i++)
  {
    printf("\t#%i\n", i);
    //Time Start
    auto before = clock.now();
    BruteForceTrace(image, &object);
    //Time End
    auto after = clock.now();

    dataCSV << i << ',' << std::chrono::duration_cast<std::chrono::milliseconds>(after - before).count() << std::endl;

    if (g_visualDrawing)
    {
      ss.str("");
      ss << "BF/" << i << ".png";
      ExportImage(image, ss.str().c_str());
      ScrubRawImage(image);
    }
  }
  DumpData(dataCSV, "Brute_Force_Times.csv");
  dataCSV.str("");


  /********************************************
  *              Bounding Volume              *
  *********************************************/

  //Done as a pre-comuputation
  object.BuildBV();

  printf("BV:\n");
  dataCSV << "Iteration,run-time(ms)" << std::endl;
  for (size_t i = 0; i < 32; i++)
  {
    printf("\t#%i\n", i);
    //Time Start
    auto before = clock.now();
    BVTrace(image, &object);
    //Time End
    auto after = clock.now();

    dataCSV << i << ',' << std::chrono::duration_cast<std::chrono::milliseconds>(after - before).count() << std::endl;

    if (g_visualDrawing)
    {
      ss.str("");
      ss << "BV/" << i << ".png";
      ExportImage(image, ss.str().c_str());
      ScrubRawImage(image);
    }
  }
  DumpData(dataCSV, "Bounding_Volume_Times.csv");
  dataCSV.str("");

  /********************************************
  *               Triangle BV                 *
  *********************************************/

  printf("TriBV:\n");
  dataCSV << "Iteration,run-time(ms)" << std::endl;
  for (size_t i = 0; i < 32; i++)
  {
    printf("\t#%i\n", i);
    //Time Start
    auto before = clock.now();
    BVTrace(image, &object);
    //Time End
    auto after = clock.now();

    dataCSV << i << ',' << std::chrono::duration_cast<std::chrono::milliseconds>(after - before).count() << std::endl;

    if (g_visualDrawing)
    {
      ss.str("");
      ss << "TriBV/" << i << ".png";
      ExportImage(image, ss.str().c_str());
      ScrubRawImage(image);
    }
  }
  DumpData(dataCSV, "Tri_BV.csv");

  return 0;
}

void ParseArgs(int argc, char **argv)
{
  //Command Line Switches
  std::vector<std::string> validSwitches;
  validSwitches.push_back("--visual");
  validSwitches.push_back("--input");

  bool validUse = false;

  //Start at 1 to ignore location arg
  for (size_t i = 1; i < argc; i++)
  {
    std::string arg = argv[i];
    for (auto j = 0; j < validSwitches.size(); j++)
    {
      if (arg == validSwitches.at(j))
      {
        if (validSwitches.at(j) == "--visual")
        {
          //Making folders as lodePNG cannot create them
          system("mkdir BF");
          system("mkdir BV");
          system("mkdir TriBV");

          g_visualDrawing = true;
          validUse = true;
          break;
        }

        if (validSwitches.at(j) == "--input")
        {
          if (argc > i)
          {
            g_inputFilePath = argv[i + 1];
            validUse = true;
            break;
          }
          else
          {
            validUse = false;
          }
        }
      }
    }

    if (!validUse)
    {
      std::cerr << "Usage:\n\tRayTrace [switches]\n\tValidSwitches:\n\t\t--visual - Outputs pngs of traces\n\t\t--input [path] uses input obj file to trace\n";
    }
  }
}
void DumpData(std::stringstream &_ss, std::string _path)
{
  std::ofstream file(_path);
  if (file.is_open())
  {
    file << _ss.str().c_str();
    file.close();
  }
  else
  {
    std::cerr << "! Error: Couldn't Open File: " << _path << std::endl;
  }
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
void ScrubRawImage(std::vector<unsigned char> &_raw)
{
  for (auto p = _raw.begin(); p < _raw.end(); p++) (*p) = 128;
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
      //printf("Tri: %i / %i\n", f, _obj->shapes.at(s).mesh.num_face_vertices.size());
      int fv = _obj->shapes.at(s).mesh.num_face_vertices.at(f);

      Tri tri;
      for (size_t v = 0; v < fv; v++)
      {
        tinyobj::index_t i = _obj->shapes.at(s).mesh.indices.at(indexOffset + v);
        tri.v[v].x = MODEL_SCALE * _obj->attrib.vertices.at(3 * i.vertex_index + 0) + (MAX_X / 2.0f);
        tri.v[v].y = MODEL_SCALE * _obj->attrib.vertices.at(3 * i.vertex_index + 1) + (MAX_Y / 2.0f);
        tri.v[v].z = MODEL_SCALE * _obj->attrib.vertices.at(3 * i.vertex_index + 2) + (MAX_Z / 2.0f);
      }
      //Calculating normal once per-triangle
      //rather than once per-ray
      tri.CalculateNorm();
      indexOffset += fv;

      for (int x = 0; x < MAX_X; x++)
      {
//#pragma omp parallel for
        for (int y = 0; y < MAX_Y; y++)
        {
          Hoops::Vec3 hitPoint;
          //Using x&y pixels for Rays (0,x,y) origin as-per the criteria
          bool isHit = Ray::RayTri(CONST_DIR, Hoops::Vec3(0, y, x), tri.v, tri.n, hitPoint);
          if (isHit && hitPoint.x < MAX_X && hitPoint.y < MAX_Y && hitPoint.z < MAX_Z
                    && hitPoint.x >= 0  && hitPoint.y >= 0  && hitPoint.z >= 0)
          {
            if (g_visualDrawing)
            {
              float facingRatio = Hoops::clamp(-Hoops::Dot(CONST_DIR, tri.n), 0.4f, 1.0f) * 0.5f;

              _imageVec[(MAX_Y - y) *(MAX_Y * 4) + 4 * x + 0] = facingRatio * 128;
              _imageVec[(MAX_Y - y) *(MAX_Y * 4) + 4 * x + 1] = facingRatio * 255;
              _imageVec[(MAX_Y - y) *(MAX_Y * 4) + 4 * x + 2] = facingRatio * 128;
              _imageVec[(MAX_Y - y) *(MAX_Y * 4) + 4 * x + 3] = 255;
            }
            else
            {
              _imageVec[(MAX_Y - y) *(MAX_Y * 4) + 4 * x + 0] = 255;
              _imageVec[(MAX_Y - y) *(MAX_Y * 4) + 4 * x + 1] = 255;
              _imageVec[(MAX_Y - y) *(MAX_Y * 4) + 4 * x + 2] = 255;
              _imageVec[(MAX_Y - y) *(MAX_Y * 4) + 4 * x + 3] = 255;
            }
          }
        }
      }
    }
  }
}
void BVTrace(std::vector<unsigned char> &_imageVec, Obj *_obj)
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
      }
      //Calculating normal once per-triangle
      //rather than once per-ray
      tri.CalculateNorm();
      indexOffset += fv;

      for (int x = 0; x < MAX_X; x++)
      {

        if (x <= _obj->min.z || x >= _obj->max.z)  
          continue;
//#pragma omp parallel for
        for (int y = 0; y < MAX_Y; y++)
        {
          if (y <= _obj->min.y || y >= _obj->max.y)  
            continue;

          Hoops::Vec3 hitPoint;
          bool isHit = Ray::RayTri(CONST_DIR, Hoops::Vec3(0, y, x), tri.v, tri.n, hitPoint);
          if (isHit && hitPoint.x < MAX_X && hitPoint.y < MAX_Y && hitPoint.z < MAX_Z
                    && hitPoint.x >= 0  && hitPoint.y >= 0  && hitPoint.z >= 0)
          {
            if (g_visualDrawing)
            {
              float facingRatio = Hoops::clamp(-Hoops::Dot(CONST_DIR, tri.n), 0.4f, 1.0f) * 0.5f;

              _imageVec[(MAX_Y - y) *(MAX_Y * 4) + 4 * x + 0] = facingRatio * 128;
              _imageVec[(MAX_Y - y) *(MAX_Y * 4) + 4 * x + 1] = facingRatio * 255;
              _imageVec[(MAX_Y - y) *(MAX_Y * 4) + 4 * x + 2] = facingRatio * 128;
              _imageVec[(MAX_Y - y) *(MAX_Y * 4) + 4 * x + 3] = 255;
            }
            else
            {
              _imageVec[(MAX_Y - y) *(MAX_Y * 4) + 4 * x + 0] = 255;
              _imageVec[(MAX_Y - y) *(MAX_Y * 4) + 4 * x + 1] = 255;
              _imageVec[(MAX_Y - y) *(MAX_Y * 4) + 4 * x + 2] = 255;
              _imageVec[(MAX_Y - y) *(MAX_Y * 4) + 4 * x + 3] = 255;
            }            
          }
        }
      }
    }
  }
}
void TriBVTrace(std::vector<unsigned char> &_imageVec, Obj *_obj)
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
      }
      //Calculating normal once per-triangle
      //rather than once per-ray
      tri.CalculateNorm();
      tri.CalculateBV();
      indexOffset += fv;

      for (int x = 0; x < MAX_X; x++)
      {

        if (x <= tri.min.z || x >= tri.max.z)
          continue;
//#pragma omp parallel for
        for (int y = 0; y < MAX_Y; y++)
        {
          if (y <= tri.min.y || y >= tri.max.y)
            continue;

          Hoops::Vec3 hitPoint;
          bool isHit = Ray::RayTri(CONST_DIR, Hoops::Vec3(0, y, x), tri.v, tri.n, hitPoint);
          if (isHit && hitPoint.x < MAX_X && hitPoint.y < MAX_Y && hitPoint.z < MAX_Z
                    && hitPoint.x >= 0  && hitPoint.y >= 0  && hitPoint.z >= 0)
          {
            if (g_visualDrawing)
            {
              float facingRatio = Hoops::clamp(-Hoops::Dot(CONST_DIR, tri.n), 0.4f, 1.0f) * 0.5f;

              _imageVec[(MAX_Y - y) *(MAX_Y * 4) + 4 * x + 0] = facingRatio * 128;
              _imageVec[(MAX_Y - y) *(MAX_Y * 4) + 4 * x + 1] = facingRatio * 255;
              _imageVec[(MAX_Y - y) *(MAX_Y * 4) + 4 * x + 2] = facingRatio * 128;
              _imageVec[(MAX_Y - y) *(MAX_Y * 4) + 4 * x + 3] = 255;
            }
            else
            {
              _imageVec[(MAX_Y - y) *(MAX_Y * 4) + 4 * x + 0] = 255;
              _imageVec[(MAX_Y - y) *(MAX_Y * 4) + 4 * x + 1] = 255;
              _imageVec[(MAX_Y - y) *(MAX_Y * 4) + 4 * x + 2] = 255;
              _imageVec[(MAX_Y - y) *(MAX_Y * 4) + 4 * x + 3] = 255;
            }
          }
        }
      }
    }
  }
}