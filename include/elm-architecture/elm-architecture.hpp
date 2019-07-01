#pragma once
#include <deque>
#include <future>
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

// Start the eventloop with a given list of initial actions to take
template <typename Model, typename Msg, typename Update>
auto
start_eventloop(const std::vector<command_type<Model, Msg>>& init = {}) {
    auto                                 model = Model {};
    std::deque<command_type<Model, Msg>> pending{init.begin(), init.end()};
    std::vector<std::shared_future<Msg>> in_progress;

    while(pending.size( ) > 0 || in_progress.size( ) > 0) {
        // Step One: Apply all pending events and remove them
        while(pending.size( ) > 0) {
            const auto& item = pending.front( );
            if(std::holds_alternative<std::shared_future<Msg>>(item)) {
                in_progress.push_back(std::get<std::shared_future<Msg>>(item));
            } else {
                const auto& msg             = std::get<Msg>(item);
                const auto  visitor         = [&model](const auto& msg) { return Update::update(model, msg); };
                auto [next_model, commands] = std::visit(visitor, msg);
                std::copy(commands.begin( ), commands.end( ), std::back_inserter(pending));
                model = next_model;
            }
            pending.pop_front( );
        }

        // Step Two: Pause the loop, the only way we get more events now is by polling
        // until one of the pending events finishes.
        {
            for(auto future = in_progress.begin( ); future != in_progress.end( ); ++future) {
                if(future->wait_for(std::chrono::milliseconds {1}) == std::future_status::ready) {
                    pending.push_back(future->get( ));
                    in_progress.erase(future);
                    break;
                }
            }
        }
    }
}
} // namespace elm_architecture
