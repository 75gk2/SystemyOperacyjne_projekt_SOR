add_executable(main main/src/main.cpp
        main/src/scenarios/manageRegistrationWindows.cpp)
add_executable(doctorProc doctor/src/main.cpp)
add_executable(patientProc patient/src/main.cpp)
add_executable(registrationProc registration/src/main.cpp)
add_executable(triageProc triage/src/main.cpp)
add_executable(waitingRoomProc waitingRoom/src/main.cpp)

# Header-only interface for shared headers
add_library(appHeaders INTERFACE)
target_include_directories(appHeaders INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}
)

# Implementation library used only by main
add_library(appImpl
        childProcesses/Patient.cpp
        childProcesses/Registration.cpp
)
target_link_libraries(appImpl PUBLIC sorLib appHeaders)

# Link headers everywhere; implementation only to main
target_link_libraries(doctorProc PRIVATE sorLib appHeaders)
target_link_libraries(patientProc PRIVATE sorLib appHeaders)
target_link_libraries(registrationProc PRIVATE sorLib appHeaders)
target_link_libraries(triageProc PRIVATE sorLib appHeaders)
target_link_libraries(waitingRoomProc PRIVATE sorLib appHeaders)
target_link_libraries(main PRIVATE sorLib appImpl appHeaders)

add_library(global_config INTERFACE)
target_compile_definitions(global_config INTERFACE
        DOCTOR_PROC_PATH="$<TARGET_FILE:doctorProc>"
        PATIENT_PROC_PATH="$<TARGET_FILE:patientProc>"
        REGISTRATION_PROC_PATH="$<TARGET_FILE:registrationProc>"
        TRIAGE_PROC_PATH="$<TARGET_FILE:triageProc>"
        WAITING_ROOM_PROC_PATH="$<TARGET_FILE:waitingRoomProc>"
)
target_link_libraries(main PRIVATE global_config test_config)
