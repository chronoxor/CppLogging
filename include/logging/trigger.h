/*!
    \file trigger.h
    \brief Logging trigger definition
    \author Ivan Shynkarenka
    \date 27.07.2016
    \copyright MIT License
*/

#ifndef CPPLOGGING_TRIGGER_H
#define CPPLOGGING_TRIGGER_H

#include "logging/filter.h"

#include <atomic>

namespace CppLogging {

//! Logging trigger
/*!
    Logging trigger is a special filter that allows to enable or disable logging.

    Thread-safe.
*/
class Trigger : public Filter
{
public:
    //! Initialize trigger with an initial logging state (enabled or disabled)
    /*!
         \param state - Initial logging state (default is true)
    */
    explicit Trigger(bool state = true) : _state(state) {}
    Trigger(const Trigger&) = delete;
    Trigger(Trigger&&) = delete;
    virtual ~Trigger() = default;

    Trigger& operator=(const Trigger&) = delete;
    Trigger& operator=(Trigger&&) = delete;

    //! Is logging enabled?
    bool IsEnabled() noexcept { return _state; }

    //! Enable logging
    void Enable() noexcept { _state = true; }
    //! Disable logging
    void Disable() noexcept { _state = false; }
    //! Toggle logging
    void Toggle() noexcept { _state = !_state; }

    // Implementation of Filter
    bool FilterRecord(Record& record) override
    { return _state; }

private:
    std::atomic<bool> _state;
};

} // namespace CppLogging

#endif // CPPLOGGING_TRIGGER_H
