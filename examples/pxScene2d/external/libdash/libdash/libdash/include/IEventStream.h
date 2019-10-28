/**
 *  @class      dash::mpd::IEventStream
 *  @brief      This interface is needed for accessing the attributes and elements of the <b><tt>EventStream</tt></b> element
 *              as specified in <em>ISO/IEC 23009-1, Part 1, 2014</em>, section 5.10, table 24
 *  @details    Each Period consists of one or more EventStreams. An Adaptation Set is described by an <b><tt>EventStream</tt></b> element.
 *              <b><tt>EventStream</tt></b> elements are contained in a <b><tt>Period</tt></b> element.\n\n
 *              An EventStream contains one or more Events.
 *  @see        dash::mpd::IDescriptor dash::mpd::IEvent
 *              dash::mpd::IMPDElement
 *
 */

#ifndef IEVENTSTREAM_H_
#define IEVENTSTREAM_H_

#include "config.h"

#include "IMPDElement.h"
#include "IEvent.h"

namespace dash
{
    namespace mpd
    {
        class IEventStream : public virtual IMPDElement
        {
            public:
                virtual ~IEventStream(){}

                /**
                 *  Returns a reference to a vector of pointers to dash::mpd::IEvent objects that specify the timed events appears in an EventStream.\n
                 *  For more details refer to the description in section 5.10.2 of <em>ISO/IEC 23009-1, Part 1, 2014</em>.
                 *  @return     a reference to a vector of pointers to dash::mpd::IEvent objects
                 */
                virtual const std::vector<IEvent *>&    GetEvents          ()  const = 0;


                /**
                 *  Returns a reference to a string that specifies a reference to external <tt><b>EventStream</b></tt> element.
                 *  @return     a reference to a string
                 */
                virtual const std::string&              GetXlinkHref       ()  const = 0;

                /**
                 *  Returns a reference to a string that specifies the processing instructions, which can be either \c \"onLoad\" or \c \"onRequest\".
                 *  @return     a reference to a string
                 */
                virtual const std::string&              GetXlinkActuate    ()  const = 0;

                /**
                 *  Returns a reference to a string that specifies a URI to identify the scheme. \n
                 *  The semantics of this element are specific to the scheme specified by this attribute.
                 *  The \c \@schemeIdUri may be a URN or URL. When a URL is used, it should also contain a month-date in the form
                 *  mmyyyy; the assignment of the URL must have been authorized by the owner of the domain name in that URL on
                 *  or very close to that date, to avoid problems when domain names change ownership.
                 *  @return     a reference to a string
                 */
                virtual const std::string&              GetSchemeIdUri     () const = 0;

                /**
                 *  Returns a reference to a string that specifies the value for the descriptor element. \n
                 *  The value space and semantics must be defined by the owners of the scheme identified in the \c \@schemeIdUri attribute.
                 *  @return     a reference to a string
                 */
                virtual const std::string&              GetValue           () const = 0;

                /**
                 *  Returns an integer representing a timescale that specifies the timescale in units per seconds
                 *  to be used for the derivation of different real-time duration values in the Segment Information.\n
                 *  \b NOTE:  This may be any frequency but typically is the media clock frequency of one of the media streams (or a positive integer multiple thereof).
                 *  @return     an unsigned integer
                 */
                virtual const uint32_t                  GetTimescale       () const = 0;
        };
    }
}

#endif /* IEVENTSTREAM_H_ */
