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

// pxTransform.h

#ifndef PX_TRANSFORM_H
#define PX_TRANSFORM_H

#include "rtCore.h"
#include "rtRef.h"
#include "rtValue.h"
#include "rtObject.h"
#include "pxMatrix4T.h"

#define INVALID_REG UINT32_MAX

class pxTransform;

class pxTransformData
{
public:
  // TODO make constructor private
  pxTransformData(pxTransform* t, uint32_t numRegisters):mRegData(NULL)
  {
    mTransform = t;
    mRegData = (float*)malloc(sizeof(float)*numRegisters);
  }

  ~pxTransformData()
  {
    //rtLogDebug("In ~pxTransformData\n");
#if 0
    if (mRegData)
      free(mRegData);
#endif
  }

  // non-destructive applies transform on top of provided matrix
  rtError applyMatrix(pxMatrix4f& m);
  rtError get(const char* name, float& v);
  rtError set(const char* name, float v);

  // warning no range checks
  rtError get(uint32_t i, float& v)
  {
    v = mRegData[i];
    return RT_OK;
  }

  rtError set(uint32_t i, float v)
  {
    mRegData[i] = v;
    return RT_OK;
  }

private:
  rtRef<pxTransform> mTransform;
  float* mRegData;
};

class pxTransform;
typedef bool (*transformFunc)(pxTransform*);


struct regInfo
{
  rtString name;
  float defaultValue;
};

struct funcEntry
{
  const char* id;
  transformFunc func;
};

typedef struct
{
  uint8_t opcode;
  union
  {
    float floatValue;
    uint32_t regIndex;
    transformFunc func;
  };
} instruction;

class pxTransform: public rtObject
{
public:
  pxTransform();
  ~pxTransform();

  rtError initTransform(rtObjectRef vars, const char* transform);

// TODO probably should make this utf8 clean
  rtError compile(const char*s);
  rtError applyMatrix(pxTransformData* d, pxMatrix4f& m);


  rtError get(pxTransformData* d, const char* n, float& v)
  {
    uint32_t r = getReg(n, strlen(n));
    if (r != INVALID_REG)
      return d->get(r, v);
    return RT_OK;
  }

  rtError set(pxTransformData* d, const char* n, float v)
  {
    uint32_t r = getReg(n, strlen(n));
    if (r != INVALID_REG)
      return d->set(r, v);
    return RT_OK;
  }

  pxTransformData *newData()
  {
    return new pxTransformData(this, mRegInfo.size());
  }

  void deleteData(pxTransformData* d)
  {
    delete d;
  }

private:

  bool emitInstruction(instruction& i);

  bool emitCallFunction(transformFunc f);
  bool emitPushFloat(float f);
  bool emitPushRegister(int r);

  transformFunc getFunc(const char* id, uint32_t l);
  uint32_t getReg(const char* id, uint32_t l);



  vector<regInfo> mRegInfo;

  // execution context
  float* mStack;
  int32_t mStackTop;  // top
  int32_t mStackSize; // capacity

  // bytecode
  instruction* mByteCode;
  uint32_t mByteCodeLen;
  uint32_t mByteCodeSize;

  pxMatrix4f mMatrix;

  bool push(float v);
  bool pop(float& v);

  static bool opAdd(pxTransform*);
  static bool opSub(pxTransform*);
  static bool opMul(pxTransform*);
  static bool opDiv(pxTransform*);
  static bool opTranslateXY(pxTransform*);
  static bool opRotateInDegreesXYZ(pxTransform*);
  static bool opScaleXY(pxTransform*);

  static funcEntry funcEntries[];
  static uint32_t numFuncEntries;
};

#endif
