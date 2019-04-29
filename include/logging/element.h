/*!
    \file element.h
    \brief Logging element interface definition
    \author Ivan Shynkarenka
    \date 26.07.2016
    \copyright MIT License
*/

#ifndef CPPLOGGING_ELEMENT_H
#define CPPLOGGING_ELEMENT_H

namespace CppLogging {

//! Logging element interface
/*!
    Logging filter takes an instance of a single logging record and
    performs some checks to detect if the record should be filered
    out and not processed anymore.

    \see Appender
    \see Filter
    \see Layout
    \see Processor
*/
class Element
{
public:
    //! Is the logging element started?
    virtual bool IsStarted() const noexcept { return true; }

    //! Start the logging element
    /*!
         \return 'true' if the logging element was successfully started, 'false' if the logging element failed to start
    */
    virtual bool Start() { return true; }
    //! Stop the logging element
    /*!
         \return 'true' if the logging element was successfully stopped, 'false' if the logging element failed to stop
    */
    virtual bool Stop() { return true; }
};

} // namespace CppLogging

#endif // CPPLOGGING_ELEMENT_H
