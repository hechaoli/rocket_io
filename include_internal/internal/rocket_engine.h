#pragma once

#include <rocket_engine.h>
#include <internal/rocket_future.h>

rocket_future_t* rocket_engine_await_next(rocket_engine_t* engine);
