/*
 * eLog_rtos_demo.c
 *
 * Demonstration of eLog RTOS threading features
 * This file shows how to integrate eLog with your RTOS application
 */

#include "eLog.h"

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
    /* Initialize enhanced logging with thread support if available */
#if (ELOG_THREAD_SAFE == 1)
    LOG_INIT_WITH_THREAD_INFO();  /* Console subscriber with task names */
#else
    LOG_INIT_WITH_CONSOLE_AUTO(); /* Standard console subscriber */
#endif
    
    LOG_INFO("eLog RTOS integration initialized");
    LOG_INFO("Thread safety: %s", (ELOG_THREAD_SAFE ? "ENABLED" : "DISABLED"));
    LOG_INFO("RTOS type: %d", ELOG_RTOS_TYPE);
}

/**
 * @brief Example task that uses thread-safe logging
 */
void sensor_task_example(void) {
    /* This would be called from within an RTOS task */
    
    LOG_DEBUG("Sensor task starting");
    
    /* Simulate sensor reading */
    int sensor_value = 42;
    LOG_INFO("Sensor reading: %d", sensor_value);
    
    /* Simulate error condition */
    if (sensor_value > 50) {
        LOG_WARNING("Sensor value high: 0x%02X", SENSOR_ERR_RANGE);
    }
    
    /* Critical error simulation */
    if (sensor_value < 0) {
        LOG_CRITICAL("Sensor failure detected: 0x%02X", SENSOR_ERR_NOT_FOUND);
    }
    
    LOG_DEBUG("Sensor task completed");
}

/**
 * @brief Example communication task with thread-safe logging
 */
void comm_task_example(void) {
    /* This would be called from within an RTOS task */
    
    LOG_DEBUG("Communication task starting");
    
    /* Simulate UART operation */
    LOG_INFO("Initializing UART communication");
    
    /* Simulate I2C operation */
    LOG_DEBUG("Starting I2C transaction");
    
    /* Simulate communication error */
    int comm_status = -1; /* Simulate failure */
    if (comm_status != 0) {
        LOG_ERROR("I2C communication failed: 0x%02X", COMM_ERR_I2C);
    } else {
        LOG_INFO("I2C communication successful");
    }
    
    LOG_DEBUG("Communication task completed");
}

/**
 * @brief Example of using custom subscriber in RTOS environment
 */
void custom_subscriber_example(log_level_t level, const char *msg) {
    /* In a real RTOS application, you might:
     * - Send logs to a queue for background processing
     * - Store logs in a circular buffer
     * - Send logs over network/BLE
     * - Write logs to flash memory
     */
    
    /* For this demo, just add a timestamp prefix */
    printf("[%lu] CUSTOM[%s]: %s\n", 
           (unsigned long)0, /* Would be real timestamp */
           log_level_name(level), 
           msg);
}

/**
 * @brief Demonstrate multi-subscriber setup for RTOS
 */
void rtos_multi_subscriber_demo(void) {
    LOG_INFO("Setting up multiple subscribers for RTOS environment");
    
    /* Add custom subscriber for ERROR and above */
    LOG_SUBSCRIBE(custom_subscriber_example, LOG_LEVEL_ERROR);
    
    /* Test messages at different levels */
    LOG_DEBUG("This goes only to console");
    LOG_INFO("This also goes only to console");  
    LOG_WARNING("This also goes only to console");
    LOG_ERROR("This goes to BOTH console and custom subscriber");
    LOG_CRITICAL("This also goes to BOTH subscribers");
    
    LOG_INFO("Multi-subscriber demo completed");
}

/**
 * @brief Main demo function
 */
void rtos_logging_demo(void) {
    printf("\n=== eLog RTOS Integration Demo ===\n");
    
    /* Initialize logging */
    rtos_logging_init();
    
    /* Run task examples */
    sensor_task_example();
    comm_task_example();
    
    /* Demonstrate multi-subscriber setup */
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
 *    LOG_INIT_WITH_THREAD_INFO() or log_console_subscriber_with_thread
 */
