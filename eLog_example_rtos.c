/***********************************************************
* @file	eLog_example.c
* @author	Andy Chen (clgm216@gmail.com)
* @version	0.03
* @date	2024-09-10
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

/**
 * @brief Example file subscriber (simplified for demonstration)
 * @param level: Severity level of the message
 * @param msg: Formatted message string
 */
void file_subscriber(log_level_t level, const char *msg) {
  /* In a real implementation, you would write to a file or storage */
  printf("FILE[%s]: %s\n", log_level_name(level), msg);
}

/**
 * @brief Example memory buffer subscriber
 * @param level: Severity level of the message
 * @param msg: Formatted message string
 */
void memory_subscriber(log_level_t level, const char *msg) {
  /* In a real implementation, you would store in circular buffer */
  printf("MEM[%s]: %s\n", log_level_name(level), msg);
}

/**
 * @brief Example network subscriber (for remote logging)
 * @param level: Severity level of the message
 * @param msg: Formatted message string
 */
void network_subscriber(log_level_t level, const char *msg) {
  /* In a real implementation, you would send over network */
  printf("NET[%s]: %s\n", log_level_name(level), msg);
}

/* ========================================================================== */
/* Basic Examples */
/* ========================================================================== */

/**
 * @brief Basic logging demonstration
 */
void basic_logging_example(void) {
  printf("\n=== Basic Logging Example ===\n");
  
  /* Initialize with console subscriber */
  LOG_INIT_WITH_CONSOLE_AUTO();
  
  /* Test all log levels */
  LOG_TRACE("This is a trace message");
  LOG_DEBUG("Debug: Variable x = %d", 42);
  LOG_INFO("System initialization completed");
  LOG_WARNING("Low memory warning: %d%% used", 85);
  LOG_ERROR("Communication error: code 0x%02X", COMM_ERR_UART);
  LOG_CRITICAL("Critical system failure!");
  LOG_ALWAYS("System startup message");
  
  printf("Basic logging complete.\n");
}

/**
 * @brief Multiple subscribers demonstration
 */
void multiple_subscribers_example(void) {
  printf("\n=== Multiple Subscribers Example ===\n");
  
  /* Initialize logging system */
  LOG_INIT();
  
  /* Subscribe multiple outputs with different thresholds */
  LOG_SUBSCRIBE(log_console_subscriber, LOG_LEVEL_INFO);    /* Console: INFO and above */
  LOG_SUBSCRIBE(file_subscriber, LOG_LEVEL_WARNING);        /* File: WARNING and above */
  LOG_SUBSCRIBE(memory_subscriber, LOG_LEVEL_ERROR);        /* Memory: ERROR and above */
  
  /* Test messages at different levels */
  LOG_DEBUG("Debug message - only console should see this");
  LOG_INFO("Info message - console should see this");
  LOG_WARNING("Warning message - console and file should see this");
  LOG_ERROR("Error message - all subscribers should see this");
  
  printf("Multiple subscribers complete.\n");
}

/**
 * @brief Subscriber management demonstration
 */
void subscriber_management_example(void) {
  printf("\n=== Subscriber Management Example ===\n");
  
  LOG_INIT();
  
  /* Add subscribers */
  LOG_SUBSCRIBE(log_console_subscriber, LOG_LEVEL_DEBUG);
  LOG_SUBSCRIBE(network_subscriber, LOG_LEVEL_ERROR);
  
  LOG_ERROR("Error before unsubscribing network");
  
  /* Remove network subscriber */
  LOG_UNSUBSCRIBE(network_subscriber);
  
  LOG_ERROR("Error after unsubscribing network - should only go to console");
  
  printf("Subscriber management complete.\n");
}

/**
 * @brief Error code demonstration
 */
void error_codes_example(void) {
  printf("\n=== Error Codes Example ===\n");
  
  LOG_INIT_WITH_CONSOLE_AUTO();
  
  /* System errors */
  LOG_ERROR("System init failed: 0x%02X", SYS_ERR_INIT);
  LOG_ERROR("Memory allocation failed: 0x%02X", SYS_ERR_MEMORY);
  
  /* Communication errors */
  LOG_WARNING("UART timeout: 0x%02X", COMM_ERR_UART);
  LOG_ERROR("I2C bus error: 0x%02X", COMM_ERR_I2C);
  
  /* Sensor errors */
  LOG_WARNING("Sensor not found: 0x%02X", SENSOR_ERR_NOT_FOUND);
  LOG_ERROR("Accelerometer error: 0x%02X", ACCEL_ERR);
  
  /* Power management */
  LOG_CRITICAL("Low voltage detected: 0x%02X", PWR_ERR_LOW_VOLTAGE);
  
  /* RTOS errors */
  LOG_ERROR("Task creation failed: 0x%02X", RTOS_ERR_TASK);
  LOG_ERROR("Mutex error: 0x%02X", RTOS_ERR_MUTEX);
  
  /* Critical system errors */
  LOG_CRITICAL("Stack overflow detected: 0x%02X", CRITICAL_ERR_STACK);
  LOG_CRITICAL("Hard fault: 0x%02X", CRITICAL_ERR_HARDFAULT);
  
  printf("Error codes demonstration complete.\n");
}

