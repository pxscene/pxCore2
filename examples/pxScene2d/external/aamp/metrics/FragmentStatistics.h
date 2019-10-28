/*
 * If not stated otherwise in this file or this component's license file the
 * following copyright and licenses apply:
 *
 * Copyright 2018 RDK Management
 *
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

#ifndef __FRAGMENT_STATISTICS_H__
#define __FRAGMENT_STATISTICS_H__

#include "HTTPStatistics.h"


class CFragmentStatistics
{
private:
	CHTTPStatistics * pNormalFragmentStat;

	CHTTPStatistics * pInitFragmentStat;

	// last failure url ( could be fragment/ init fragment
	// URL is added to indicate CDN server info espacially in failure case.
	std::string m_url;
public:
	CFragmentStatistics() : pInitFragmentStat(NULL), pNormalFragmentStat(NULL), m_url()
	{

	}

	CFragmentStatistics(const CFragmentStatistics& newObj)  : CFragmentStatistics()
	{
		if(newObj.pInitFragmentStat)
		{
			pInitFragmentStat = new CHTTPStatistics(*newObj.pInitFragmentStat);
		}

		if(newObj.pNormalFragmentStat)
		{
			pNormalFragmentStat = new CHTTPStatistics(*newObj.pNormalFragmentStat);
		}

		m_url = newObj.m_url;

	}

	CFragmentStatistics& operator=(const CFragmentStatistics& newObj)
	{
		if(newObj.pInitFragmentStat)
		{
			pInitFragmentStat = GetInitFragmentStat(); // Allocate if required

			*pInitFragmentStat = *newObj.pInitFragmentStat;
		}
		else
		{
			if(pInitFragmentStat)
			{
				delete pInitFragmentStat;
				pInitFragmentStat = NULL;
			}
		}

		if(newObj.pNormalFragmentStat)
		{
			pNormalFragmentStat = GetNormalFragmentStat(); // Allocate if required

			*pNormalFragmentStat = *newObj.pNormalFragmentStat;
		}
		else
		{
			if(pNormalFragmentStat)
			{
				delete pNormalFragmentStat;
				pNormalFragmentStat = NULL;
			}
		}


		m_url = newObj.m_url;

		return *this;
	}

	/**
	 *   @brief  gets Init Fragment stats object, Creates if not created
	 *
	 *   @param[in]  None
     *
	 *   @return None
	 */
	CHTTPStatistics * GetInitFragmentStat()
	{
		if(!pInitFragmentStat)
		{
			pInitFragmentStat = new CHTTPStatistics();
		}
		return pInitFragmentStat;
	}

	/**
	 *   @brief  gets Normal Fragment stats object, Creates if not created
	 *
	 *   @param[in]  None
     *
	 *   @return None
	 */
	CHTTPStatistics * GetNormalFragmentStat()
	{
		if(!pNormalFragmentStat)
		{
			pNormalFragmentStat = new CHTTPStatistics();
		}
		return pNormalFragmentStat;
	}

	/**
	 *   @brief  Default Destructor
	 *
	 *   @param[in]  None
     *
	 *   @return None
	 */
	~CFragmentStatistics()
	{
		if(pInitFragmentStat)
		{
			delete pInitFragmentStat;
		}

		if(pNormalFragmentStat)
		{
			delete pNormalFragmentStat;
		}
	}

	/**
	 *   @brief  Sets the Url
	 *
	 *   @param[in]  std::string url to be set
	 *
	 *   @return None
	 */
	void SetUrl(std::string & url)
	{
		m_url = url;
	}

	/**
	 *   @brief  Converts class object data to Json object
	 *
	 *   @param[in]  NONE
     *
	 *   @return cJSON pointer
	 */
	cJSON * ToJson() const;
};




#endif /* __FRAGMENT_STATISTICS_H__ */
