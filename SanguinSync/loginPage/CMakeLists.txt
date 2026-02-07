cmake_minimum_required(VERSION 3.10)
project(SanguineSync CXX)

set(CMAKE_CXX_STANDARD 11)

add_executable(sanguine_sync
    Index.cpp
    LogInPage.cpp
    RegisterUser.cpp
    RegisterHospital.cpp
    UserLogIn.cpp
    UserMenu.cpp
    HospLogIn.cpp
    HospMenu.cpp
)
