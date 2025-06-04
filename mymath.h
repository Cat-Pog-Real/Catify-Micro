#ifndef MYMATH_H
#define MYMATH_H

#include <iostream>

typedef struct iVector2 {
  int x = 0;
  int y = 0;
};

typedef struct Vector2 {
  float x = 0.0;
  float y = 0.0;
};

float Find_Squared_Distance(Vector2 pos, Vector2 pos2);

bool AABB_Point_Collision(Vector2 point, Vector2 pos, Vector2 size);

float lerpf(float x, float y, float t);

Vector2 iVector2_to_Vector2(iVector2 Vec);

iVector2 Vector2_to_iVector2(Vector2 Vec);

#endif