#ifndef OpenCDMSessionAdapter_h
#define OpenCDMSessionAdapter_h


#include "AampDrmSession.h"
#include "aampoutputprotection.h"
#include <open_cdm.h>
#include <open_cdm_adapter.h>

using namespace std;

/**
 * @class AAMPOCDMSession
 * @brief Open CDM DRM session
 */

class Event {
private:
	bool signalled; //TODO: added to handle the events fired before calling wait, need to recheck
	pthread_mutex_t lock;
	pthread_cond_t condition;
public:
	Event() : signalled(false), lock(PTHREAD_MUTEX_INITIALIZER), condition(PTHREAD_COND_INITIALIZER) {
		pthread_cond_init(&condition, NULL);
		pthread_mutex_init(&lock, NULL);
	}
	virtual ~Event() {
		pthread_cond_destroy(&condition);
		pthread_mutex_destroy(&lock);
	}

	inline bool wait(const uint32_t waitTime)
	{
		int ret = 0;
		pthread_mutex_lock(&lock);
		if (!signalled) {
			if (waitTime == 0) {
				ret = pthread_cond_wait(&condition, &lock);
			} else {
				struct timespec time;
				clock_gettime(CLOCK_MONOTONIC, &time);

				time.tv_nsec += ((waitTime % 1000) * 1000 * 1000);
				time.tv_sec += (waitTime / 1000) + (time.tv_nsec / 1000000000);
				time.tv_nsec = time.tv_nsec % 1000000000;

				ret = pthread_cond_timedwait(&condition, &lock, &time);

			}
		}

		signalled = false;
		pthread_mutex_unlock(&lock);

		return ((ret == 0)? true: false);
	}

	inline void signal()
        {
		pthread_mutex_lock(&lock);
		signalled = true;
		pthread_cond_broadcast(&condition);
	        pthread_mutex_unlock(&lock);
        }
};

class AAMPOCDMSession : public AampDrmSession
{

private:
	pthread_mutex_t decryptMutex;

	KeyState m_eKeyState;

	OpenCDMSession* m_pOpenCDMSession;
	struct OpenCDMAccessor* m_pOpenCDMSystem;
	OpenCDMSessionCallbacks m_OCDMSessionCallbacks;
	AampOutputProtection* m_pOutputProtection;

	std::string m_challenge;
	uint16_t m_challengeSize;

	std::string m_destUrl;
	KeyStatus m_keyStatus;

	Event m_challengeReady;
	Event m_keyStatusReady;
	string m_sessionID;

	uint8_t* m_keyId;
	int32_t m_keyLength;

public:
	void processOCDMChallenge(const char destUrl[], const uint8_t challenge[], const uint16_t challengeSize);
	void keyUpdatedOCDM(const uint8_t key[], const uint8_t keySize);

private:
	void initAampDRMSystem();

public:
    AAMPOCDMSession(string& keySystem);
	~AAMPOCDMSession();
    AAMPOCDMSession(const AAMPOCDMSession&) = delete;
	AAMPOCDMSession& operator=(const AAMPOCDMSession&) = delete;
	void generateAampDRMSession(const uint8_t *f_pbInitData,
			uint32_t f_cbInitData);
	DrmData * aampGenerateKeyRequest(string& destinationURL);
	int aampDRMProcessKey(DrmData* key);
	int decrypt(GstBuffer* keyIDBuffer, GstBuffer* ivBuffer, GstBuffer* buffer, unsigned subSampleCount, GstBuffer* subSamplesBuffer);
	KeyState getState();
	void clearDecryptContext();
	void setKeyId(const char* keyId, int32_t keyLength);
};

#endif