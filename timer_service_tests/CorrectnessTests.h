#pragma once

namespace timer_service_tests {

void SetTimerTest();
void SparseTimer();
void InterleavingTest();
void CancelingTest();
void NotCancelingTest();
void InvalidCancelTest();
void CancelExpiredTest();
void InOrderTest();
void ModifyContextTest();
void RunCorrectnessTests();

}
