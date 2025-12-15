/*
 * eLog_rtos_demo.c
 *
 * Demonstration of eLog RTOS threading features
 * This file shows how to integrate eLog with your RTOS application
 */

#include "eLog.h"
#include <stdio.h>

/* Example: ThreadX configuration */
#if 0
#define ELOG_THREAD_SAFE 1
#define ELOG_RTOS_TYPE ELOG_RTOS_THREADX
#include "tx_api.h"
#endif

/* Example: FreeRTOS configuration */
#if 0  
#define ELOG_THREAD_SAFE 1
#define ELOG_RTOS_TYPE ELOG_RTOS_FREERTOS
#include "FreeRTOS.h"
#include "task.h"
#endif

/* For this demo, we'll use bare metal mode */
#define ELOG_THREAD_SAFE 0
#define ELOG_RTOS_TYPE ELOG_RTOS_NONE

/**
 * @brief Initialize logging system for RTOS application
 */
void rtos_logging_init(void) {
#if (ELOG_THREAD_SAFE == 1)
    LOG_INIT_WITH_THREAD_INFO();
#else
    LOG_INIT_WITH_CONSOLE_AUTO();
#endif
    ELOG_INFO(ELOG_MD_MAIN, "eLog RTOS integration initialized");
    ELOG_INFO(ELOG_MD_MAIN, "Thread safety: %s", (ELOG_THREAD_SAFE ? "ENABLED" : "DISABLED"));
    ELOG_INFO(ELOG_MD_MAIN, "RTOS type: %d", ELOG_RTOS_TYPE);
}

/**
 * @brief Example task that uses thread-safe logging
 */
void sensor_task_example(void) {
    elog_set_module_threshold(ELOG_MD_SENSOR, ELOG_LEVEL_DEBUG);

    ELOG_DEBUG(ELOG_MD_SENSOR, "Sensor task starting");
    int sensor_value = 42;
    ELOG_INFO(ELOG_MD_SENSOR, "Sensor reading: %d", sensor_value);

    if (sensor_value > 50) {
        ELOG_WARNING(ELOG_MD_SENSOR, "Sensor value high: 0x%02X", ELOG_SENSOR_ERR_RANGE);
    }
    if (sensor_value < 0) {
        ELOG_CRITICAL(ELOG_MD_SENSOR, "Sensor failure detected: 0x%02X", ELOG_SENSOR_ERR_NOT_FOUND);
    }
    ELOG_DEBUG(ELOG_MD_SENSOR, "Sensor task completed");
}

/**
 * @brief Example communication task with thread-safe logging
 */
void comm_task_example(void) {
    elog_set_module_threshold(ELOG_MD_COMM, ELOG_LEVEL_DEBUG);

    ELOG_DEBUG(ELOG_MD_COMM, "Communication task starting");
    ELOG_INFO(ELOG_MD_COMM, "Initializing UART communication");
    ELOG_DEBUG(ELOG_MD_COMM, "Starting I2C transaction");

    int comm_status = -1;
    if (comm_status != 0) {
        ELOG_ERROR(ELOG_MD_COMM, "I2C communication failed: 0x%02X", ELOG_COMM_ERR_I2C);
    } else {
        ELOG_INFO(ELOG_MD_COMM, "I2C communication successful");
    }
    ELOG_DEBUG(ELOG_MD_COMM, "Communication task completed");
}

/**
 * @brief Example of using custom subscriber in RTOS environment
 */
void custom_subscriber_example(elog_level_t level, const char *msg) {
    printf("[%lu] CUSTOM[%s]: %s\n", 
           (unsigned long)0, /* Would be real timestamp */
           elog_level_name(level), 
           msg);
}

/**
 * @brief Demonstrate multi-subscriber setup for RTOS
 */
void rtos_multi_subscriber_demo(void) {
    ELOG_INFO(ELOG_MD_MAIN, "Setting up multiple subscribers for RTOS environment");

    /* Add custom subscriber for ERROR and above */
    LOG_SUBSCRIBE(custom_subscriber_example, ELOG_LEVEL_ERROR);

    /* Test messages at different levels */
    ELOG_DEBUG(ELOG_MD_MAIN, "This goes only to console");
    ELOG_INFO(ELOG_MD_MAIN, "This also goes only to console");  
    ELOG_WARNING(ELOG_MD_MAIN, "This also goes only to console");
    ELOG_ERROR(ELOG_MD_MAIN, "This goes to BOTH console and custom subscriber");
    ELOG_CRITICAL(ELOG_MD_MAIN, "This also goes to BOTH subscribers");

    /* Per-module threshold demonstration */
    elog_set_module_threshold(ELOG_MD_MAIN, ELOG_LEVEL_WARNING);
    ELOG_INFO(ELOG_MD_MAIN, "This info message will NOT be shown (threshold too high)");
    ELOG_WARNING(ELOG_MD_MAIN, "This warning message WILL be shown");
    ELOG_ERROR(ELOG_MD_MAIN, "This error message WILL be shown");
    ELOG_CRITICAL(ELOG_MD_MAIN, "This critical message WILL be shown");

    ELOG_INFO(ELOG_MD_MAIN, "Multi-subscriber demo completed");
}

/**
 * @brief Main demo function
 */
void rtos_logging_demo(void) {
    printf("\n=== eLog RTOS Integration Demo ===\n");

    rtos_logging_init();
    sensor_task_example();
    comm_task_example();
    rtos_multi_subscriber_demo();

    printf("\n=== Demo Complete ===\n");
}

/* Usage in your RTOS application:
 *
 * 1. Configure eLog for your RTOS in eLog.h:
 *    #define ELOG_THREAD_SAFE 1
 *    #define ELOG_RTOS_TYPE ELOG_RTOS_FREERTOS  // or THREADX, CMSIS
 *
 * 2. Include RTOS headers before eLog.h
 *
 * 3. Call rtos_logging_init() early in your application
 *
 * 4. Use LOG_xxx macros from any task - they're automatically thread-safe
 *
 * 5. Task names will automatically appear in log messages when using
 *    LOG_INIT_WITH_THREAD_INFO() or elog_console_subscriber_with_thread
 */
