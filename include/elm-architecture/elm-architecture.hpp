#pragma once
#include <algorithm>
#include <deque>
#include <future>
#include <iostream>
#include <variant>
#include <vector>

namespace elm_architecture {

// A command can either be a deferred command (shared_future) or
// invoked directly.
template <typename Model, typename Msg>
using command_type = std::variant<std::shared_future<Msg>, Msg>;

// The return type for the update functions, a new model and a
// list of actions to take after.
template <typename Model, typename Msg>
using return_type = std::tuple<Model, std::vector<command_type<Model, Msg>>>;

template <typename Model, typename Msg>
using view_return_type = std::vector<command_type<Model, Msg>>;

// Start the eventloop with a given list of initial actions to take
template <typename Model, typename Msg, typename Update, typename View = void>
auto
start_eventloop(const std::vector<command_type<Model, Msg>>& init = {}) {
    auto                                 model = Model {};
    std::deque<command_type<Model, Msg>> pending {init.begin( ), init.end( )};
    std::vector<std::shared_future<Msg>> in_progress;
    auto                                 sleep_duration_us     = 1024;
    const auto                           max_sleep_duration_us = sleep_duration_us * 16;

    while(pending.size( ) > 0 || in_progress.size( ) > 0) {
        // Step One: Apply all pending events and remove them
        while(pending.size( ) > 0) {
            const auto& item = pending.front( );
            if(std::holds_alternative<std::shared_future<Msg>>(item)) {
                in_progress.push_back(std::get<std::shared_future<Msg>>(item));
            } else {
                const Msg& msg                 = std::get<Msg>(item);
                const auto update_visitor      = [&model](const auto& msg) { return Update::update(model, msg); };
                return_type<Model, Msg> result = std::visit(update_visitor, msg);
                model                          = std::get<Model>(result);
                const auto& commands           = std::get<std::vector<command_type<Model, Msg>>>(result);
                std::copy(commands.begin( ), commands.end( ), std::back_inserter(pending));
                if constexpr(!std::is_same_v<View, void>) {
                    const auto view_visitor = [&model](const auto& msg) { return View::view(model, msg); };
                    std::vector<command_type<Model, Msg>> commands = std::visit(view_visitor, msg);
                    std::copy(commands.begin( ), commands.end( ), std::back_inserter(pending));
                }
            }
            pending.pop_front( );
        }

        // Step Two: Process all the finished IO tasks and push their resulting
        // messages to the message queue
        const auto remove_from   = std::remove_if(in_progress.begin( ), in_progress.end( ), [&](auto& future) {
            const auto status = future.wait_for(std::chrono::microseconds {sleep_duration_us});
            if(status == std::future_status::ready) {
                pending.push_back(future.get( ));
                return true;
            }
            return false;
        });
        const auto removed_tasks = std::distance(remove_from, in_progress.end( ));
        in_progress.erase(remove_from, in_progress.end( ));

        // Step Three: Adjust the sleep duration so that on higher loads we
        // sleep less (increasing throughput), and on lower loads we sleep more
        // (decreasing CPU load).
        sleep_duration_us = removed_tasks ? std::max(1, sleep_duration_us / 2)
                                          : std::min(sleep_duration_us * 2, max_sleep_duration_us);

        std::this_thread::yield( );
    }
}
} // namespace elm_architecture
