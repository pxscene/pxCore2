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
