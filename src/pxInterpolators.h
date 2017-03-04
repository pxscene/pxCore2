/*

 pxCore Copyright 2005-2017 John Robinson

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

     http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.

*/

// pxInterpolators.h

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
