cmake_minimum_required(VERSION 3.10)

set(APP_TEST_SOURCES
        RegistrationPreTest.cpp
        RegistrationTest.cpp
)

add_executable(app_tests ${APP_TEST_SOURCES})

target_link_libraries(app_tests PRIVATE sorLib appImpl)
target_link_libraries(app_tests PRIVATE Catch2::Catch2WithMain)
target_link_libraries(app_tests PRIVATE global_config)

target_include_directories(app_tests PRIVATE ${CMAKE_CURRENT_LIST_DIR}/../../app)

target_compile_features(app_tests PRIVATE cxx_std_17)

include(Catch)
catch_discover_tests(app_tests)
