/***********************************************************
 * @file	log_example.c
 * @author	Andy Chen (clgm216@gmail.com)
 * @version	0.01
 * @date	2024-09-10
 * @brief  Enhanced logging system usage examples
 *         Demonstrates both legacy and enhanced logging APIs
 * **********************************************************
 * @copyright Copyright (c) 2025 TTK. All rights reserved.
 *
 ************************************************************/

#include "eLog.h"
#include <stdio.h>

/* ========================================================================== */
/* Enhanced Logging Examples */
/* ========================================================================== */

/**
 * @brief  Demonstrate per-file log threshold usage
 * @retval None
 */
void perFileThresholdExample(void) {
    // Set log threshold for this file (module)
    elog_set_file_threshold("eLog_example.c", LOG_LEVEL_DEBUG);

    LOG_INFO("This info message will be shown if threshold allows");
    LOG_DEBUG("This debug message will be shown due to per-file threshold");
    LOG_TRACE("This trace message will NOT be shown (threshold too high)");
}

/**
 * @brief  Demonstrate basic enhanced logging usage
 * @retval None
 */
void enhancedLoggingBasicExample(void) {
  LOG_INIT_WITH_CONSOLE_AUTO();

  LOG_INFO("Enhanced logging system initialized successfully");
  LOG_DEBUG("Debug information: value=%d, pointer=%p", 42, (void *)0x12345678);
  LOG_WARNING("This is a warning message");
  LOG_ERROR("Error occurred: code=0x%02X", 0xAB);
  LOG_CRITICAL("Critical system failure detected!");
  LOG_ALWAYS("This message is always logged");

  LOG_INFO_STR("Simple info message");
  LOG_ERROR_STR("Simple error message");
}

/**
 * @brief  Demonstrate legacy logging compatibility
 * @retval None
 */
void legacyLoggingExample(void) {
  printIF("Information message using legacy printIF");
  printLOG("Debug message using legacy printLOG: value=%d", 123);
  printWRN("Warning message using legacy printWRN");
  printERR("Error message using legacy printERR: status=0x%04X", 0x1234);

  printIF_STR("Simple info using legacy printIF_STR");
  printERR_STR("Simple error using legacy printERR_STR");
}

/**
 * @brief  Custom subscriber example
 * @param  level: Log level
 * @param  msg: Formatted message
 */
void customFileSubscriber(log_level_t level, const char *msg) {
  printf("[FILE] %s: %s\n", log_level_name(level), msg);
}

/**
 * @brief  Custom memory buffer subscriber
 * @param  level: Log level
 * @param  msg: Formatted message
 */
void customMemorySubscriber(log_level_t level, const char *msg) {
  static int message_count = 0;
  message_count++;
  printf("[MEM #%d] %s: %s\n", message_count, log_level_name(level), msg);
}

/**
 * @brief  Demonstrate multiple subscribers
 * @retval None
 */
void multipleSubscribersExample(void) {
  LOG_INIT();

  LOG_SUBSCRIBE(elog_console_subscriber, LOG_LEVEL_DEBUG);
  LOG_SUBSCRIBE(customFileSubscriber, LOG_LEVEL_INFO);
  LOG_SUBSCRIBE(customMemorySubscriber, LOG_LEVEL_ERROR);

  LOG_INFO("=== Multiple Subscribers Demo ===");

  LOG_TRACE("This trace message won't appear anywhere (threshold too low)");
  LOG_DEBUG("This debug message only goes to console");
  LOG_INFO("This info message goes to console and file");
  LOG_WARNING("This warning goes to console and file");
  LOG_ERROR("This error goes to console, file, and memory");
  LOG_CRITICAL("This critical message goes everywhere");

  LOG_INFO_STR("=== End Multiple Subscribers Demo ===");
}

/**
 * @brief  Demonstrate automatic threshold calculation
 * @retval None
 */
void autoThresholdExample(void) {
  LOG_INIT_WITH_CONSOLE_AUTO();

  LOG_INFO("=== Auto Threshold Demo ===");

  log_level_t threshold = elog_get_auto_threshold();
  LOG_INFO("Current auto-threshold: %s (%d)", log_level_name(threshold), threshold);

  LOG_INFO("Based on debug flags, console subscriber will receive:");

#if (DEBUG_TRACE == YES)
  LOG_INFO("- TRACE messages (DEBUG_TRACE=YES)");
#else
  LOG_INFO("- No TRACE messages (DEBUG_TRACE=NO)");
#endif

#if (DEBUG_LOG == YES)
  LOG_INFO("- DEBUG messages (DEBUG_LOG=YES)");
#else
  LOG_INFO("- No DEBUG messages (DEBUG_LOG=NO)");
#endif

#if (DEBUG_INFO == YES)
  LOG_INFO("- INFO messages (DEBUG_INFO=YES)");
#else
  LOG_INFO("- No INFO messages (DEBUG_INFO=NO)");
#endif

#if (DEBUG_WARN == YES)
  LOG_INFO("- WARNING messages (DEBUG_WARN=YES)");
#else
  LOG_INFO("- No WARNING messages (DEBUG_WARN=NO)");
#endif

#if (DEBUG_ERR == YES)
  LOG_INFO("- ERROR messages (DEBUG_ERR=YES)");
#else
  LOG_INFO("- No ERROR messages (DEBUG_ERR=NO)");
#endif

#if (DEBUG_CRITICAL == YES)
  LOG_INFO("- CRITICAL messages (DEBUG_CRITICAL=YES)");
#else
  LOG_INFO("- No CRITICAL messages (DEBUG_CRITICAL=NO)");
#endif

#if (DEBUG_ALWAYS == YES)
  LOG_INFO("- ALWAYS messages (DEBUG_ALWAYS=YES)");
#else
  LOG_INFO("- No ALWAYS messages (DEBUG_ALWAYS=NO)");
#endif

  LOG_INFO_STR("=== End Auto Threshold Demo ===");
}

