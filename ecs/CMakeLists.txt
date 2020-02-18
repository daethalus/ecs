# CMakeList.txt : CMake project for ecs, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.8)

# Add source to this project's executable.
add_executable (ecs "ecs.h" "main.cpp" "ecs_old.h" "position.h")
set_property (TARGET ${PROJECT_NAME} PROPERTY
    CXX_STANDARD 17
)

# TODO: Add tests and install targets if needed.
