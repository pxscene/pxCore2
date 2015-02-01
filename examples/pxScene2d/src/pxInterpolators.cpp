// pxCore CopyRight 2007-2015 John Robinson
// pxInterpolators.cpp

#include "pxInterpolators.h"

#include <math.h>



double easeOutElastic_helper(double t, double /*b*/, double c, double /*d*/, double a, double p) {
  if (t <= 0) return 0;
  if (t >= 1) return 1;
  double s;
  
  if (a < c) {
    a = c;
    s = p / 4.0;
  }
  else {
    s = p / (2*M_PI)*sin(c/a);
  }
  return (a * pow(2.0, -10*t) * sin((t-s)*(2.0*M_PI)/p)+c);
}

double easeOutBounce_helper(double t, double c, double a) {
  if (t == 1.0) return c;
  if (t < (4 / 11.0)) {
    return c * (7.5625 * t * t);
  } else if (t < (8 / 11.0)) {
    t -= (6 / 11.0);
    return -a * (1. - (7.5625 * t * t + .75)) + c;
  } else if (t < (10 / 11.0)) {
    t -= (9 / 11.0);
    return -a * (1. - (7.5625 * t * t + .9375)) + c;
  } else {
    t -= (21 / 22.0);
    return -a * (1. - (7.5625 * t * t + .984375)) + c;
  }
}

double easeOutElastic(double t) {
  double a = 0.8;
  double p = 0.5;
  return easeOutElastic_helper(t, 0, 1, 1, a, p);
}

double easeOutBounce(double t) {
  double a = 1.0;
  return easeOutBounce_helper(t, 1, a);
}

double pxExp(double t) {
  return pow(t, 0.48);
}

// pxStop helpers
static double PulseScale = 8;		// ratio of "tail" to "acceleration"
static double PulseNormalize = 1;

// viscous fluid with a pulse for part and decay for the rest
static double Pulse_(double x)
{
	double val;

	// test
	x = x * PulseScale;
	if (x < 1) {
		val = x - (1 - exp(-x));
	} else {
		// the previous animation ended here:
		double start = exp(-1);

		// simple viscous drag
		x -= 1;
		double expx = 1 - exp(-x);
		val = start + (expx * (1.0 - start));
	}

	return val * PulseNormalize;
}

static void ComputePulseScale()
{
	PulseNormalize = 1.0 / Pulse_(1);
}

// viscous fluid with a pulse for part and decay for the rest
double pxStop(double x)
{
	if (x >= 1) return 1;
	if (x <= 0) return 0;

	if (PulseNormalize == 1) {
		ComputePulseScale();
	}

	return Pulse_(x);
}
