#pragma once

#include <msg/handler.hpp>

#include <stdx/tuple.hpp>
#include <stdx/tuple_algorithms.hpp>

namespace msg {
template <typename Callbacks, typename MsgBase, typename... ExtraCallbackArgs>
struct handler_builder {
    Callbacks callbacks;

    template <typename... Ts> [[nodiscard]] constexpr auto add(Ts... ts) {
        auto new_callbacks =
            stdx::tuple_cat(callbacks, stdx::make_tuple(ts...));
        using new_callbacks_t = decltype(new_callbacks);
        return handler_builder<new_callbacks_t, MsgBase, ExtraCallbackArgs...>{
            new_callbacks};
    }

    template <typename BuilderValue> constexpr static auto build() {
        return handler<Callbacks, MsgBase, ExtraCallbackArgs...>{
            BuilderValue::value.callbacks};
    }
};

} // namespace msg
