cmake_minimum_required(VERSION 3.18 FATAL_ERROR)
project(splendor)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_CXX_FLAGS_DEBUG  "-g -O0")
set(CMAKE_CXX_FLAGS_RELEASE  "-g -O3")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${TORCH_CXX_FLAGS}")

include_directories(src/agent)
include_directories(src/engine)
include_directories(src/neural)
include_directories(src/test)
include_directories(src/util)
include_directories(src/view)


add_executable(splendor src/main.cpp)
target_sources(splendor PRIVATE src/agent/agent_PrunedRandom.cpp)
target_sources(splendor PRIVATE src/agent/agent_MonteCarloTreeSearch.cpp)
target_sources(splendor PRIVATE src/agent/agent_SmartRollout.cpp)
target_sources(splendor PRIVATE src/engine/engine_GameState.cpp)
target_sources(splendor PRIVATE src/engine/engine_Runner.cpp)
target_sources(splendor PRIVATE src/engine/engine_DevelopmentCard.cpp)
target_sources(splendor PRIVATE src/engine/engine_NobleCard.cpp)
target_sources(splendor PRIVATE src/engine/engine_Player.cpp)
target_sources(splendor PRIVATE src/test/test_Collect.cpp)
target_sources(splendor PRIVATE src/util/util_Format.cpp)

set_property(TARGET splendor PROPERTY CXX_STANDARD 20)
set_property(TARGET splendor PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)
