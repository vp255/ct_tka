#include "timer_service_tests/CorrectnessTests.h"
#include "timer_service_tests/DemoTimerServiceClient.h"
#include "timer_service_tests/PerformanceTests.h"
#include "timer_service/TimerService.h"

int main() {
  RunDemoListenerTests();

  std::cout << "Running CorrectnessTests for TimerService" << std::endl;
  timer_service_tests::RunCorrectnessTests<TimerService>();
  std::cout << "Passed Correctness Tests for TimerService" << std::endl;

  std::cout << "Running PerformanceTests for TimerService\n";
  timer_service_tests::RunPerformanceTests<TimerService>();
  std::cout << "\n\n";

  return 0;
}
