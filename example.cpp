#include "elm-architecture/elm-architecture.hpp"

#include <iostream>

namespace elm = elm_architecture;

// Model

struct model_type {
    int counter = 0;
};

// Msg

struct increase {};

struct decrease {};

struct user_increase {
    int value;
};

using message_type = std::variant<increase, decrease, user_increase>;

std::shared_future<message_type>
delayed_increase(std::chrono::milliseconds delay) {
    return std::async(
               std::launch::async,
               [delay]( ) -> message_type {
                   std::this_thread::sleep_for(delay);
                   return increase {};
               })
        .share( );
}

std::shared_future<message_type>
delayed_decrease(std::chrono::milliseconds delay) {
    return std::async(
               std::launch::async,
               [delay]( ) -> message_type {
                   std::this_thread::sleep_for(delay);
                   return decrease {};
               })
        .share( );
}

std::shared_future<message_type>
ask_user( ) {
    return std::async(
               std::launch::async,
               []( ) -> message_type {
                   int amount = 0;
                   std::cin >> amount;
                   return user_increase {amount};
               })
        .share( );
}

// Update

struct update_fn {
    using return_type = elm::return_type<model_type, message_type>;

    static auto
    update(const model_type& mod, const increase&) -> return_type {
        auto next = mod;
        next.counter += 1;
        std::cout << "Increasing counter from " << mod.counter << " to " << next.counter << std::endl;
        return {next, {}};
    }

    static auto
    update(const model_type& mod, const decrease&) -> return_type {
        auto next = mod;
        next.counter -= 1;
        std::cout << "Decreasing counter from " << mod.counter << " to " << next.counter << std::endl;
        return {next, {}};
    }

    static auto
    update(const model_type& mod, const user_increase& msg) -> return_type {
        auto next = mod;
        next.counter += msg.value;
        std::cout << "User increasing counter from " << mod.counter << " to " << next.counter << std::endl;
        return {next, {ask_user( )}};
    }
};

// Event Loop

int
main( ) {
    elm::start_eventloop<model_type, message_type, update_fn>({
        increase {},
        delayed_increase(std::chrono::milliseconds {1500}),
        delayed_decrease(std::chrono::milliseconds {1000}),
        delayed_increase(std::chrono::milliseconds {400}),
        ask_user( ),
    });
}
