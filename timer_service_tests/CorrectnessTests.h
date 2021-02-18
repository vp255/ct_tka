#pragma once

namespace timer_service_tests {

void SetTimerTest();
void CancelingTest();
void InvalidCancelTest();
void CancelExpiredTest();
void InOrderTest();
void SetRepeatingTimerTest();
void RunCorrectnessTests();

}
