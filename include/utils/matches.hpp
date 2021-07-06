#pragma once

#include <variant>

template <typename... Tp, typename... Fs>
inline decltype(auto) matches(const std::variant<Tp...>& val, Fs&&... fs) {
    struct overloaded : Fs... { using Fs::operator()...; };
    return std::visit(overloaded { std::forward<Fs>(fs)... }, val);
}
