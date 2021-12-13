/*!
    \file hash_layout.h
    \brief Hash layout definition
    \author Ivan Shynkarenka
    \date 12.12.2021
    \copyright MIT License
*/

#ifndef CPPLOGGING_LAYOUTS_HASH_LAYOUT_H
#define CPPLOGGING_LAYOUTS_HASH_LAYOUT_H

#include "logging/layout.h"

namespace CppLogging {

//! Hash layout
/*!
    Hash layout performs simple memory copy operation to convert
    the given logging record into the plane raw buffer. Logging
    message is stored as a 32-bit hash of the message string.

    Hash algorithm is 32-bit FNV-1a string hashing.

    Thread-safe.
*/
class HashLayout : public Layout
{
public:
    HashLayout() = default;
    HashLayout(const HashLayout&) = delete;
    HashLayout(HashLayout&&) = delete;
    virtual ~HashLayout() = default;

    HashLayout& operator=(const HashLayout&) = delete;
    HashLayout& operator=(HashLayout&&) = delete;

    //! Hash the given string message using FNV-1a hashing algorithm
    /*!
         FNV-1a string hashing is the fast non-cryptographic hash function created by
         Glenn Fowler, Landon Curt Noll, and Kiem-Phong Vo.

         https://en.wikipedia.org/wiki/Fowler%E2%80%93Noll%E2%80%93Vo_hash_function

         \param message - Message string
         \return Calculated 32-bit hash value of the message
    */
    static uint32_t Hash(std::string_view message);

    // Implementation of Layout
    void LayoutRecord(Record& record) override;
};

} // namespace CppLogging

#endif // CPPLOGGING_LAYOUTS_HASH_LAYOUT_H
