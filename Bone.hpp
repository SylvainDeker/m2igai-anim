#ifndef BONE_H_
#define BONE_H_
#include <glm/glm.hpp>

class Bone {
public:

  Bone ();
  Bone (const glm::mat4& mat_transform);
  glm::mat4 transform; // public


};

#endif
