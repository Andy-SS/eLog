/***********************************************************
* @file	eLog_example_rtos.c
* @author	Andy Chen (clgm216@gmail.com)
* @version	0.04
* @date	2025-12-02
* @brief  Enhanced logging system examples with RTOS threading support
* **********************************************************
* @copyright Copyright (c) 2025 TTK. All rights reserved.
* 
************************************************************/

#include "eLog.h"
#include <stdio.h>

/* ========================================================================== */
/* Example Custom Subscribers */
/* ========================================================================== */

void file_subscriber(log_level_t level, const char *msg) {
  printf("FILE[%s]: %s\n", log_level_name(level), msg);
}

void memory_subscriber(log_level_t level, const char *msg) {
  printf("MEM[%s]: %s\n", log_level_name(level), msg);
}

void network_subscriber(log_level_t level, const char *msg) {
  printf("NET[%s]: %s\n", log_level_name(level), msg);
}

/* ========================================================================== */
/* Basic Examples */
/* ========================================================================== */

void basic_logging_example(void) {
  printf("\n=== Basic Logging Example ===\n");
  LOG_INIT_WITH_CONSOLE_AUTO();

  LOG_TRACE("This is a trace message");
  LOG_DEBUG("Debug: Variable x = %d", 42);
  LOG_INFO("System initialization completed");
  LOG_WARNING("Low memory warning: %d%% used", 85);
  LOG_ERROR("Communication error: code 0x%02X", COMM_ERR_UART);
  LOG_CRITICAL("Critical system failure!");
  LOG_ALWAYS("System startup message");

  printf("Basic logging complete.\n");
}

/* ========================================================================== */
/* Per-File Log Threshold Example */
/* ========================================================================== */

void per_file_threshold_example(void) {
  printf("\n=== Per-File Log Threshold Example ===\n");
  elog_set_file_threshold("eLog_example_rtos.c", LOG_LEVEL_WARNING);

  LOG_INFO("This info message will NOT be shown (threshold too high)");
  LOG_WARNING("This warning message WILL be shown");
  LOG_ERROR("This error message WILL be shown");
  LOG_CRITICAL("This critical message WILL be shown");

  printf("Per-file log threshold demonstration complete.\n");
}

/* ========================================================================== */
/* Multiple Subscribers Example */
/* ========================================================================== */

void multiple_subscribers_example(void) {
  printf("\n=== Multiple Subscribers Example ===\n");
  LOG_INIT();

  LOG_SUBSCRIBE(elog_console_subscriber, LOG_LEVEL_INFO);
  LOG_SUBSCRIBE(file_subscriber, LOG_LEVEL_WARNING);
  LOG_SUBSCRIBE(memory_subscriber, LOG_LEVEL_ERROR);

  LOG_DEBUG("Debug message - only console should see this");
  LOG_INFO("Info message - console should see this");
  LOG_WARNING("Warning message - console and file should see this");
  LOG_ERROR("Error message - all subscribers should see this");

  printf("Multiple subscribers complete.\n");
}

/* ========================================================================== */
/* Subscriber Management Example */
/* ========================================================================== */

void subscriber_management_example(void) {
  printf("\n=== Subscriber Management Example ===\n");
  LOG_INIT();

  LOG_SUBSCRIBE(elog_console_subscriber, LOG_LEVEL_DEBUG);
  LOG_SUBSCRIBE(network_subscriber, LOG_LEVEL_ERROR);

  LOG_ERROR("Error before unsubscribing network");

  LOG_UNSUBSCRIBE(network_subscriber);

  LOG_ERROR("Error after unsubscribing network - should only go to console");

  printf("Subscriber management complete.\n");
}

/* ========================================================================== */
/* Error Codes Example */
/* ========================================================================== */

