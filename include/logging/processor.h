/*!
    \file processor.h
    \brief Logging processor interface definition
    \author Ivan Shynkarenka
    \date 26.07.2016
    \copyright MIT License
*/

#ifndef CPPLOGGING_PROCESSOR_H
#define CPPLOGGING_PROCESSOR_H

#include "logging/appenders.h"
#include "logging/filters.h"
#include "logging/layouts.h"

namespace CppLogging {

//! Logging processor interface
/*!
    Logging processor takes an instance of a single logging record
    and process it though all child filters, layouts and appenders.

    Not thread-safe.

    \see SyncProcessor
    \see AsyncProcessor
    \see BufferedProcessor
    \see ExclusiveProcessor
*/
class Processor : public Element
{
public:
    //! Initialize logging processor with a given layout interface
    /*!
         \param layout - Logging layout interface
    */
    explicit Processor(const std::shared_ptr<Layout>& layout) : _layout(layout) {}
    Processor(const Processor&) = delete;
    Processor(Processor&&) noexcept = delete;
    virtual ~Processor();

    Processor& operator=(const Processor&) = delete;
    Processor& operator=(Processor&&) noexcept = delete;

    //! Get the logging processor layout
    std::shared_ptr<Layout>& layout() noexcept { return _layout; }
    //! Get collection of child filters
    std::vector<std::shared_ptr<Filter>>& filters() noexcept { return _filters; }
    //! Get collection of child appenders
    std::vector<std::shared_ptr<Appender>>& appenders() noexcept { return _appenders; }
    //! Get collection of child processors
    std::vector<std::shared_ptr<Processor>>& processors() noexcept { return _processors; }

    //! Is the logging processor started?
    bool IsStarted() const noexcept override { return _started; }

    //! Start the logging processor
    /*!
         \return 'true' if the logging processor was successfully started, 'false' if the logging processor failed to start
    */
    bool Start() override;
    //! Stop the logging processor
    /*!
         \return 'true' if the logging processor was successfully stopped, 'false' if the logging processor failed to stop
    */
    bool Stop() override;

    //! Filter the given logging record
    /*!
         \param record - Logging record
         \return 'true' if the logging record should be processed, 'false' if the logging record was filtered out
    */
    virtual bool FilterRecord(Record& record);

    //! Process the given logging record through all child filters, layouts and appenders
    /*!
         Default behavior of the method will take the given logging record and process it
         in the following sequence:
         - all filters (if any present)
         - all layouts (if any present)
         - all appenders (if any present)
         - all sub processors (if any present)

         \param record - Logging record
         \return 'true' if the logging record was successfully processed, 'false' if the logging record was filtered out
    */
    virtual bool ProcessRecord(Record& record);

    //! Flush the current logging processor
    /*!
         Default behavior of the method will flush in the following sequence:
         - all appenders (if any present)
         - all sub processors (if any present)
    */
    virtual void Flush();

protected:
    std::atomic<bool> _started{true};
    std::shared_ptr<Layout> _layout;
    std::vector<std::shared_ptr<Filter>> _filters;
    std::vector<std::shared_ptr<Appender>> _appenders;
    std::vector<std::shared_ptr<Processor>> _processors;
};

} // namespace CppLogging

#endif // CPPLOGGING_PROCESSOR_H
