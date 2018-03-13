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

#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <assert.h>
#include <rtRemote.h>

static char const* objectName = "rtTest";

class rtTest : public rtObject {
public:
    rtDeclareObject(rtTest, rtObject);
    rtProperty(num, num, setNum, int);
    rtTest()
        : m_num(0) {
    }
    rtError num(int &num) const {
        num = m_num;
        return RT_OK;
    }
    rtError setNum(int const& num) {
        m_num = num;
        return RT_OK;
    }
private:
    int m_num;
    rtEmitRef m_emit;
};
rtDefineObject    (rtTest, rtObject);
rtDefineProperty  (rtTest, num);

int main(int argc, char *argv[]) {
    rtError rc;
    rc = rtRemoteInit();
    assert(rc == RT_OK);

    rtObjectRef objectRef;

    if (argc > 1) {
        objectRef = new rtTest();
        rc = rtRemoteRegisterObject(objectName, objectRef);
        assert(rc == RT_OK);
        for(;;) usleep(100000);
    } else {
        do {
            rc = rtRemoteLocateObject(objectName, objectRef);
        } while (rc != RT_OK);

        int limit = getenv("RT_PERF_LIMIT") ? atoi(getenv("RT_PERF_LIMIT")) : 10000;

        for (int i=0; i<limit; ++i) {
            objectRef.set("num", i);
        }
    }

    objectRef = nullptr;

    rtRemoteShutdown();

    return 0;
}
