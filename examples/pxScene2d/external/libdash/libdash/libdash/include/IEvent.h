/**
 *  @class      dash::mpd::IEvent
 *  @brief      This interface is needed for accessing the attributes and elements of the <b><tt>Event</tt></b> element
 *              as specified in <em>ISO/IEC 23009-1, Part 1, 2014</em>, section 5.10.2, table 24
 *  @details    Each EventStream consists of one or more Events.
 *              <b><tt>Event</tt></b> elements are contained in a <b><tt>EventStream</tt></b> element.\n\n
 *
 */

#ifndef IEVENTS_H_
#define IEVENTS_H_

#include "config.h"

#include "IMPDElement.h"

namespace dash
{
    namespace mpd
    {
        class IEvent : public virtual IMPDElement
        {
            public:
                virtual ~IEvent(){}

                /**
                 *  Returns an integer that specifies the presentation time of the Event.\n
                 *  The value of the presentation time in seconds is the division of the value of this attribute and the value of the \c \@timescale attribute.\n
                 *  If not present, the value of the presentation time is 0.
                 *  @return     an unsigned integer
                 */
                virtual const uint64_t                    GetPresentationTime   ()  const = 0;

                /**
                 *  Returns an integer that specifies the duration of the Event.\n
                 *  The value of the duration in seconds is the division of the value of this attribute and the value of the \c \@timescale attribute.\n
                 *  If not present, the value of the duration is unknown.
                 *  @return     an unsigned integer
                 */
                virtual const uint32_t                    GetDuration           ()  const = 0;

                /**
                 *  Returns an integer that specifies the identifier of the Event.\n
                 *  @return     an unsigned integer
                 */
                virtual const uint32_t                    GetId                 ()  const = 0;
        };
    }
}

#endif /* IEVENTS_H_ */
