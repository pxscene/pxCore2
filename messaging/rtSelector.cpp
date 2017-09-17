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
#include "rtSelector.h"
#include "rtLog.h"

static void
rtPushFd(fd_set* set, int fd, int& max)
{
  FD_SET(fd, set);
  if (fd > max)
    max = fd;
}

rtError
rtSelector::registerSelectable(rtSelectable* obj, rtSelectorOps ops, void* argp)
{
  rtSelectorKey key;
  key.selectable = obj;
  key.arg = argp;
  // user should not supply Read & Accept
  // user should not supply Write & Connect
  key.ops_config = ops;
  m_keys.push_back(key);
  return RT_OK;
}

rtError
rtSelector::select()
{
  int max_fd = -1;

  fd_set read_fds;
  fd_set write_fds;
  fd_set error_fds;

  FD_ZERO(&read_fds);
  FD_ZERO(&write_fds);
  FD_ZERO(&error_fds);

  for (rtSelectorKeyList::iterator begin = m_keys.begin(); begin != m_keys.end(); ++begin)
  {
      begin->ops_set = rtSelectorOps_None;
      rtPushFd(&error_fds, begin->selectable->handle(), max_fd);

      if (((begin->ops_config & rtSelectorOps_Read) == rtSelectorOps_Read) ||
          ((begin->ops_config & rtSelectorOps_Accept) == rtSelectorOps_Accept))
      {
        rtPushFd(&read_fds, begin->selectable->handle(), max_fd);
      }

      if (((begin->ops_config & rtSelectorOps_Write) == rtSelectorOps_Write) ||
          ((begin->ops_config & rtSelectorOps_Connect) == rtSelectorOps_Connect))
      {
        rtPushFd(&write_fds, begin->selectable->handle(), max_fd);
      }
  }

  rtError e = RT_OK;

  struct timeval timeout;
  timeout.tv_sec = 2;
  timeout.tv_usec = 0;

  int ret = ::select(max_fd + 1, &read_fds, &write_fds, &error_fds, &timeout);
  if (ret == -1)
  {
    e = rtErrorFromErrno(errno);
    rtLogWarn("select failed: %s", rtStrError(e));
    return e;
  }
  else if (ret == 0)
  {
    e = rtErrorFromErrno(ETIMEDOUT);
  }
  else
  {
    m_setKeys = m_keys;
    for (rtSelectorKeyList::iterator begin = m_setKeys.begin(); begin != m_setKeys.end(); ++begin)
    {
      int fd = begin->selectable->handle();
      if (FD_ISSET(fd, &read_fds))
      {
        if ((begin->ops_config & rtSelectorOps_Accept) == rtSelectorOps_Accept)
          e = begin->selectable->onReadyAccept(begin->arg);
        else
          e = begin->selectable->onReadyRead(begin->arg);
      }
      if (FD_ISSET(fd, &write_fds))
      {
        if ((begin->ops_config & rtSelectorOps_Connect) == rtSelectorOps_Connect)
          e = begin->selectable->onReadyConnect(begin->arg);
        else
          e = begin->selectable->onReadyWrite(begin->arg);
      }
      if (FD_ISSET(fd, &error_fds))
      {
        rtError select_error = RT_FAIL;
        e = begin->selectable->onError(select_error, begin->arg);
      }
    }
  }

  return e;
}
