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
*/
class Processor
{
public:
    //! Initialize logging processor with a given layout interface
    /*!
         \param layout - Logging layout interface
    */
    explicit Processor(const std::shared_ptr<Layout>& layout) : _layout(layout) {}
    Processor(const Processor&) = default;
    Processor(Processor&&) noexcept = default;
    virtual ~Processor();

    Processor& operator=(const Processor&) = default;
    Processor& operator=(Processor&&) noexcept = default;

    //! Get the logging processor layout
    std::shared_ptr<Layout>& layout() noexcept { return _layout; }
    //! Get collection of child pre-filters
    std::vector<std::shared_ptr<Filter>>& pre_filters() noexcept { return _pre_filters; }
    //! Get collection of child post-filters
    std::vector<std::shared_ptr<Filter>>& post_filters() noexcept { return _post_filters; }
    //! Get collection of child appenders
    std::vector<std::shared_ptr<Appender>>& appenders() noexcept { return _appenders; }
    //! Get collection of child processors
    std::vector<std::shared_ptr<Processor>>& processors() noexcept { return _processors; }

    //! Pre-Filter the given logging record
    /*!
         \param record - Logging record
         \return 'true' if the logging record should be processed, 'false' if the logging record was filtered out
    */
    virtual bool PreFilterRecord(Record& record);
    //! Post-Filter the given logging record
    /*!
         \param record - Logging record
         \return 'true' if the logging record should be processed, 'false' if the logging record was filtered out
    */
    virtual bool PostFilterRecord(Record& record);

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

private:
    std::shared_ptr<Layout> _layout;
    std::vector<std::shared_ptr<Filter>> _pre_filters;
    std::vector<std::shared_ptr<Filter>> _post_filters;
    std::vector<std::shared_ptr<Appender>> _appenders;
    std::vector<std::shared_ptr<Processor>> _processors;
};

} // namespace CppLogging

#endif // CPPLOGGING_PROCESSOR_H
