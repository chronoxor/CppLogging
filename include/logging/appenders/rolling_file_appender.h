/*!
    \file rolling_file_appender.h
    \brief Rolling file appender definition
    \author Ivan Shynkarenka
    \date 12.09.2016
    \copyright MIT License
*/

#ifndef CPPLOGGING_APPENDERS_ROLLING_FILE_APPENDER_H
#define CPPLOGGING_APPENDERS_ROLLING_FILE_APPENDER_H

#include "logging/appender.h"

#include "filesystem/filesystem.h"

#include <memory>

namespace CppLogging {

//! Time rolling policy
enum class TimeRollingPolicy
{
    YEAR,       //!< Year rolling policy
    MONTH,      //!< Monthly rolling policy
    DAY,        //!< Daily rolling policy
    HOUR,       //!< Hour rolling policy
    MINUTE,     //!< Minute rolling policy
    SECOND      //!< Second rolling policy
};

//! Rolling file appender
/*!
    Rolling file appender writes the given logging record into the file
    and performs file rolling operation depends on the given policy.
    In case of any IO error this appender will lost the logging record,
    but try to recover from fail in a short interval of 100ms.

    Time-based rolling policy will create a new logging file to write
    into using a special pattern (contains date & time placeholders).

    Size-based rolling policy will create a new logging file to write
    when the current file size exceeded size limit. Logging backups
    are indexed and its count could be limited as well.

    It is possible to enable archivation of the logging backups in a
    background thread.

    Not thread-safe.
*/
class RollingFileAppender : public Appender
{
    friend class SizePolicyImpl;
    friend class TimePolicyImpl;

public:
    //! Initialize the rolling file appender with a time-based policy
    /*!
         Time-based policy composes logging filename from the given pattern
         using the following placeholders:
         - {UtcDateTime} / {LocalDateTime} - converted to the UTC/local date & time (e.g. "1997-07-16T192030Z" / "1997-07-16T192030+0100")
         - {UtcDate} / {LocalDate} - converted to the UTC/local date (e.g. "1997-07-16")
         - {UtcTime} / {LocalTime} - converted to the UTC/local time (e.g. "192030Z" / "192030+0100")
         - {UtcYear} / {LocalYear} - converted to the UTC/local four-digits year (e.g. "1997")
         - {UtcMonth} / {LocalMonth} - converted to the UTC/local two-digits month (e.g. "07")
         - {UtcDay} / {LocalDay} - converted to the UTC/local two-digits day (e.g. "16")
         - {UtcHour} / {LocalHour} - converted to the UTC/local two-digits hour (e.g. "19")
         - {UtcMinute} / {LocalMinute} - converted to the UTC/local two-digits minute (e.g. "20")
         - {UtcSecond} / {LocalSecond} - converted to the UTC/local two-digits second (e.g. "30")
         - {UtcTimezone} / {LocalTimezone} - converted to the UTC/local timezone suffix (e.g. "Z" / "+0100")

         \param path - Logging path
         \param policy - Time-based rolling policy (default is TimeRollingPolicy::DAY)
         \param pattern - Logging pattern (default is "{UtcDateTime}.log")
         \param archive - Archivation flag (default is false)
         \param truncate - Truncate flag (default is false)
         \param auto_flush - Auto-flush flag (default is false)
         \param auto_start - Auto-start flag (default is true)
    */
    explicit RollingFileAppender(const CppCommon::Path& path, TimeRollingPolicy policy = TimeRollingPolicy::DAY, const std::string& pattern = "{UtcDateTime}.log", bool archive = false, bool truncate = false, bool auto_flush = false, bool auto_start = true);
    //! Initialize the rolling file appender with a size-based policy
    /*!
         Size-based policy for 5 backups works in a following way:

         example.log   -> example.1.log
         example.1.log -> example.2.log
         example.2.log -> example.3.log
         example.3.log -> example.4.log
         example.4.log -> example.5.log
         example.5.log -> remove

         \param path - Logging path
         \param filename - Logging filename
         \param extension - Logging extension
         \param size - Rolling size limit in bytes (default is 100 megabytes)
         \param backups - Rolling backups count (default is 10)
         \param archive - Archivation flag (default is false)
         \param truncate - Truncate flag (default is false)
         \param auto_flush - Auto-flush flag (default is false)
         \param auto_start - Auto-start flag (default is true)
    */
    explicit RollingFileAppender(const CppCommon::Path& path, const std::string& filename, const std::string& extension, size_t size = 104857600, size_t backups = 10, bool archive = false, bool truncate = false, bool auto_flush = false, bool auto_start = true);
    RollingFileAppender(const RollingFileAppender&) = delete;
    RollingFileAppender(RollingFileAppender&& appender) = delete;
    virtual ~RollingFileAppender();

    RollingFileAppender& operator=(const RollingFileAppender&) = delete;
    RollingFileAppender& operator=(RollingFileAppender&& appender) = delete;

    // Implementation of Appender
    bool IsStarted() const noexcept override;
    bool Start() override;
    bool Stop() override;
    void AppendRecord(Record& record) override;
    void Flush() override;

protected:
    //! Initialize archivation thread handler
    /*!
         This handler can be used to initialize priority or affinity of the archivation thread.
    */
    virtual void onArchiveThreadInitialize() {}
    //! Cleanup archivation thread handler
    /*!
         This handler can be used to cleanup priority or affinity of the archivation thread.
    */
    virtual void onArchiveThreadCleanup() {}

private:
    class Impl;
    std::unique_ptr<Impl> _pimpl;
};

} // namespace CppLogging

#endif // CPPLOGGING_APPENDERS_ROLLING_FILE_APPENDER_H
