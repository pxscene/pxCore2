/* 
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include "rtError.h"
#include "rtIpAddress.h"
#include "rtLog.h"
#include "rtSelector.h"
#include "rtSocket.h"

#include <vector>
#include <getopt.h>

class rtBrokerListener : public rtTcpListener
{
public:
  rtBrokerListener(rtIpEndPoint const& ep)
    : rtTcpListener(ep)
  {
  }

  virtual ~rtBrokerListener()
  {
  }

  virtual rtError onReadyAccept(void* argp)
  {
    return RT_OK;
  }
};

int main(int argc, char* argv[])
{
  rtLogLevel log_level = RT_LOG_INFO;

  typedef std::vector<rtTcpListener *> listener_list;
  listener_list listeners;

  rtLogSetLevel(RT_LOG_DEBUG);
  rtLogSetLogHandler(NULL);

  while (1)
  {
    static struct option opts[] = 
    {
      {"listen", required_argument, 0, 'l'},
      {"log-level", required_argument, 0, 10000},
      { 0, 0, 0, 0}
    };

    int index = 0;

    int c = getopt_long(argc, argv, "l:", opts, &index);
    if (c == -1)
      break;

    switch (c)
    {
      case 'l':
      {
        rtIpEndPoint ep = rtIpEndPoint::fromString(optarg);
        rtTcpListener* listener = new rtBrokerListener(ep);
        listeners.push_back(listener);
      }
      break;

      case 10000:
      {
        log_level = rtLogLevelFromString(optarg);
      }
      break;

      case '?':
      exit(0);
      break;

      default:
        RT_ASSERT(false);
        break;
    }
  }

  rtLogSetLevel(log_level);
  rtLogSetLogHandler(NULL);

  rtSelector selector;

  for (listener_list::iterator itr = listeners.begin(); itr != listeners.end(); ++itr)
  {
    rtError e = (*itr)->start(false);
    if (e != RT_OK)
    {
      rtLogError("failed to start listener on %s. %s", (*itr)->localEndPoint().toString().c_str(),
        rtStrError(e));
    }
    else
    {
      e = selector.registerSelectable(*itr, rtSelectorOps_Accept);
      rtLogInfo("listener is register: %s", rtStrError(e));
    }
  }

  while (true)
  {
    rtError e = selector.select();
    if (e != RT_OK)
    {
      rtLogWarn("select:%s", rtStrError(e));
      continue;
    }
  }

  return 0;
}
