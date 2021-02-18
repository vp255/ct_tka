#include "timer_service_tests/DemoTimerServiceClient.h"
#include "timer_service_tests/CorrectnessTests.h"

int main() {
  //RunDemoListenerTests();
  timer_service_tests::RunCorrectnessTests();
  return 0;
}