void error_codes_example(void) {
  printf("\n=== Error Codes Example ===\n");
  LOG_INIT_WITH_CONSOLE_AUTO();

  LOG_ERROR("System init failed: 0x%02X", SYS_ERR_INIT);
  LOG_ERROR("Memory allocation failed: 0x%02X", SYS_ERR_MEMORY);
  LOG_WARNING("UART timeout: 0x%02X", COMM_ERR_UART);
  LOG_ERROR("I2C bus error: 0x%02X", COMM_ERR_I2C);
  LOG_WARNING("Sensor not found: 0x%02X", SENSOR_ERR_NOT_FOUND);
  LOG_ERROR("Accelerometer error: 0x%02X", ACCEL_ERR);
  LOG_CRITICAL("Low voltage detected: 0x%02X", PWR_ERR_LOW_VOLTAGE);
  LOG_ERROR("Task creation failed: 0x%02X", RTOS_ERR_TASK);
  LOG_ERROR("Mutex error: 0x%02X", RTOS_ERR_MUTEX);
  LOG_CRITICAL("Stack overflow detected: 0x%02X", CRITICAL_ERR_STACK);
  LOG_CRITICAL("Hard fault: 0x%02X", CRITICAL_ERR_HARDFAULT);

  printf("Error codes demonstration complete.\n");
}

/* ========================================================================== */
/* Legacy Compatibility Example */
/* ========================================================================== */

void legacy_compatibility_example(void) {
  printf("\n=== Legacy Compatibility Example ===\n");
  LOG_INIT_WITH_CONSOLE_AUTO();

  printIF("Legacy info message: %s", "system ready");
  printERR("Legacy error: code %d", 404);
  printLOG("Legacy debug: value = %d", 42);
  printWRN("Legacy warning: %s", "low battery");
  printCRITICAL("Legacy critical: %s", "system failure");
  printALWAYS("Legacy always: %s", "important message");

  printf("Legacy compatibility complete.\n");
}

/* ========================================================================== */
/* RTOS Threading Examples */
/* ========================================================================== */

#if (ELOG_THREAD_SAFE == 1)

void thread_safety_example(void) {
  printf("\n=== Thread Safety Example ===\n");
  LOG_INIT_WITH_CONSOLE_AUTO();

  LOG_INFO("Thread safety is enabled (ELOG_THREAD_SAFE=%d)", ELOG_THREAD_SAFE);
  LOG_INFO("RTOS type: %d", ELOG_RTOS_TYPE);
  LOG_INFO("Current task: %s (ID: 0x%08X)", elog_get_task_name(), (unsigned int)elog_get_task_id());

  log_err_t result = log_subscribe_safe(memory_subscriber, LOG_LEVEL_WARNING);
  if (result == LOG_ERR_NONE) {
    LOG_INFO("Successfully subscribed memory subscriber in thread-safe mode");
  } else {
    LOG_ERROR("Failed to subscribe memory subscriber: %d", result);
  }

  LOG_WARNING("This message should go to both console and memory subscribers");

  printf("Thread safety demonstration complete.\n");
}

void thread_aware_logging_example(void) {
  printf("\n=== Thread-Aware Logging Example ===\n");
  LOG_INIT();
  LOG_SUBSCRIBE(elog_console_subscriber_with_thread, LOG_LEVEL_DEBUG);

  LOG_DEBUG("This message includes task name in output");
  LOG_INFO("Task information: %s", elog_get_task_name());
  LOG_WARNING("Multi-threaded logging demonstration");

  printf("Thread-aware logging complete.\n");
}

void simulated_multitask_example(void) {
  printf("\n=== Simulated Multi-Task Example ===\n");
  LOG_INIT_WITH_THREAD_INFO();

  LOG_INFO("Task A: Starting sensor initialization");
  LOG_DEBUG("Task A: I2C bus configured");
  LOG_INFO("Task A: Sensors online");
  LOG_WARNING("Task B: Communication timeout on UART");
  LOG_ERROR("Task C: Memory allocation failed in data processing");
  LOG_INFO("Task A: Sensor data ready");
  LOG_INFO("Task B: Retrying communication");
  LOG_INFO("Task B: Communication restored");

  printf("Simulated multi-task demonstration complete.\n");
}

