/*

pxCore Copyright 2005-2018 John Robinson

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
  bool Start(int port, bool wait);
  // Stop the inspector agent
  void Stop();

  void addContext(v8::Local<v8::Context>&, int);
  void breakOnStart(std::string url);
 private:
  AgentImpl* impl;
};
#endif  // SRC_INSPECTOR_AGENT_H_
