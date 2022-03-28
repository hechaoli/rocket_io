#pragma once

#include <internal/rocket_future.h>
#include <rocket/rocket_engine.h>

rocket_future_t* rocket_engine_await_next(rocket_engine_t* engine);
