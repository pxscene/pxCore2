#ifndef SRC_INSPECTOR_AGENT_H_
#define SRC_INSPECTOR_AGENT_H_

#include <v8.h>
#include <headers.h>

class AgentImpl;

class Agent {
 public:
  explicit Agent(Environment* env);
  ~Agent();

  // Start the inspector agent thread
  bool Start(const char* path, int port, bool wait);
  // Stop the inspector agent
  void Stop();

  void addContext(v8::Local<v8::Context>&, int);
  void breakOnStart(std::string url);
 private:
  AgentImpl* impl;
};
#endif  // SRC_INSPECTOR_AGENT_H_
