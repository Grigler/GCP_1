#include <iostream>

#include "tiny_obj_loader.h"
#include "lodepng.h"

#include "Ray.h"

#define UNIFORM_VAL 32
#define MAX_X UNIFORM_VAL
#define MAX_Y UNIFORM_VAL
#define MAX_Z UNIFORM_VAL

struct Tri
{
  glm::vec3 v[3];
};

int main(int argc, char **argv)
{
  //Test
  //glm::vec3 tri[3] = { glm::vec3(-1, -1, 0), glm::vec3(1, -1, 0), glm::vec3(0.5, 1, 0) };
  //bool isHit = Ray::RayTri(CONST_DIR, glm::vec3(0.5, 0.5, -2), tri, CCW);

  //Load objects here
  tinyobj::attrib_t attrib;
  std::vector<tinyobj::shape_t> shapes;
  std::vector<tinyobj::material_t> materials;
  std::string err;
  bool ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &err, "C:\\Users\\i7465070\\GCP_1\\models\\sandal.obj", NULL, true);
  if (!err.empty())
  {
    std::cerr << err << std::endl;
    getchar();
  }
  if (!ret)
  {
    std::cerr << "Failed to parse" << std::endl;
  }

  std::vector<unsigned char> image;
  image.resize(MAX_X * MAX_Y * 4);
  for (auto i = image.begin(); i < image.end(); i++) (*i) = 128;

  for (int x = 0; x < MAX_X; x++)
  {
    for (int y = 0; y < MAX_Y; y++)
    {
      std::cout << x << " " << y << std::endl;
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
            tri.v[v].x = attrib.vertices.at(3 * i.vertex_index + 0);// +(MAX_X / 2.0f);
            tri.v[v].y = attrib.vertices.at(3 * i.vertex_index + 1);// +(MAX_Y / 2.0f);
            tri.v[v].z = attrib.vertices.at(3 * i.vertex_index + 2);// +(MAX_Z / 2.0f);
          }
          indexOffset += fv;

          bool isHit = Ray::RayTri(CONST_DIR, rayPos, tri.v);
          if (isHit)
          {
            std::cout << "\tHIT" << std::endl;
            image[4 * x * y + 4 * x + 0] = 255;
            image[4 * x * y + 4 * x + 1] = 0;
            image[4 * x * y + 4 * x + 2] = (tri.v[0].z / MAX_Z) * 255;
            image[4 * x * y + 4 * x + 3] = 255;
            //getchar();
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
  //unsigned int errVal = lodepng_encode_file("./Output/OUT.png", &image[0], MAX_X, MAX_Y, LodePNGColorType::LCT_RGBA);
  
  std::vector<unsigned char> out;
  unsigned int errVal = lodepng::encode(out, image, MAX_X, MAX_Y);
  if (errVal)
  {
    std::cout << "Err " << errVal << ": " << lodepng_error_text(errVal) << std::endl;
    getchar();
  }
  lodepng::save_file(out, "OUT.png");

  return 0;
}