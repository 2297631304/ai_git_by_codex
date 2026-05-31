#pragma once

#include <cstddef>
#include <source_location>
#include <sstream>
#include <stdexcept>
#include <string_view>

namespace esl {

template <typename VectorLike>
decltype(auto) checked_at(
    VectorLike& values,
    std::size_t index,
    std::string_view logical_name,
    const std::source_location location = std::source_location::current()) {
    if (index >= values.size()) {
        std::ostringstream oss;
        oss << "BOUNDS_ERROR vector=" << logical_name
            << " index=" << index
            << " size=" << values.size()
            << " at " << location.file_name()
            << ':' << location.line()
            << " in " << location.function_name();
        throw std::out_of_range(oss.str());
    }
    return values[index];
}

template <typename VectorLike>
decltype(auto) checked_at(
    const VectorLike& values,
    std::size_t index,
    std::string_view logical_name,
    const std::source_location location = std::source_location::current()) {
    if (index >= values.size()) {
        std::ostringstream oss;
        oss << "BOUNDS_ERROR vector=" << logical_name
            << " index=" << index
            << " size=" << values.size()
            << " at " << location.file_name()
            << ':' << location.line()
            << " in " << location.function_name();
        throw std::out_of_range(oss.str());
    }
    return values[index];
}

} // namespace esl
