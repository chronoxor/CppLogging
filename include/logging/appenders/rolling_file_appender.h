/*!
    \file rolling_file_appender.h
    \brief Rolling file appender definition
    \author Ivan Shynkarenka
    \date 12.09.2016
    \copyright MIT License
*/

#ifndef CPPLOGGING_APPENDERS_ROLLING_FILE_APPENDER_H
#define CPPLOGGING_APPENDERS_ROLLING_FILE_APPENDER_H

#include "logging/appenders/file_appender.h"

namespace CppLogging {

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
class RollingFileAppender : public FileAppender
{
public:
    //! Rolling policy
    enum class RollingPolicy
    {
        SIZE,   //!< Size rolling policy
        YEAR,   //!< Year rolling policy
        MONTH,  //!< Monthly rolling policy
        DAY,    //!< Daily rolling policy
        HOUR,   //!< Hour rolling policy
        MINUTE, //!< Minute rolling policy
        SECOND, //!< Second rolling policy
    };

    //! Initialize the rolling file appender with a time-based policy
    /*!
         Time-based policy composes logging filename from the given pattern
         using the following placeholders:
         - {UtcDateTime}/{LocalDateTime} - converted to the UTC/local date & time (e.g. "1997-07-16T19-20-30Z"/"1997-07-16T19-20-30+01-00")
         - {UtcDate}/{LocalDate} - converted to the UTC/local date (e.g. "1997-07-16")
         - {Time}/{LocalTime} - converted to the UTC/local time (e.g. "19-20-30Z"/"19-20-30+01-00")
         - {UtcYear}/{LocalYear} - converted to the UTC/local four-digits year (e.g. "1997")
         - {UtcMonth}/{LocalMonth} - converted to the UTC/local two-digits month (e.g. "07")
         - {UtcDay}/{LocalDay} - converted to the UTC/local two-digits day (e.g. "16")
         - {UtcHour}/{LocalHour} - converted to the UTC/local two-digits hour (e.g. "19")
         - {UtcMinute}/{LocalMinute} - converted to the UTC/local two-digits minute (e.g. "20")
         - {UtcSecond}/{LocalSecond} - converted to the UTC/local two-digits second (e.g. "30")
         - {UtcTimezone}/{LocalTimezone} - converted to the UTC/local timezone suffix (e.g. "+01-00"/"Z")

         \param path - Logging path
         \param pattern - Logging pattern
         \param policy - Time rolling policy
         \param archive - Archivation flag (default is false)
         \param truncate - Truncate flag (default is false)
         \param auto_flush - Auto-flush flag (default is false)
    */
    explicit RollingFileAppender(const CppCommon::Path& path, const std::string& pattern, RollingPolicy policy, bool archive = false, bool truncate = false, bool auto_flush = false);
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
    */
    explicit RollingFileAppender(const CppCommon::Path& path, const std::string& filename, const std::string& extension, size_t size = 104857600, size_t backups = 10, bool archive = false, bool truncate = false, bool auto_flush = false);
    RollingFileAppender(const RollingFileAppender&) = delete;
    RollingFileAppender(RollingFileAppender&&) = default;
    virtual ~RollingFileAppender() = default;

    RollingFileAppender& operator=(const RollingFileAppender&) = delete;
    RollingFileAppender& operator=(RollingFileAppender&&) = default;

    //! Get the rolling file appender pattern
    const std::string& pattern() const noexcept { return _pattern; }

    // Implementation of Appender
    void AppendRecord(Record& record) override;

private:
    CppCommon::Path _path;
    RollingPolicy _policy;
    std::string _pattern;
    std::string _filename;
    std::string _extension;
    size_t _size;
    size_t _backups;
    size_t _written;
    bool _archive;

    static const std::string ARCHIVE_EXTENSION;

    //! Prepare the file for writing
    /*
        - Perform the rolling based on the current rolling strategy
        - If the file is opened and ready to write immediately returns true
        - If the last retry was earlier than 100ms immediately returns false
        - If the file is closed try to open it for writing, returns true/false

        \param size - Append size in bytes
    */
    bool PrepareFile(size_t size) override;

    //! Archive the file in a background thread
    /*
        \param file - File to archive
    */
    void ArchiveFile(const CppCommon::File& file);
};

} // namespace CppLogging

#endif // CPPLOGGING_APPENDERS_ROLLING_FILE_APPENDER_H
