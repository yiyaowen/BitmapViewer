cmake_minimum_required(VERSION 3.5)

project(BitmapViewer LANGUAGES C)

set(CMAKE_C_STANDARD_REQUIRED 17)

# Set subsystem to Win32.
add_executable(BitmapViewer WIN32
    BitmapIO.h
    BitmapIO.c
    BitmapViewer.c)
