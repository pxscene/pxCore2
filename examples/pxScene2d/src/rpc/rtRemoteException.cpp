#ifndef __RT_REMOTE_EXCEPTION__
#define __RT_REMOTE_EXCEPTION__

#include <exception>
#include <sstream>
#include <string>
#include <memory>

class rtRemoteException : public std::exception
{
public:
  rtRemoteException(std::string const& reason, char const* file, char const* func, int line)
    : m_reason(reason)
    , m_file(file)
    , m_func(func)
    , m_line(line)
  {
  }

  rtRemoteException(std::string const& reason, char const* file, char const* func,
      int line, rtRemoteException* inner)
    : m_reason(reason)
    , m_file(file)
    , m_func(func)
    , m_line(line)
  {
    if (inner)
    {
      m_inner.reset(new rtRemoteException());
      m_inner->m_reason = inner->m_reason;
      m_inner->m_file = inner->m_file;
      m_inner->m_func = inner->m_func;
      m_inner->m_line = inner->m_line;
    }
  }

  rtRemoteException(rtRemoteException const& rhs)
    : m_reason(rhs.m_reason)
    , m_file(rhs.m_file)
    , m_func(rhs.m_func)
    , m_line(rhs.m_line)
  {
    if (rhs.m_inner)
      m_inner = rhs.m_inner;
  }

  rtRemoteException const& operator = (rtRemoteException const& rhs)
  {
    m_reason = rhs.m_reason;
    m_file = rhs.m_file;
    m_func = rhs.m_func;
    m_line = rhs.m_line;
    if (rhs.m_inner)
      m_inner = rhs.m_inner;
    else
      m_inner.reset();
    return *this;
  }

  rtRemoteException()
    : m_file(nullptr)
    , m_func(nullptr)
    , m_line(0)
    , m_inner(nullptr)
  {
  }

  std::string toString() const
  {
    std::stringstream buff;
    return buff.str();
  }

private:
  using Pointer = std::shared_ptr<rtRemoteException>;

private:
  std::string   m_reason;
  char const*   m_file;
  char const*   m_func;
  int           m_line;
  Pointer       m_inner;
};

#endif

#include <iostream>

void baz()
{
  throw rtRemoteError("it failed");
}

void bar()
{
  try
  {
    baz();
  }
  catch (rtRemoteException const& err)
  {
  }
}

void foo()
{
  try
  {
    bar();
  }
  catch (rtRemoteException const& err)
  {
  }
}

int main(int argc, char* argv[])
{
  try
  {
    foo();
  }
  catch (rtRemoteException const& err)
  {
    std::cout << "err" << std::endl;
    std::cout << err.toString();
  }
  return 0;
}
