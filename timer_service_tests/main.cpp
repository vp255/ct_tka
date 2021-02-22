#include "timer_service_tests/CorrectnessTests.h"
#include "timer_service_tests/DemoTimerServiceClient.h"
#include "timer_service_tests/PerformanceTests.h"
#include "timer_service/TimerService.h"
#include "timer_service/TimerService_map_impl.h"
#include "timer_service/TimerService_priority_queue_impl.h"

int main() {
  RunDemoListenerTests();

  std::cout << "Running CorrectnessTests for TimerService" << std::endl;
  timer_service_tests::RunCorrectnessTests<TimerService>();
  std::cout << "Passed Correctness Tests for TimerService" << std::endl;

  //std::cout << "Running CorrectnessTests for TimerService_map_impl" << std::endl;
  //timer_service_tests::RunCorrectnessTests<TimerService_map_impl>();
  //std::cout << "Passed Correctness Tests for TimerService_map_impl" << std::endl;

  //std::cout << "Running CorrectnessTests for TimerService_priority_queue_impl" << std::endl;
  //timer_service_tests::RunCorrectnessTests<TimerService_priority_queue_impl>();
  //std::cout << "Passed Correctness Tests for TimerService_priority_queue_impl" << std::endl;


  std::cout << "Running PerformanceTests for TimerService\n";
  timer_service_tests::RunPerformanceTests<TimerService>();
  std::cout << "\n\n";

  //std::cout << "Running PerformanceTests for TimerService_map_impl\n";
  //timer_service_tests::RunPerformanceTests<TimerService_map_impl>();
  //std::cout << "\n\n";

  //std::cout << "Running PerformanceTests for TimerService_priority_queue_impl\n";
  //timer_service_tests::RunPerformanceTests<TimerService_priority_queue_impl>();
  //std::cout << "\n\n";

  return 0;
}
