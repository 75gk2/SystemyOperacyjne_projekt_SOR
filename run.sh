cmake -S . -B build
cmake --build build --target main doctorProc patientProc registrationProc triageProc waitingRoomProc

if [ "$1" = "1" ]; then
	./build/bin/main 1
else
	./build/bin/main
fi