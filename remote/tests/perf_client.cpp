#include <rtRemote.h>
#include <rtRemoteConfig.h>
#include <rtRemoteEnvironment.h>
#include <rtGuid.h>

#include <getopt.h>
#include <unistd.h>

struct option longOptions[] =
{
	{ "test-id", required_argument, 0, 'i' },
	{ "count", required_argument, 0, 'c' },
	{ 0, 0, 0, 0 }
};


int main(int argc, char* argv[])
{

	int optionIndex;
	std::string testId;
	unsigned int count=100;
	rtError e;

	while (true)
	{
		int c = getopt_long(argc, argv, "i:c:", longOptions, &optionIndex);
		if (c == -1)
			break;

		switch (c)
		{
			case 'i':
				testId = optarg;
				break;
			case 'c':
				count = atoi(optarg);
				break;
		}
	}
	rtRemoteEnvironment* env = rtEnvironmentGetGlobal();

	rtObjectRef objectRef;
	e = rtRemoteInit(env);
	RT_ASSERT(e == RT_OK);

	e = rtRemoteLocateObject(env, testId.c_str(),objectRef );
	RT_ASSERT(e == RT_OK);

	for (unsigned int j = 0; j < count; ++j)
	{
		while ((e = rtRemoteLocateObject(env, testId.c_str(), objectRef)) != RT_OK)
		{
			rtLogInfo("failed to find %s:%s\n",testId.c_str(), rtStrError(e));
		}
		e = objectRef.set("height", j);
		rtLogInfo("set:%d", j);

		uint32_t n = objectRef.get<uint32_t>("height");
		rtLogInfo("get:%d", n);
		// RT_ASSERT(n == static_cast<uint32_t>(j));
		j++;
		rtLogInfo("sleeping for 1");
		sleep(1);
	}   

	rtLogInfo("server shutting down for %s", testId.c_str());
	return 0;
}
