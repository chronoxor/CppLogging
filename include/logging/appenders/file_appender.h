/*!
    \file file_appender.h
    \brief File appender definition
    \author Ivan Shynkarenka
    \date 07.09.2016
    \copyright MIT License
*/

#ifndef CPPLOGGING_APPENDERS_FILE_APPENDER_H
#define CPPLOGGING_APPENDERS_FILE_APPENDER_H

#include "logging/appender.h"

#include "filesystem/filesystem.h"

namespace CppLogging {

//! File appender
/*!
    File appender writes the given logging record into the file with
    the given file name. In case of any IO error this appender will
    lost the logging record, but try to recover from fail in a short
    interval of 100ms.

    Not thread-safe.
*/
class FileAppender : public Appender
{
public:
    //! Initialize the appender with a given file, truncate/append and auto-flush flags
    /*!
         \param file - File
         \param truncate - Truncate flag (default is false)
         \param auto_flush - Auto-flush flag (default is false)
    */
    explicit FileAppender(const CppCommon::File& file, bool truncate = false, bool auto_flush = false);
    FileAppender(const FileAppender&) = delete;
    FileAppender(FileAppender&&) = default;
    virtual ~FileAppender() = default;

    FileAppender& operator=(const FileAppender&) = delete;
    FileAppender& operator=(FileAppender&&) = default;

    // Implementation of Appender
    void AppendRecord(Record& record) override;
    void Flush() override;

private:
    CppCommon::Timestamp _retry;
    CppCommon::File _file;
    bool _truncate;
    bool _auto_flush;

    //! Prepare the file for writing
    /*
        - If the file is opened and ready to write immediately returns true
        - If the last retry was earlier than 100ms immediately returns false
        - If the file is closed try to open it for writing, returns true/false
    */
    bool PrepareFile();
};

} // namespace CppLogging

/*! \example file.cpp File logger example */

#endif // CPPLOGGING_APPENDERS_FILE_APPENDER_H
