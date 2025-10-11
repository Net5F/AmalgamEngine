#pragma once

// STL
#include <vector>
#include <array>
#include <memory>
#include <string>
#include <cstddef>
#include <chrono>
#include <functional>
#include <variant>
#include <algorithm>
#include <filesystem>
#include <unordered_map>

// Libs
#include <SDL_stdinc.h>
#include "readerwriterqueue.h"
#include "nlohmann/json.hpp"

// Ours
#include "QueuedEvents.h"
#include "Log.h"
#include "NetworkDefs.h"
#include "SharedConfig.h"
