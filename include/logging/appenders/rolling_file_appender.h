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
    File appender writes the given logging record into the file with
    the given file name. In case of any IO error this appender will
    lost the logging record, but try to recover from fail in a short
    interval of 100ms.

    Not thread-safe.
*/
class RollingFileAppender : public FileAppender
{
public:
    //! Rolling policy
    enum class RollingPolicy
    {
        SIZE    = 0,    //!< Size-based policy
        YEAR    = 1,    //!< Per year rolling policy
        MONTH   = 2,    //!< Monthly rolling policy
        DAY     = 3,    //!< Daily rolling policy
        HOUR    = 4,    //!< Daily rolling policy

        DIRECTORY = 2,      //!< Directory
        SYMLINK   = 3,      //!< Symbolic link
        BLOCK     = 4,      //!< Block device
        CHARACTER = 5,      //!< Character device
        FIFO      = 6,      //!< FIFO (named pipe)
        SOCKET    = 7,      //!< Socket
        UNKNOWN   = 8       //!< Unknown
    };

    //! Initialize the rolling appender with a size-based policy
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
         \param max_size - Rolling size limit in bytes (default is 100 megabytes)
         \param max_backups - Rolling backups count (default is 10)
         \param archive - Archivation flag (default is false)
         \param truncate - Truncate flag (default is false)
         \param auto_flush - Auto-flush flag (default is false)
    */
    explicit RollingFileAppender(const CppCommon::Path& path, const std::string& filename, const std::string& extension, size_t max_size = 104857600, size_t max_backups = 10, bool archive = false, bool truncate = false, bool auto_flush = false);
    RollingFileAppender(const RollingFileAppender&) = delete;
    RollingFileAppender(RollingFileAppender&&) = default;
    ~RollingFileAppender() { Flush(); }

    RollingFileAppender& operator=(const RollingFileAppender&) = delete;
    RollingFileAppender& operator=(RollingFileAppender&&) = default;

    // Implementation of Appender
    void AppendRecord(Record& record) override;
    void Flush() override;

private:
    std::string _pattern;
    size_t _max_size;
    size_t _max_backups;
    bool _archive;

    //! Prepare the file for writing
    /*
        - Perform the rolling based on the current rolling strategy
        - If the file is opened and ready to write immediately returns true
        - If the last retry was earlier than 100ms immediately returns false
        - If the file is closed try to open it for writing, returns true/false
    */
    bool PrepareFile() override;
};

} // namespace CppLogging

#endif // CPPLOGGING_APPENDERS_FILE_APPENDER_H
