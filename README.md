# Elm Architecture for C++17

Elm is a pure functional language for the front-end. It enforces an architecture that allows programs to stay pure in an event-based setting.

This simple header-only library implements a variant of the the Elm Architecture for C++17, its small footprint and simplicity makes it easy to understand and its quick to get started.

## Features

The architecture supports running commands in parallel as asynchronous tasks, aswell as in a immediate fashion.

## Example Program

This example utilizes both direct and deferred action modes. It initializes by immediately increasing a conuter, then it manipulates the counter at a later time by deferring commands for later execution. To demonstrate the asynchronicity of the library the user can also enter a number in order to increase or decrease the counter.

```c++
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
```

## Usage

Simply copy the header file into your project and include it, or add the whole project folder and do a `add_subdirectory(elm-architecture)` and `target_link_library(your-target elm-architecture)` in CMake.

## Credits

The library is heavily inspired by [elm-architecture-haskell](https://github.com/lazamar/elm-architecture-haskell), so I encourage you to check that out aswell.
