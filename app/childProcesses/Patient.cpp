#include "Patient.hpp"

#include <string>

Patient::Patient(int index, bool isVip, bool allowDiesNow, bool isThisParentWithChildren)
	: Process(ProcessType::PATIENT,
	          {
		          std::to_string(index), isVip ? "1" : "0", allowDiesNow ? "1" : "0",
		          isThisParentWithChildren ? "1" : "0"
	          }) {
}
