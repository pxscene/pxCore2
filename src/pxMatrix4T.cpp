#include "pxMatrix4T.h"

void sincos(double x, double *s, double *c) {
  *s = sin(x);
  *c = cos(x);
}

void sincosf(float x, float *s, float *c) {
  *s = sin(x);
  *c = cos(x);
}