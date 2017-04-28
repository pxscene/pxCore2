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

// pxMatrix4T.cpp

#include "pxMatrix4T.h"

void sincos(double x, double *s, double *c) {
  *s = sin(x);
  *c = cos(x);
}

void sincosf(float x, float *s, float *c) {
  *s = sin(x);
  *c = cos(x);
}