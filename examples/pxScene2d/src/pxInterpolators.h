#ifndef PX_INTERPOLATORS_H
#define PX_INTERPOLATORS_H

typedef double (*pxInterp)(double i);

double pxInterpLinear(double i);
double pxStop(double t);
double pxExp1(double t);
double pxExp2(double t);
double pxExp3(double t);
double pxInQuad(double t);
double pxInCubic(double t);
double pxInBack(double t);
double pxEaseInElastic(double t);
double pxEaseOutBounce(double t);
double pxEaseOutElastic(double t);
double pxEaseInOutBounce(double t);
#endif
