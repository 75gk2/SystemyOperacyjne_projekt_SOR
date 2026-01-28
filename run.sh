cmake -S . -B build
cmake --build build --target main doctorProc patientProc registrationProc triageProc waitingRoomProc
./build/bin/main