/**
 * @brief  Demonstrate performance comparison
 * @retval None
 */
void performanceDemo(void) {
  LOG_INIT_WITH_CONSOLE_AUTO();

  LOG_INFO("=== Performance Demo ===");

  LOG_INFO("Active logging levels are optimized at compile time");

#if (DEBUG_TRACE == YES)
  LOG_TRACE("TRACE is enabled - this message has runtime cost");
#else
  LOG_TRACE("TRACE is disabled - this line compiles to: do {} while(0)");
#endif

  LOG_INFO("Legacy macros also benefit from compile-time optimization:");

#if (DEBUG_LOG == YES)
  printLOG("printLOG is enabled - uses LOG_DEBUG internally");
#else
  printLOG("printLOG is disabled - compiles to empty macro");
#endif

  LOG_INFO_STR("=== End Performance Demo ===");
}

/**
 * @brief  Demonstrate unified debug flag control
 * @retval None
 */
void unifiedDebugControlDemo(void) {
  LOG_INIT_WITH_CONSOLE_AUTO();

  LOG_INFO("=== Unified Debug Control Demo ===");
  LOG_INFO("Single debug flags control both legacy and enhanced logging:");

  LOG_INFO("Enhanced API: This uses LOG_INFO (DEBUG_INFO flag)");
  printIF("Legacy API: This uses printIF (same DEBUG_INFO flag)");

  LOG_ERROR("Enhanced API: This uses LOG_ERROR (DEBUG_ERR flag)");
  printERR("Legacy API: This uses printERR (same DEBUG_ERR flag)");

  LOG_DEBUG("Enhanced API: This uses LOG_DEBUG (DEBUG_LOG flag)");
  printLOG("Legacy API: This uses printLOG (same DEBUG_LOG flag)");

  LOG_INFO("Result: Consistent behavior between legacy and enhanced APIs");
  LOG_INFO_STR("=== End Unified Debug Control Demo ===");
}

/**
 * @brief  Demonstrate subscriber management
 * @retval None
 */
void subscriberManagementDemo(void) {
  LOG_INIT();

  LOG_SUBSCRIBE_CONSOLE();
  LOG_INFO("Console subscriber added");

  LOG_SUBSCRIBE(customMemorySubscriber, LOG_LEVEL_WARNING);
  LOG_WARNING("Memory subscriber added - you should see this in both console and memory");

  LOG_UNSUBSCRIBE(customMemorySubscriber);
  LOG_WARNING("Memory subscriber removed - you should only see this in console");

  LOG_INFO_STR("Subscriber management demo complete");
}

/**
 * @brief  Complete demonstration of enhanced logging features
 * @retval None
 */
void completeLoggingDemo(void) {
  printf("\n" LOG_COLOR(LOG_COLOR_CYAN) "==========================================\n");
  printf("    Enhanced Logging System Demo\n");
  printf("==========================================" LOG_RESET_COLOR "\n\n");

  enhancedLoggingBasicExample();
  printf("\n");

  legacyLoggingExample();
  printf("\n");

  autoThresholdExample();
  printf("\n");

  unifiedDebugControlDemo();
  printf("\n");

  performanceDemo();
  printf("\n");

  subscriberManagementDemo();
  printf("\n");

  multipleSubscribersExample();
  printf("\n");

  perFileThresholdExample();
  printf("\n");

  printf(LOG_COLOR(LOG_COLOR_GREEN) "==========================================\n");
  printf("    Enhanced Logging Demo Complete!\n");
  printf("==========================================" LOG_RESET_COLOR "\n\n");
}

/**
 * @brief  Simple initialization example for real applications
 * @retval None
 */
void simpleAppInitializationExample(void) {
  LOG_INIT_WITH_CONSOLE_AUTO();

  LOG_INFO("Application started successfully");
  printIF("Legacy logging also works");
}

/**
 * @brief Demonstrate RTOS readiness update
 * @retval None
 */
void rtosReadinessExample(void) {
    elogUpdateRtosReady(true);
    LOG_INFO("RTOS is now ready for logging");
}

/* Updated example usage for eLog */
int main() {
    completeLoggingDemo();
    return 0;
}
