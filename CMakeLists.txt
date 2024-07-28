cmake_minimum_required(VERSION 3.10)
project(CarromBoard)

set(CMAKE_CXX_STANDARD 17)

# Add Box2D
add_subdirectory(libs/box2d/src)

# Find SFML
find_package(SFML 2.5 COMPONENTS graphics window system REQUIRED)

# Include directories
include_directories(include libs/box2d/include)
include_directories(${PROJECT_SOURCE_DIR}/include)  # Uncomment if you have a separate 'include' directory for your project

# Add executable
add_executable(CarromBoard
    src/main.cpp
    src/loading_assets.cpp
    src/eventsHandling.cpp
)

# Link libraries
target_link_libraries(CarromBoard PRIVATE sfml-graphics sfml-window sfml-system box2d)
