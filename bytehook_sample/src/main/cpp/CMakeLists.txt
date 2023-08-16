cmake_minimum_required(VERSION 3.22.1)
project(sample)

find_package(bytehook REQUIRED CONFIG)

# libhacker.so
add_library(hacker SHARED hacker.cpp)
target_compile_features(hacker PUBLIC c_std_11)
target_compile_options(hacker PUBLIC -Werror)
target_link_libraries(hacker bytehook::bytehook log)

# libunity.so
add_library(unity SHARED Vector3.cpp)
target_compile_features(unity PUBLIC c_std_11)
target_compile_options(unity PUBLIC -Werror) # -Weverything
target_link_libraries(unity log)


# libhookee.so
add_library(hookee SHARED hookee.cpp)
target_compile_features(hookee PUBLIC c_std_11)
target_compile_options(hookee PUBLIC -Werror) # -Weverything
target_link_libraries(hookee log unity)


