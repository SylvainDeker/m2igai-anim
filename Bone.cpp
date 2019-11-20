#include "Bone.hpp"
#include <glm/glm.hpp>

Bone::Bone():transform(glm::mat4(1.0)){}

Bone::Bone(const glm::mat4& mat_transform):transform(mat_transform){}
