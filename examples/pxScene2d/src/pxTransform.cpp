#include "pxTransform.h"
#include <ctype.h>

// non-destructive applies transform on top of provided matrix
rtError pxTransformData::applyMatrix(pxMatrix4f& m)
{
  return mTransform->applyMatrix(this, m);
}

rtError pxTransformData::get(const char* name, float& v)
{
  return mTransform->get(this, name, v);
}

rtError pxTransformData::set(const char* name, float v)
{
  return mTransform->set(this, name, v);
}












enum opcode
{
  OP_PUSHFLOAT,
  OP_PUSHREGISTER,
  OP_CALLFUNCTION,
};

enum tokenType { floatType, idType };

typedef struct
{
  tokenType t;
  const char* s;
  uint32_t len;
} token;



funcEntry pxTransform::funcEntries[] =
{
#if 1
  {"translateXY", pxTransform::opTranslateXY},
  {"rotateInDegreesXYZ", pxTransform::opRotateInDegreesXYZ},
  {"scaleXY", pxTransform::opScaleXY},
#endif
};
uint32_t pxTransform::numFuncEntries = sizeof(funcEntries)/sizeof(funcEntries[0]);

pxTransform::pxTransform():mStack(NULL),mByteCode(NULL)
{
    mStackSize = 1000;
    mStack = (float*)malloc(sizeof(float)*mStackSize);
    
    mByteCodeSize = 1000;
    mByteCode = (instruction*)malloc(sizeof(instruction)*mByteCodeSize);

#if 0
    rtObjectRef i = new rtMapObject();
    i.set("x",0);
    i.set("y",0);
    i.set("cx",0);
    i.set("cy",0);
    i.set("sx",1);
    i.set("sy",1);
    i.set("r",0);
    i.set("rx",0);
    i.set("ry",0);
    i.set("rz",1);

    printf("before initTransform\n");
    initTransform(i, 
      "x cx + y cy + translateXY "
      "r rx ry rz rotateInDegreesXYZ "
      "sx sy scaleXY "
      "cx -1 * cy -1 * translateXY "
      );
    printf("after initTransform\n");

    pxTransformData* d = newData();
    if (d)
    {
#if 0
      pxMatrix4f m;
      
      d->set("x",100);
      d->set("y",100);
      
      float v;
      d->get("x", v);
      d->get("cx", v);
      
      printf("Before applyMatrix\n");    
      d->applyMatrix(m);
      printf("After applyMatrix\n");    
     
#endif 
      deleteData(d);
      printf("After deleteData\n");
    }
    else
      rtLogError("Could not allocate pxTransformData");
#endif
}

pxTransform::~pxTransform()
{
  printf("In ~pxTransform\n");
  if (mStack)
    free(mStack);
  if (mByteCode)
    free(mByteCode);
  printf("Exit ~pxTranform\n");
}

  // Can only do this once per
rtError pxTransform::initTransform(rtObjectRef regs, const char* transform)
{
  rtObjectRef keys = regs.get<rtObjectRef>("allKeys");
  uint32_t numKeys = keys.get<uint32_t>("length");
  for (uint32_t i = 0; i < numKeys; i++)
  {
    rtString key = keys.get<rtString>(i);
    float defaultValue = regs.get<float>(key);
    printf("define reg %s: %f\n", key.cString(), defaultValue);
    regInfo r;
    r.name = key;
    r.defaultValue = defaultValue;
    mRegInfo.push_back(r);
  }
  return compile(transform);
}

rtError pxTransform::applyMatrix(pxTransformData* /*d*/, pxMatrix4f& /*m*/)
{
  printf("pxTransform applyMatrix\n");
  mStackTop = -1;
  instruction* ip = mByteCode;
  printf("Here\n");

  while(ip < mByteCode+mByteCodeLen)
  {
    printf("There\n");
    switch(ip->opcode)
    {
    case OP_PUSHFLOAT:
      printf("OP_PUSHFLOAT %f\n", ip->floatValue);
      if (!push(ip->floatValue))
        return RT_FAIL;
      break;
    case OP_PUSHREGISTER:
      printf("OP_PUSHREG %f\n", 5.0);
      if (!push(5.0))
        return RT_FAIL;
      break;
    case OP_CALLFUNCTION:
      printf("OP_CALLFUNCTION \n");
      printf("stacktop %d\n", mStackTop);
      if (!ip->func(this))
        return RT_FAIL;
      break;
    default:
      rtLogError("pxTransform illegal instruction");
      break;
    }
    ip++;
  }
  return RT_OK;
}

bool pxTransform::emitInstruction(instruction& i)
{
  if (mByteCodeLen >= mByteCodeSize-1)
    return false;
  mByteCode[mByteCodeLen++] = i;
  return true;
}

bool pxTransform::emitCallFunction(transformFunc f)
{
  printf("emitCallFunction\n");
  instruction i;
  i.opcode = OP_CALLFUNCTION;
  i.func = f;
  return emitInstruction(i);
}

