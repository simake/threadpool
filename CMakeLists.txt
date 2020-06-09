cmake_minimum_required(VERSION 3.13)  # cxx_std_11 (3.8), relative target_sources (3.13)
project(threadpool)

add_library(threadpool INTERFACE)
target_compile_features(threadpool INTERFACE cxx_std_11)
target_sources(threadpool INTERFACE threadpool.hpp)
