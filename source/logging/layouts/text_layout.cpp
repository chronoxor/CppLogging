/*!
    \file text_layout.cpp
    \brief Logging text layout implementation
    \author Ivan Shynkarenka
    \date 08.07.2016
    \copyright MIT License
*/

#include "logging/layouts/text_layout.h"

namespace CppLogging {

class TextLayout::Impl
{
public:
    Impl(const std::string& layout) : _buffer(1024)
    {
    }

    ~Impl()
    {
    }

    std::pair<void*, size_t> LayoutRecord(const Record& record)
    {
        // Return the serialized buffer
        return std::make_pair(_buffer.data(), _buffer.size());
    }

private:
    std::vector<uint8_t> _buffer;
};

TextLayout::TextLayout(const std::string& layout) : _pimpl(new Impl(layout))
{
}

TextLayout::~TextLayout()
{
}

std::pair<void*, size_t> TextLayout::LayoutRecord(const Record& record)
{
    return _pimpl->LayoutRecord(record);
}

} // namespace CppLogging