/**
 * @brief Legacy compatibility demonstration
 */
void legacy_compatibility_example(void) {
  printf("\n=== Legacy Compatibility Example ===\n");
  
  LOG_INIT_WITH_CONSOLE_AUTO();
  
  /* Test legacy print macros - these should work seamlessly */
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

/**
 * @brief Thread safety demonstration
 */
void thread_safety_example(void) {
  printf("\n=== Thread Safety Example ===\n");
  
  /* Initialize with thread-safe logging */
  LOG_INIT_WITH_CONSOLE_AUTO();
  
  /* Test thread-safe functions */
  LOG_INFO("Thread safety is enabled (ELOG_THREAD_SAFE=%d)", ELOG_THREAD_SAFE);
  LOG_INFO("RTOS type: %d", ELOG_RTOS_TYPE);
  
  /* Demonstrate task information */
  LOG_INFO("Current task: %s (ID: 0x%08X)", elog_get_task_name(), (unsigned int)elog_get_task_id());
  
  /* Test mutex timeout behavior */
  log_err_t result = log_subscribe_safe(memory_subscriber, LOG_LEVEL_WARNING);
  if (result == LOG_ERR_NONE) {
    LOG_INFO("Successfully subscribed memory subscriber in thread-safe mode");
  } else {
    LOG_ERROR("Failed to subscribe memory subscriber: %d", result);
  }
  
  LOG_WARNING("This message should go to both console and memory subscribers");
  
  printf("Thread safety demonstration complete.\n");
}

/**
 * @brief Thread-aware logging demonstration
 */
void thread_aware_logging_example(void) {
  printf("\n=== Thread-Aware Logging Example ===\n");
  
  /* Initialize with thread-aware console subscriber */
  LOG_INIT();
  LOG_SUBSCRIBE(log_console_subscriber_with_thread, LOG_LEVEL_DEBUG);
  
  LOG_DEBUG("This message includes task name in output");
  LOG_INFO("Task information: %s", elog_get_task_name());
  LOG_WARNING("Multi-threaded logging demonstration");
  
  printf("Thread-aware logging complete.\n");
}

/**
 * @brief Simulated multi-task logging scenario
 */
void simulated_multitask_example(void) {
  printf("\n=== Simulated Multi-Task Example ===\n");
  
  LOG_INIT_WITH_THREAD_INFO();
  
  /* Simulate different tasks logging */
  LOG_INFO("Task A: Starting sensor initialization");
  LOG_DEBUG("Task A: I2C bus configured");
  LOG_INFO("Task A: Sensors online");
  
  /* Simulate concurrent logging (in real RTOS, these would be different tasks) */
  LOG_WARNING("Task B: Communication timeout on UART");
  LOG_ERROR("Task C: Memory allocation failed in data processing");
  LOG_INFO("Task A: Sensor data ready");
  
  /* Simulate error recovery */
  LOG_INFO("Task B: Retrying communication");
  LOG_INFO("Task B: Communication restored");
  
  printf("Simulated multi-task demonstration complete.\n");
}

/**
 * @brief RTOS-specific logging features demonstration
 */
void rtos_features_example(void) {
  printf("\n=== RTOS Features Example ===\n");
  
  LOG_INIT_WITH_CONSOLE_AUTO();
  
  /* Test RTOS-specific features */
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
  
  /* Test task information retrieval */
  const char *task_name = elog_get_task_name();
  uint32_t task_id = elog_get_task_id();
  
  LOG_INFO("Current task: %s", task_name);
  LOG_INFO("Task ID: 0x%08X", (unsigned int)task_id);
  
  /* Test thread-safe subscriber management */
  LOG_INFO("Testing thread-safe subscriber operations...");
  
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

/**
 * @brief Performance test demonstration
 */
void performance_test_example(void) {
  printf("\n=== Performance Test Example ===\n");
  
  LOG_INIT_WITH_CONSOLE_AUTO();
  
  printf("Testing logging performance...\n");
  
  /* Test rapid logging */
  for (int i = 0; i < 10; i++) {
    LOG_DEBUG("Performance test iteration %d", i);
  }
  
  /* Test with different message lengths */
  LOG_INFO("Short");
  LOG_INFO("Medium length message with some data: %d", 12345);
  LOG_INFO("Longer message with multiple parameters: %d, %s, 0x%08X", 42, "test", 0xDEADBEEF);
  
#if (ELOG_THREAD_SAFE == 1)
  /* Test thread-safe performance */
  printf("Testing thread-safe logging performance...\n");
  for (int i = 0; i < 5; i++) {
    log_message_safe(LOG_LEVEL_DEBUG, "Thread-safe performance test %d", i);
  }
#endif
  
  printf("Performance test complete.\n");
}

/**
 * @brief Configuration showcase
 */
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

/**
 * @brief Complete logging system demonstration
 */
void complete_logging_demo(void) {
  printf("===============================================\n");
  printf("Enhanced Logging System (eLog) Demonstration\n");
  printf("Version 0.03 with RTOS Threading Support\n");
  printf("===============================================\n");
  
  /* Run all examples */
  basic_logging_example();
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
