#include "mymath.h"
#include <cmath>

float Find_Squared_Distance(Vector2 pos1, Vector2 pos2) {
  return (pow(pos1.x-pos2.x, 2) + pow(pos1.y-pos2.y, 2));
}

bool AABB_Point_Collision(Vector2 point, Vector2 pos, Vector2 size) {
  return (point.x > pos.x and point.x < pos.x + size.x and point.y > pos.y and point.y < pos.y + size.y);
}

float lerpf(float x, float y, float t) {
  return (1.0 - t) * x + t * y;
}

Vector2 iVector2_to_Vector2(iVector2 Vec) {
  return Vector2{(float)Vec.x, (float)Vec.y};
}

iVector2 iVector2_to_Vector2(Vector2 Vec) {
  return iVector2{(int)Vec.x, (int)Vec.y};
}