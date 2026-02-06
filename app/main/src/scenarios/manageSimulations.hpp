#pragma once
#include <string>

void simulateManageRegistrationWindows();
void simulateFullLargeFlowNoDelay();

void simulateTriage();

void simulateNoDoctors();

void runCustomSimulation();
void simulateDefault();

int readInt(const std::string &prompt, int minValue, int maxValue);