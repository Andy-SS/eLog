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
 * @brief  Demonstrate basic enhanced logging usage
 * @retval None
 */
void enhanced_logging_basic_example(void) {
  // Initialize the enhanced logging system with console subscriber
  LOG_INIT_WITH_CONSOLE_AUTO();

  // Basic logging with different levels
  LOG_INFO("Enhanced logging system initialized successfully");
  LOG_DEBUG("Debug information: value=%d, pointer=%p", 42, (void *)0x12345678);
  LOG_WARNING("This is a warning message");
  LOG_ERROR("Error occurred: code=0x%02X", 0xAB);
  LOG_CRITICAL("Critical system failure detected!");
  LOG_ALWAYS("This message is always logged");

  // String-only versions (more efficient for simple messages)
  LOG_INFO_STR("Simple info message");
  LOG_ERROR_STR("Simple error message");
}

/**
 * @brief  Demonstrate legacy logging compatibility
 * @retval None
 */
void legacy_logging_example(void) {
  // Legacy print macros (now use enhanced logging internally)
  printIF("Information message using legacy printIF");
  printLOG("Debug message using legacy printLOG: value=%d", 123);
  printWRN("Warning message using legacy printWRN");
  printERR("Error message using legacy printERR: status=0x%04X", 0x1234);

  // String-only versions
  printIF_STR("Simple info using legacy printIF_STR");
  printERR_STR("Simple error using legacy printERR_STR");
}

/**
 * @brief  Custom subscriber example
 * @param  level: Log level
 * @param  msg: Formatted message
 */
void custom_file_subscriber(log_level_t level, const char *msg) {
  // This is just an example - in real implementation you would write to file
  printf("[FILE] %s: %s\n", log_level_name(level), msg);
}

/**
 * @brief  Custom memory buffer subscriber
 * @param  level: Log level
 * @param  msg: Formatted message
 */
void custom_memory_subscriber(log_level_t level, const char *msg) {
  // This is just an example - in real implementation you would store in memory buffer
  static int message_count = 0;
  message_count++;
  printf("[MEM #%d] %s: %s\n", message_count, log_level_name(level), msg);
}

/**
 * @brief  Demonstrate multiple subscribers
 * @retval None
 */
void multiple_subscribers_example(void) {
  // Initialize logging system
  LOG_INIT();

  // Subscribe multiple handlers with different thresholds
  LOG_SUBSCRIBE(log_console_subscriber, LOG_LEVEL_DEBUG);   // Console gets debug and above
  LOG_SUBSCRIBE(custom_file_subscriber, LOG_LEVEL_INFO);    // File gets info and above
  LOG_SUBSCRIBE(custom_memory_subscriber, LOG_LEVEL_ERROR); // Memory gets only errors

  LOG_INFO("=== Multiple Subscribers Demo ===");

  // Test different log levels - observe which subscribers receive which messages
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
void auto_threshold_example(void) {
  LOG_INIT_WITH_CONSOLE_AUTO();

  LOG_INFO("=== Auto Threshold Demo ===");

  // Show current auto-calculated threshold
  log_level_t threshold = log_get_auto_threshold();
  LOG_INFO("Current auto-threshold: %s (%d)", log_level_name(threshold), threshold);

  // Explain what this means
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
void performance_demo(void) {
  LOG_INIT_WITH_CONSOLE_AUTO();

  LOG_INFO("=== Performance Demo ===");

  // When debug flags are disabled, these macros compile to nothing
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
void unified_debug_control_demo(void) {
  LOG_INIT_WITH_CONSOLE_AUTO();

  LOG_INFO("=== Unified Debug Control Demo ===");
  LOG_INFO("Single debug flags control both legacy and enhanced logging:");

  // Both of these are controlled by the same DEBUG_INFO flag
  LOG_INFO("Enhanced API: This uses LOG_INFO (DEBUG_INFO flag)");
  printIF("Legacy API: This uses printIF (same DEBUG_INFO flag)");

  // Both controlled by DEBUG_ERR flag
  LOG_ERROR("Enhanced API: This uses LOG_ERROR (DEBUG_ERR flag)");
  printERR("Legacy API: This uses printERR (same DEBUG_ERR flag)");

  // Both controlled by DEBUG_LOG flag
  LOG_DEBUG("Enhanced API: This uses LOG_DEBUG (DEBUG_LOG flag)");
  printLOG("Legacy API: This uses printLOG (same DEBUG_LOG flag)");

  LOG_INFO("Result: Consistent behavior between legacy and enhanced APIs");
  LOG_INFO_STR("=== End Unified Debug Control Demo ===");
}

/**
 * @brief  Demonstrate subscriber management
 * @retval None
 */
void subscriber_management_demo(void) {
  LOG_INIT();

  // Subscribe console
  LOG_SUBSCRIBE_CONSOLE();
  LOG_INFO("Console subscriber added");

  // Add custom subscriber
  LOG_SUBSCRIBE(custom_memory_subscriber, LOG_LEVEL_WARNING);
  LOG_WARNING("Memory subscriber added - you should see this in both console and memory");

  // Remove memory subscriber
  LOG_UNSUBSCRIBE(custom_memory_subscriber);
  LOG_WARNING("Memory subscriber removed - you should only see this in console");

  LOG_INFO_STR("Subscriber management demo complete");
}

/**
 * @brief  Complete demonstration of enhanced logging features
 * @retval None
 */
void complete_logging_demo(void) {
  printf("\n" LOG_COLOR(LOG_COLOR_CYAN) "==========================================\n");
  printf("    Enhanced Logging System Demo\n");
  printf("==========================================" LOG_RESET_COLOR "\n\n");

  // Run all examples
  enhanced_logging_basic_example();
  printf("\n");

  legacy_logging_example();
  printf("\n");

  auto_threshold_example();
  printf("\n");

  unified_debug_control_demo();
  printf("\n");

  performance_demo();
  printf("\n");

  subscriber_management_demo();
  printf("\n");

  multiple_subscribers_example();
  printf("\n");

  printf(LOG_COLOR(LOG_COLOR_GREEN) "==========================================\n");
  printf("    Enhanced Logging Demo Complete!\n");
  printf("==========================================" LOG_RESET_COLOR "\n\n");
}

/**
 * @brief  Simple initialization example for real applications
 * @retval None
 */
void simple_app_initialization_example(void) {
  // This is all you need in your main() or initialization function:
  LOG_INIT_WITH_CONSOLE_AUTO();

  // Now you can use both enhanced and legacy logging anywhere:
  LOG_INFO("Application started successfully");
  printIF("Legacy logging also works");

  // The system automatically:
  // - Uses colors in console output
  // - Respects debug flag settings
  // - Optimizes disabled levels at compile time
  // - Provides unified behavior between APIs
}
