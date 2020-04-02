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


/**
* @file AampDRMutils.cpp
* @brief DataStructures and methods for DRM license acquisition
*/

#include "AampDRMutils.h"

/**
 *  @brief		Default constructor for DrmData.
 *				NULL initialize data and dataLength.
 */
DrmData::DrmData() : data(NULL), dataLength(0)
{
}

/**
 *  @brief      Constructor for DrmData
 *              allocate memory and initialize data and
 *				dataLength with given params.
 *
 *  @param[in]	data - pointer to data to be copied.
 *  @param[in]	dataLength - length of data
 */
DrmData::DrmData(unsigned char *data, int dataLength) : data(NULL), dataLength(dataLength)
{
	this->data =(unsigned char*) malloc(dataLength + 1);
	memcpy(this->data,data,dataLength + 1);
}

/**
 *  @brief		Distructor for DrmData.
 *				Free memory (if any) allocated for data.
 */
DrmData::~DrmData()
{
	if(data != NULL)
	{
		free(data);
		data = NULL;
	}
}

/**
 *  @brief		Getter method for data.
 *
 *  @return		Returns pointer to data.
 */
unsigned char * DrmData::getData()
{
	return data;
}


/**
 *  @brief      Getter method for dataLength.
 *
 *  @return     Returns dataLength.
 */
int DrmData::getDataLength()
{
	return dataLength;
}

/**
 *  @brief		Updates DrmData with given data.
 *				Frees the existing data, before copying new data.
 *
 *  @param[in]	data - Pointer to data to be set.
 *  @param[in]	dataLength - length of data.
 *  @return		void.
 */
void DrmData::setData(unsigned char * data, int dataLength)
{
	if(this->data != NULL)
	{
		free(data);
	}
	this->data =  (unsigned char*) malloc(dataLength + 1);
	this->dataLength = dataLength;
	memcpy(this->data,data,dataLength + 1);
}

/**
 *  @brief      Appends DrmData with given data.
 *
 *  @param[in]  data - Pointer to data to be appended.
 *  @param[in]  dataLength - length of data.
 *  @return     void.
 */
void DrmData::addData(unsigned char * data, int dataLength)
{
	if(NULL == this->data)
	{
		this->setData(data,dataLength);
	}
	else
	{
		this->data = (unsigned char*) realloc(this->data, this->dataLength + dataLength + 1);
		assert(this->data);
		memcpy(&(this->data[this->dataLength]),data,dataLength + 1);
		this->dataLength = this->dataLength + dataLength;
	}
}
