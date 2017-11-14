#include <iostream>
#include <chrono>
#include <sstream>
#include <fstream>

#include <glm/glm.hpp>
//For lerp function
#include <glm/gtx/compatibility.hpp>

#include "tiny_obj_loader.h"
#include "lodepng.h"

#include "Ray.h"

//TODO - Take these define values as command line arguments
#define UNIFORM_VAL 256
#define MAX_X UNIFORM_VAL
#define MAX_Y UNIFORM_VAL
#define MAX_Z UNIFORM_VAL
#define MODEL_SCALE 75.0f

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
std::string g_inputFilePath = "C:\\Users\\i7465070\\GCP_1\\models\\gourd.obj";

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

  std::chrono::high_resolution_clock clock;
  std::stringstream ss;

  std::stringstream dataCSV;

  dataCSV << "Iteration,run-time(ms)" << std::endl;
  for (size_t i = 0; i < 4; i++)
  {
    printf("#%i\n", i);
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


  dataCSV << "Iteration,run-time(ms)" << std::endl;
  for (size_t i = 0; i < 5000; i++)
  {
    printf("#%i\n", i);
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
//#pragma omp parallel for
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