bool pxTransform::emitPushFloat(float f)
{
  printf("emitPushFloat\n");
  instruction i;
  i.opcode = OP_PUSHFLOAT;
  i.floatValue = f;
  return emitInstruction(i);
}

bool pxTransform::emitPushRegister(int r)
{
  printf("emitPushRegister\n");
  instruction i;
  i.opcode = OP_PUSHREGISTER;
  i.regIndex = r;
  return emitInstruction(i);
}

transformFunc pxTransform::getFunc(const char* id, uint32_t l)
{
  for (uint32_t i = 0; i < numFuncEntries; i++)
  {
    if (!strncmp(funcEntries[i].id,id,l) && strlen(funcEntries[i].id)==l)
      return funcEntries[i].func;
  }
  return NULL;
}


uint32_t pxTransform::getReg(const char* /*id*/, uint32_t /*l*/)
{
return 0; //INVALID_REG;
}


// TODO probably should make this utf8 clean
rtError pxTransform::compile(const char*s)
{
  mByteCodeLen = 0;
  bool done = false;
  
  while (!done)
  {
    while(*s && isspace(*s)) 
      s++;
    
    if (!*s) 
      done = true;
    else
    {
      if (*s == '-' || isdigit(*s))
      {
        char* e;
        errno = 0;
        float f = strtof(s,&e);
        if (errno || s == e)
        {
          done = true;
          rtLogError("pxTransform expected floating point constant\n");
          return RT_FAIL;
        }
        s = e;
        printf("push %f\n", f);
        emitPushFloat(f);
      }
      else if (isalpha(*s))  
      {
        const char* id = s;
        ++s;
        while(*s == '_' || isalnum(*s)) 
          s++;
        printf("id: %.*s\n", (int)(s-id), id);
        transformFunc func = getFunc(id,s-id);
        if (func)
          emitCallFunction(func);
        else
        {
          uint32_t i = getReg(id, s-id);
          if (i != INVALID_REG)
            emitPushRegister(i);
          else
          {
            rtLogError("pxTransform unknown identifier: %.*s",(int)(s-id),id);
            return RT_FAIL;
          }
        }
      }
      else if (*s == '+')
      {
        ++s;
        printf("add\n");
        emitCallFunction(opAdd);
      }
      else if (*s == '-')
      {
        ++s;
        printf("sub\n");
        emitCallFunction(opSub);
      }
      else if (*s == '*')
      {
        ++s;
        printf("mul\n");
        emitCallFunction(opMul);
      }
      else if (*s == '/')
      {
        ++s;
        printf("div\n");
        emitCallFunction(opDiv);
      }
      else
      {
        rtLogError("pxTransform unexpected char, %c when compiling.\n", *s);
        return RT_FAIL;
      }
    }
  }
  return RT_OK;
}

bool pxTransform::push(float v)
{
  if (mStackTop >= mStackSize-1)
  {
    rtLogError("pxTransform stack overflow");
    return false;
  }
  mStack[++mStackTop] = v;
  return true;
}

bool pxTransform::pop(float& v)
{
  if (mStackTop < 0)
  {
    rtLogError("pxTransform stack underflow");
    return false;
  }
  v = mStack[mStackTop--];
  return true;
}

bool pxTransform::opAdd(pxTransform* t)
{
  float op1, op2;
  if (!t->pop(op2))
    return false;
  if (!t->pop(op1))
    return false;
  return t->push(op1+op2);
}

bool pxTransform::opSub(pxTransform* t)
{
  float op1, op2;
  if (!t->pop(op2))
    return false;
  if (!t->pop(op1))
    return false;
  return t->push(op1-op2);
}

bool pxTransform::opMul(pxTransform* t)
{
  float op1, op2;
  if (!t->pop(op2))
    return false;
  if (!t->pop(op1))
    return false;
  return t->push(op1*op2);    
}

bool pxTransform::opDiv(pxTransform* t)
{
  float op1, op2;
  if (!t->pop(op2))
    return false;
  if (!t->pop(op1))
    return false;
  return t->push(op1/op2);
  }

bool pxTransform::opTranslateXY(pxTransform* t)
{
  float x, y;
  if (!t->pop(y))
    return false;
  if (!t->pop(x))
    return false;
  t->mMatrix.translate(x,y);
  return true;
}

bool pxTransform::opRotateInDegreesXYZ(pxTransform* t)
{
  float r, rx, ry, rz;
  if (!t->pop(rz))
    return false;
  if (!t->pop(ry))
    return false;
  if (!t->pop(rx))
    return false;
  if (!t->pop(r))
    return false;
  t->mMatrix.rotateInDegrees(r,rx,ry,rz);
  return true;
}

bool pxTransform::opScaleXY(pxTransform* t)
{
  float sx, sy;
  if (!t->pop(sy))
    return false;
  if (!t->pop(sx))
    return false;
  t->mMatrix.scale(sx,sy);
  return true;
}

