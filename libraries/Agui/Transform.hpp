#pragma once

namespace agui
{
  /** Used for transforming mouse coordinates. */
  class Transform
  {
    float m[4][4];
  public:
    Transform();
    Transform(float matrix[4][4]);
    void identity();
    void invert();
    void compose(const Transform& other);
    void translate(float x, float y);
    void translate(float x, float y, float z);
    void rotate(float theta);
    void rotate(float x, float y, float z, float angle);
    void scale(float sx, float sy);
    void scale(float sx, float sy, float sz);
    void transformPoint(float* x, float* y);
  };
}
