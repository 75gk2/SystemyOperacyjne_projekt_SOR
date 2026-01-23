add_executable(main main/src/main.cpp)
target_link_libraries(main PRIVATE sorLib)


add_executable(doctorProc doctor/src/main.cpp)
target_link_libraries(doctorProc PRIVATE sorLib)

add_executable(patientProc patient/src/main.cpp)
target_link_libraries(patientProc PRIVATE sorLib)

add_executable(registrationProc registration/src/main.cpp)
target_link_libraries(registrationProc PRIVATE sorLib)

add_executable(triageProc triage/src/main.cpp)
target_link_libraries(triageProc PRIVATE sorLib)

add_executable(waitingRoomProc waitingRoom/src/main.cpp)
target_link_libraries(waitingRoomProc PRIVATE sorLib)

add_library(global_config INTERFACE)
target_compile_definitions(global_config INTERFACE
        DOCTOR_PROC_PATH="$<TARGET_FILE:doctorProc>"
        PATIENT_PROC_PATH="$<TARGET_FILE:patientProc>"
        REGISTRATION_PROC_PATH="$<TARGET_FILE:registrationProc>"
        TRIAGE_PROC_PATH="$<TARGET_FILE:triageProc>"
        WAITING_ROOM_PROC_PATH="$<TARGET_FILE:waitingRoomProc>"
)
target_link_libraries(main PRIVATE global_config test_config)
