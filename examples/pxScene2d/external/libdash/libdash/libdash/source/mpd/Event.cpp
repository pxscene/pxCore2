/*
 * Event.cpp
 *****************************************************************************

 *****************************************************************************/

#include <cstdlib>
#include "Event.h"

using namespace dash::mpd;

Event::Event    () :
                    presentationTime(0),
                    duration(0),
                    id (0)
{
}
Event::~Event   ()
{
}


const uint64_t                    Event::GetPresentationTime   () const
{
	return this->presentationTime;
}
void                              Event::SetPresentationTime   (uint64_t presentationTime)
{
	this->presentationTime =  presentationTime;
}

const uint32_t                    Event::GetDuration           () const
{
	return this->duration;
}
void                              Event::SetDuration           (uint32_t duration)
{
	this->duration = duration;
}

const uint32_t                    Event::GetId                 () const
{
	return  this->id;
}
void                              Event::SetId                 (uint32_t id)
{
	this->id = id;
}