void rtos_features_example(void) {
  printf("\n=== RTOS Features Example ===\n");
  LOG_INIT_WITH_CONSOLE_AUTO();

  LOG_INFO("Testing RTOS integration features:");

#if (ELOG_RTOS_TYPE == ELOG_RTOS_FREERTOS)
  LOG_INFO("- FreeRTOS integration enabled");
  LOG_INFO("- Mutex timeout: %d ticks", ELOG_MUTEX_TIMEOUT);
#elif (ELOG_RTOS_TYPE == ELOG_RTOS_THREADX)
  LOG_INFO("- ThreadX integration enabled");  
  LOG_INFO("- Mutex timeout: %d ticks", ELOG_MUTEX_TIMEOUT);
#elif (ELOG_RTOS_TYPE == ELOG_RTOS_CMSIS)
  LOG_INFO("- CMSIS-RTOS integration enabled");
  LOG_INFO("- Mutex timeout: %d ms", ELOG_MUTEX_TIMEOUT);
#else
  LOG_INFO("- Bare metal mode (no RTOS)");
#endif

  const char *task_name = elog_get_task_name();
  uint32_t task_id = elog_get_task_id();

  LOG_INFO("Current task: %s", task_name);
  LOG_INFO("Task ID: 0x%08X", (unsigned int)task_id);

  log_err_t result = log_subscribe_safe(file_subscriber, LOG_LEVEL_WARNING);
  LOG_INFO("Subscribe result: %d", result);

  LOG_WARNING("Test message to new subscriber");

  result = log_unsubscribe_safe(file_subscriber);
  LOG_INFO("Unsubscribe result: %d", result);

  printf("RTOS features demonstration complete.\n");
}

#endif /* ELOG_THREAD_SAFE */

/* ========================================================================== */
/* Performance and Configuration Examples */
/* ========================================================================== */

void performance_test_example(void) {
  printf("\n=== Performance Test Example ===\n");
  LOG_INIT_WITH_CONSOLE_AUTO();

  printf("Testing logging performance...\n");
  for (int i = 0; i < 10; i++) {
    LOG_DEBUG("Performance test iteration %d", i);
  }

  LOG_INFO("Short");
  LOG_INFO("Medium length message with some data: %d", 12345);
  LOG_INFO("Longer message with multiple parameters: %d, %s, 0x%08X", 42, "test", 0xDEADBEEF);

#if (ELOG_THREAD_SAFE == 1)
  printf("Testing thread-safe logging performance...\n");
  for (int i = 0; i < 5; i++) {
    elog_message_safe(LOG_LEVEL_DEBUG, "Thread-safe performance test %d", i);
  }
#endif

  printf("Performance test complete.\n");
}

void configuration_showcase(void) {
  printf("\n=== Configuration Showcase ===\n");
  LOG_INIT();
  LOG_SUBSCRIBE_CONSOLE();

  LOG_INFO("Enhanced Logging Configuration:");
  LOG_INFO("- Max subscribers: %d", LOG_MAX_SUBSCRIBERS);
  LOG_INFO("- Max message length: %d bytes", LOG_MAX_MESSAGE_LENGTH);
  LOG_INFO("- Auto threshold: %d (%s)", LOG_AUTO_THRESHOLD, log_level_name(LOG_AUTO_THRESHOLD));

#if (ELOG_THREAD_SAFE == 1)
  LOG_INFO("- Thread safety: ENABLED");
  LOG_INFO("- RTOS type: %d", ELOG_RTOS_TYPE);
  LOG_INFO("- Mutex timeout: %d ms", ELOG_MUTEX_TIMEOUT_MS);
#else
  LOG_INFO("- Thread safety: DISABLED");
#endif

#if USE_COLOR
  LOG_INFO("- Color support: ENABLED");
#else
  LOG_INFO("- Color support: DISABLED");
#endif

  printf("Configuration showcase complete.\n");
}

/* ========================================================================== */
/* Main Example Function */
/* ========================================================================== */

void complete_logging_demo(void) {
  printf("===============================================\n");
  printf("Enhanced Logging System (eLog) Demonstration\n");
  printf("Version 0.04 with RTOS Threading Support\n");
  printf("===============================================\n");

  basic_logging_example();
  per_file_threshold_example();
  multiple_subscribers_example();
  subscriber_management_example();
  error_codes_example();
  legacy_compatibility_example();

#if (ELOG_THREAD_SAFE == 1)
  thread_safety_example();
  thread_aware_logging_example();
  simulated_multitask_example();
  rtos_features_example();
#endif

  performance_test_example();
  configuration_showcase();

  printf("\n===============================================\n");
  printf("Enhanced Logging Demonstration Complete!\n");
  printf("===============================================\n");
}

int main(void) {
  complete_logging_demo();
  return 0;
}
