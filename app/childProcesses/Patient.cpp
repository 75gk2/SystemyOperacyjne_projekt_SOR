#include "Patient.hpp"

#include <string>

Patient::Patient(int index, bool isVip, bool allowDiesNow)
	: Process(ProcessType::PATIENT,
		{std::to_string(index), isVip ? "1" : "0", allowDiesNow ? "1" : "0"}) {}