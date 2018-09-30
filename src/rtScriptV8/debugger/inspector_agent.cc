#include "inspector_agent.h"
#include "v8_inspector.h"

// Exported class Agent
Agent::Agent(Environment* env) : impl(new AgentImpl(env)) {}

Agent::~Agent() {
  delete impl;
}

bool Agent::Start(const char* path,
                  int port, bool wait) {
  return impl->Start(path, port, wait);
}

void Agent::Stop() {
  impl->Stop();
}

void Agent::addContext(v8::Local<v8::Context>& context, int group)
{
  impl->addContext(context, group);
}

void Agent::breakOnStart(std::string url)
{
  impl->breakOnStart(url);
}
