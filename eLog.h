/***********************************************************
* @file	eLog.h
* @author	Andy Chen (clgm216@gmail.com)
* @version	0.03
* @date	2024-09-10
* @brief  Enhanced logging system inspired by uLog with multiple subscribers
*         INDEPENDENT HEADER - No external dependencies (except standard C)
* **********************************************************
* @copyright Copyright (c) 2025 TTK. All rights reserved.
* 
************************************************************/
#ifndef APP_DEBUG_H_
#define APP_DEBUG_H_

#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>
#include "bit_utils.h"

/* ========================================================================== */
/* Enhanced Logging Configuration */
/* ========================================================================== */

/* Supported RTOS types - Define these first before using them */
#define ELOG_RTOS_NONE       0      /* No RTOS - bare metal */
#define ELOG_RTOS_FREERTOS   1      /* FreeRTOS */
#define ELOG_RTOS_THREADX    2      /* Azure ThreadX */
#define ELOG_RTOS_CMSIS      3      /* CMSIS-RTOS */

// /* Default eLog Configuration - Override these in your project if needed */
#define ELOG_THREAD_SAFE 1
#define ELOG_RTOS_TYPE ELOG_RTOS_THREADX
#define ELOG_MUTEX_TIMEOUT_MS 100

/* ========================================================================== */
/* Enhanced Logging Configuration (continued) */
/* ========================================================================== */

/* Local definitions for independence from app_conf.h */
#ifndef YES
#define YES (0x01)
#endif
#ifndef NO
#define NO  (0x00)
#endif
#ifndef NULL
#define NULL ((void*)0)
#endif

/* Local utility function for filename extraction (independent implementation) */
static inline const char *debug_get_filename(const char *fullpath) {
  const char *ret = fullpath;
  const char *p;
  
  /* Find last occurrence of '/' */
  p = fullpath;
  while (*p) {
    if (*p == '/' || *p == '\\') {
      ret = p + 1;
    }
    p++;
  }
  
  return ret;
}

/* Debug Configuration - Modify these for different build types */
#define DEBUG_INFO YES      /* Information messages (GREEN) */
#define DEBUG_WARN YES      /* Warning messages (BROWN) */
#define DEBUG_ERR  YES      /* Error messages (RED) */
#define DEBUG_LOG  YES      /* Debug messages (CYAN) */
#define DEBUG_TRACE YES      /* Trace messages (BLUE) - very verbose, usually disabled in production */
#define DEBUG_CRITICAL YES  /* Critical error messages (RED BOLD) */
#define DEBUG_ALWAYS YES    /* Always logged messages (WHITE BOLD) */

/* Filename Support Configuration */
#define ENABLE_DEBUG_MESSAGES_WITH_FILE_NAME 1

/* Auto-calculate the lowest enabled log level for enhanced logging */
#if (DEBUG_TRACE == YES)
#define LOG_AUTO_THRESHOLD LOG_LEVEL_TRACE
#elif (DEBUG_LOG == YES)
#define LOG_AUTO_THRESHOLD LOG_LEVEL_DEBUG
#elif (DEBUG_INFO == YES)
#define LOG_AUTO_THRESHOLD LOG_LEVEL_INFO
#elif (DEBUG_WARN == YES)
#define LOG_AUTO_THRESHOLD LOG_LEVEL_WARNING
#elif (DEBUG_ERR == YES)
#define LOG_AUTO_THRESHOLD LOG_LEVEL_ERROR
#elif (DEBUG_CRITICAL == YES)
#define LOG_AUTO_THRESHOLD LOG_LEVEL_CRITICAL
#elif (DEBUG_ALWAYS == YES)
#define LOG_AUTO_THRESHOLD LOG_LEVEL_ALWAYS
#else
#define LOG_AUTO_THRESHOLD LOG_LEVEL_ALWAYS  /* Fallback if all disabled */
#endif

/* Maximum number of log subscribers (console, file, memory, etc.) */
#ifndef LOG_MAX_SUBSCRIBERS
#define LOG_MAX_SUBSCRIBERS 6
#endif

/* Maximum length of formatted log message */
#ifndef LOG_MAX_MESSAGE_LENGTH
#define LOG_MAX_MESSAGE_LENGTH 128
#endif

/* ========================================================================== */
/* RTOS Threading Support Types */
/* ========================================================================== */

/* RTOS-specific includes based on configuration */
#if (ELOG_THREAD_SAFE == 1)
  extern volatile bool RTOS_READY; // Flag to indicate if RTOS is ready
  #if (ELOG_RTOS_TYPE == ELOG_RTOS_FREERTOS)
    /* Include FreeRTOS headers */
    #include "FreeRTOS.h"
    #include "semphr.h"
    #include "task.h"
  #elif (ELOG_RTOS_TYPE == ELOG_RTOS_THREADX)
    /* Include ThreadX headers */
    #include "tx_api.h"
  #elif (ELOG_RTOS_TYPE == ELOG_RTOS_CMSIS)
    /* Include CMSIS-RTOS headers */
    #include "cmsis_os.h"
  #endif
#endif

#if (ELOG_THREAD_SAFE == 1)

/* RTOS-abstracted mutex type */
#if (ELOG_RTOS_TYPE == ELOG_RTOS_FREERTOS)
  #ifdef INC_FREERTOS_H
    typedef SemaphoreHandle_t elog_mutex_t;
    #define ELOG_MUTEX_TIMEOUT pdMS_TO_TICKS(ELOG_MUTEX_TIMEOUT_MS)
  #else
    typedef void* elog_mutex_t;  /* Fallback when FreeRTOS not available */
    #define ELOG_MUTEX_TIMEOUT 0
  #endif
#elif (ELOG_RTOS_TYPE == ELOG_RTOS_THREADX)
  #ifdef TX_API_H
    typedef TX_MUTEX elog_mutex_t;
  #else
    typedef void* elog_mutex_t;  /* Fallback when ThreadX not available */
  #endif
#elif (ELOG_RTOS_TYPE == ELOG_RTOS_CMSIS)
  #ifdef CMSIS_OS_H_
    typedef osMutexId_t elog_mutex_t;
    #define ELOG_MUTEX_TIMEOUT ELOG_MUTEX_TIMEOUT_MS
  #else
    typedef void* elog_mutex_t;  /* Fallback when CMSIS-RTOS not available */
    #define ELOG_MUTEX_TIMEOUT 0
  #endif
#else
  /* Bare metal - no mutex needed */
  typedef int elog_mutex_t;
  #define ELOG_MUTEX_TIMEOUT 0
#endif

/* Thread safety result codes */
typedef enum {
  ELOG_THREAD_OK = 0,
  ELOG_THREAD_TIMEOUT,
  ELOG_THREAD_ERROR,
  ELOG_THREAD_NOT_SUPPORTED
} elog_thread_result_t;

/* Thread safety functions */
elog_thread_result_t elog_mutex_create(elog_mutex_t *mutex);
elog_thread_result_t elog_mutex_take(elog_mutex_t *mutex, uint32_t timeout_ms);
elog_thread_result_t elog_mutex_give(elog_mutex_t *mutex);
elog_thread_result_t elog_mutex_delete(elog_mutex_t *mutex);

#endif /* ELOG_THREAD_SAFE */

/* ========================================================================== */
/* Enhanced Logging Types and Enums */
/* ========================================================================== */

/**
 * @brief Enhanced log levels (inspired by uLog)
 */
typedef enum {
  LOG_LEVEL_TRACE = 100,    /*!< Most verbose: function entry/exit, detailed flow */
  LOG_LEVEL_DEBUG,          /*!< Debug info: variable values, state changes */
  LOG_LEVEL_INFO,           /*!< Informational: normal operation events */
  LOG_LEVEL_WARNING,        /*!< Warnings: recoverable errors, performance issues */
  LOG_LEVEL_ERROR,          /*!< Errors: serious problems that need attention */
  LOG_LEVEL_CRITICAL,       /*!< Critical: system failure, unrecoverable errors */
  LOG_LEVEL_ALWAYS          /*!< Always logged: essential system messages */
} log_level_t;

/**
 * @brief Log subscriber function prototype
 * @param level: Severity level of the message
 * @param msg: Formatted message string (temporary - copy if needed)
 */
typedef void (*log_subscriber_t)(log_level_t level, const char *msg);

/**
 * @brief Error codes for enhanced logging
 */
typedef enum {
  LOG_ERR_NONE = 0,
  LOG_ERR_SUBSCRIBERS_EXCEEDED,
  LOG_ERR_NOT_SUBSCRIBED,
  LOG_ERR_INVALID_LEVEL
} log_err_t;

#if defined(USE_DEBUG_UART_TX_AS_DEBUG_IO)
#if (DEBUG_INFO == YES) || (DEBUG_ERR == YES) && (DEBUG_LOG == YES)
#error "USE_DEBUG_UART_TX_AS_DEBUG_IO not allowed when any of DEBUG_<xxx> is YES"
#endif
#endif

/* ========================================================================== */
/* General MCU Project Error Codes */
/* ========================================================================== */

/* System Error Codes (0x10-0x1F) */
#define SYS_OK                0x00    /* Operation successful */
#define SYS_ERR_INIT          0x10    /* System initialization error */
#define SYS_ERR_CONFIG        0x11    /* Configuration error */
#define SYS_ERR_TIMEOUT       0x12    /* Operation timeout */
#define SYS_ERR_BUSY          0x13    /* System busy */
#define SYS_ERR_NOT_READY     0x14    /* System not ready */
#define SYS_ERR_INVALID_STATE 0x15    /* Invalid system state */
#define SYS_ERR_MEMORY        0x16    /* Memory allocation error */
#define SYS_ERR_WATCHDOG      0x17    /* Watchdog reset occurred */

/* Communication Error Codes (0x20-0x3F) */
#define COMM_ERR_UART         0x20    /* UART communication error */
#define COMM_ERR_I2C          0x21    /* I2C communication error */
#define COMM_ERR_SPI          0x22    /* SPI communication error */
#define COMM_ERR_CAN          0x23    /* CAN communication error */
#define COMM_ERR_USB          0x24    /* USB communication error */
#define COMM_ERR_BLE          0x25    /* Bluetooth LE error */
#define COMM_ERR_WIFI         0x26    /* WiFi communication error */
#define COMM_ERR_ETH          0x27    /* Ethernet communication error */
#define COMM_ERR_CHECKSUM     0x28    /* Data checksum error */
#define COMM_ERR_FRAME        0x29    /* Frame format error */
#define COMM_ERR_OVERRUN      0x2A    /* Buffer overrun */
#define COMM_ERR_UNDERRUN     0x2B    /* Buffer underrun */

/* Sensor Error Codes (0x40-0x5F) */
#define SENSOR_ERR_NOT_FOUND  0x40    /* Sensor not detected */
#define SENSOR_ERR_CALIB      0x41    /* Sensor calibration error */
#define SENSOR_ERR_RANGE      0x42    /* Sensor value out of range */
#define SENSOR_ERR_ACCURACY   0x43    /* Sensor accuracy degraded */
#define ACCEL_ERR             0x44    /* Accelerometer error */
#define GYRO_ERR              0x45    /* Gyroscope error */
#define MAG_ERR               0x46    /* Magnetometer error */
#define PRESS_ERR             0x47    /* Pressure sensor error */
#define HUMID_ERR             0x48    /* Humidity sensor error */
#define LIGHT_ERR             0x49    /* Light sensor error */

/* Power Management Error Codes (0x60-0x7F) */
#define PWR_ERR_LOW_VOLTAGE   0x60    /* Low voltage detected */
#define PWR_ERR_OVERVOLTAGE   0x61    /* Overvoltage detected */
#define PWR_ERR_OVERCURRENT   0x62    /* Overcurrent detected */
#define PWR_ERR_THERMAL       0x63    /* Thermal shutdown */
#define PWR_ERR_CHARGER       0x64    /* Charger error */
#define PWR_ERR_REGULATOR     0x65    /* Voltage regulator error */
#define PWR_ERR_BROWNOUT      0x66    /* Brownout detected */

/* Storage Error Codes (0x80-0x9F) */
#define STORAGE_ERR_READ      0x80    /* Storage read error */
#define STORAGE_ERR_WRITE     0x81    /* Storage write error */
#define STORAGE_ERR_ERASE     0x82    /* Storage erase error */
#define STORAGE_ERR_FULL      0x83    /* Storage full */
#define STORAGE_ERR_CORRUPT   0x84    /* Data corruption detected */
#define FLASH_ERR             0x85    /* Flash memory error */
#define EEPROM_ERR            0x86    /* EEPROM error */
#define SD_ERR                0x87    /* SD card error */

/* Application Error Codes (0xA0-0xBF) */
#define APP_ERR_INVALID_PARAM 0xA0    /* Invalid parameter */
/* BATT_ERR and TEMP_ERR already defined above */
#define RTC_ERR               0xA3    /* Real-time clock error */
#define CRYPTO_ERR            0xA4    /* Cryptographic operation error */
#define AUTH_ERR              0xA5    /* Authentication error */
#define PROT_ERR              0xA6    /* Protocol error */
#define DATA_ERR              0xA7    /* Data validation error */
#define ALGO_ERR              0xA8    /* Algorithm execution error */

/* Hardware Error Codes (0xC0-0xDF) */
#define HW_ERR_GPIO           0xC0    /* GPIO configuration error */
#define HW_ERR_CLOCK          0xC1    /* Clock configuration error */
#define HW_ERR_DMA            0xC2    /* DMA error */
#define HW_ERR_TIMER          0xC3    /* Timer error */
#define HW_ERR_ADC            0xC4    /* ADC error */
#define HW_ERR_DAC            0xC5    /* DAC error */
#define HW_ERR_PWM            0xC6    /* PWM error */
#define HW_ERR_IRQ            0xC7    /* Interrupt error */

/* RTOS Error Codes (0xE0-0xEF) */
#define RTOS_ERR_TASK         0xE0    /* Task creation/management error */
#define RTOS_ERR_QUEUE        0xE1    /* Queue operation error */
#define RTOS_ERR_SEMAPHORE    0xE2    /* Semaphore error */
#define RTOS_ERR_MUTEX        0xE3    /* Mutex error */
#define RTOS_ERR_TIMER        0xE4    /* RTOS timer error */
#define RTOS_ERR_MEMORY       0xE5    /* RTOS memory allocation error */

/* Critical System Error Codes (0xF0-0xFF) */
#define CRITICAL_ERR_STACK    0xF0    /* Stack overflow */
#define CRITICAL_ERR_HEAP     0xF1    /* Heap corruption */
#define CRITICAL_ERR_ASSERT   0xF2    /* Assertion failure */
#define CRITICAL_ERR_HARDFAULT 0xF3   /* Hard fault exception */
#define CRITICAL_ERR_MEMFAULT 0xF4    /* Memory management fault */
#define CRITICAL_ERR_BUSFAULT 0xF5    /* Bus fault */
#define CRITICAL_ERR_USAGE    0xF6    /* Usage fault */
#define CRITICAL_ERR_UNKNOWN  0xFF    /* Unknown critical error */

/* ========================================================================== */
/* Enhanced Logging API (uLog-inspired) */
/* ========================================================================== */

/* Enhanced logging functions */
/**
 * @brief Initialize the enhanced logging system
 */
void elog_init(void);

/**
 * @brief Subscribe a function to receive log messages
 * @param fn: Function to call for each log message
 * @param threshold: Minimum level to send to this subscriber
 * @return Error code
 */
log_err_t elog_subscribe(log_subscriber_t fn, log_level_t threshold);

/**
 * @brief Unsubscribe a function from receiving log messages
 * @param fn: Function to unsubscribe
 * @return Error code
 */
log_err_t elog_unsubscribe(log_subscriber_t fn);

/**
 * @brief Get the string name of a log level
 * @param level: Log level
 * @return String name of the log level
 */
const char *elog_level_name(log_level_t level);

/**
 * @brief Log a message
 * @param level: Severity level of the message
 * @param fmt: Format string (printf-style)
 */
void elog_message(log_level_t level, const char *fmt, ...);

/**
 * @brief Log a message with source location information
 * @param level: Severity level of the message
 * @param file: Source file name
 * @param func: Function name
 * @param line: Line number
 * @param fmt: Format string (printf-style)
 */
void elog_message_with_location(log_level_t level, const char *file, const char *func, int line, const char *fmt, ...);

/**
 * @brief Get the automatically calculated threshold level
 * @return The LOG_AUTO_THRESHOLD value
 */
log_level_t elog_get_auto_threshold(void);

/**
 * @brief Set log threshold for a specific source file (module)
 * @param filename: Short filename (e.g. "sensor.c")
 * @param threshold: Log level threshold
 * @return LOG_ERR_NONE on success
 */
log_err_t elog_set_file_threshold(const char *filename, log_level_t threshold);

/**
 * @brief Get log threshold for a specific source file (module)
 * @param filename: Short filename (e.g. "sensor.c")
 * @return threshold, or LOG_AUTO_THRESHOLD if not set
 */
log_level_t elog_get_file_threshold(const char *filename);

/* Thread-safe logging functions (always declared, but may have fallback implementations) */
void elog_message_safe(log_level_t level, const char *fmt, ...);
void elog_message_with_location_safe(log_level_t level, const char *file, const char *func, int line, const char *fmt, ...);
log_err_t elog_subscribe_safe(log_subscriber_t fn, log_level_t threshold);
log_err_t elog_unsubscribe_safe(log_subscriber_t fn);

#if (ELOG_THREAD_SAFE == 1)
/* Thread info functions for enhanced debugging (only available when threading enabled) */
const char *elog_get_task_name(void);
uint32_t elog_get_task_id(void);

/* Thread-aware console subscriber */
extern void elog_console_subscriber_with_thread(log_level_t level, const char *msg);
#endif

/* Thread-safe aliases - automatically select based on ELOG_THREAD_SAFE */
#if (ELOG_THREAD_SAFE == 1)
  #define LOG_MESSAGE(level, ...) elog_message_safe(level, __VA_ARGS__)
  #define LOG_MESSAGE_WITH_LOCATION(level, file, func, line, ...) elog_message_with_location_safe(level, file, func, line, __VA_ARGS__)
  #define LOG_SUBSCRIBE_THREAD_SAFE(fn, level) elog_subscribe_safe(fn, level)
  #define LOG_UNSUBSCRIBE_THREAD_SAFE(fn) elog_unsubscribe_safe(fn)
#else
  #define LOG_MESSAGE(level, ...) elog_message(level, __VA_ARGS__)
  #define LOG_MESSAGE_WITH_LOCATION(level, file, func, line, ...) elog_message_with_location(level, file, func, line, __VA_ARGS__)
  #define LOG_SUBSCRIBE_THREAD_SAFE(fn, level) elog_subscribe(fn, level)
  #define LOG_UNSUBSCRIBE_THREAD_SAFE(fn) elog_unsubscribe(fn)
#endif

/* Enhanced logging core macros */
#define LOG_INIT() elog_init()
#define LOG_SUBSCRIBE(fn, level) LOG_SUBSCRIBE_THREAD_SAFE(fn, level)
#define LOG_UNSUBSCRIBE(fn) LOG_UNSUBSCRIBE_THREAD_SAFE(fn)
#define LOG_LEVEL_NAME(level) elog_level_name(level)

/* Convenience setup macro with console subscriber */
extern void elog_console_subscriber(log_level_t level, const char *msg);
#define LOG_INIT_WITH_CONSOLE() do { \
    LOG_INIT(); \
    LOG_SUBSCRIBE(elog_console_subscriber, LOG_AUTO_THRESHOLD); \
} while(0)

/* Enhanced convenience macros with automatic threshold */
#define LOG_INIT_AUTO() do { \
    LOG_INIT(); \
} while(0)

#define LOG_SUBSCRIBE_CONSOLE() LOG_SUBSCRIBE(elog_console_subscriber, LOG_AUTO_THRESHOLD)
#define LOG_SUBSCRIBE_CONSOLE_LEVEL(level) LOG_SUBSCRIBE(elog_console_subscriber, level)

/* Ultimate convenience macro - initializes and subscribes console with auto threshold */
#define LOG_INIT_WITH_CONSOLE_AUTO() do { \
    LOG_INIT(); \
    LOG_SUBSCRIBE_CONSOLE(); \
} while(0)

/* Thread-aware convenience macros with task information */
#if (ELOG_THREAD_SAFE == 1)
#define LOG_INIT_WITH_THREAD_INFO() do { \
    LOG_INIT(); \
    LOG_SUBSCRIBE(elog_console_subscriber_with_thread, LOG_AUTO_THRESHOLD); \
} while(0)

extern void elog_console_subscriber_with_thread(log_level_t level, const char *msg);
#endif

/* Individual level macros - follow same pattern as legacy debug macros */
#if (DEBUG_TRACE == YES)
#if ENABLE_DEBUG_MESSAGES_WITH_FILE_NAME
#define LOG_TRACE(...) LOG_MESSAGE_WITH_LOCATION(LOG_LEVEL_TRACE, debug_get_filename(__ASSERT_FILE_NAME), __func__, __LINE__, __VA_ARGS__)
#define LOG_TRACE_STR(str) LOG_MESSAGE_WITH_LOCATION(LOG_LEVEL_TRACE, debug_get_filename(__ASSERT_FILE_NAME), __func__, __LINE__, "%s", str)
#else
#define LOG_TRACE(...) LOG_MESSAGE(LOG_LEVEL_TRACE, __VA_ARGS__)
#define LOG_TRACE_STR(str) LOG_MESSAGE(LOG_LEVEL_TRACE, "%s", str)
#endif
#else
#define LOG_TRACE(...) do {} while(0)
#define LOG_TRACE_STR(str) do {} while(0)
#endif

#if (DEBUG_LOG == YES)
#if ENABLE_DEBUG_MESSAGES_WITH_FILE_NAME
#define LOG_DEBUG(...) LOG_MESSAGE_WITH_LOCATION(LOG_LEVEL_DEBUG, debug_get_filename(__ASSERT_FILE_NAME), __func__, __LINE__, __VA_ARGS__)
#define LOG_DEBUG_STR(str) LOG_MESSAGE_WITH_LOCATION(LOG_LEVEL_DEBUG, debug_get_filename(__ASSERT_FILE_NAME), __func__, __LINE__, "%s", str)
#else
#define LOG_DEBUG(...) LOG_MESSAGE(LOG_LEVEL_DEBUG, __VA_ARGS__)
#define LOG_DEBUG_STR(str) LOG_MESSAGE(LOG_LEVEL_DEBUG, "%s", str)
#endif
#else
#define LOG_DEBUG(...) do {} while(0)
#define LOG_DEBUG_STR(str) do {} while(0)
#endif

#if (DEBUG_INFO == YES)
#if ENABLE_DEBUG_MESSAGES_WITH_FILE_NAME
#define LOG_INFO(...) LOG_MESSAGE_WITH_LOCATION(LOG_LEVEL_INFO, debug_get_filename(__ASSERT_FILE_NAME), __func__, __LINE__, __VA_ARGS__)
#define LOG_INFO_STR(str) LOG_MESSAGE_WITH_LOCATION(LOG_LEVEL_INFO, debug_get_filename(__ASSERT_FILE_NAME), __func__, __LINE__, "%s", str)
#else
#define LOG_INFO(...) LOG_MESSAGE(LOG_LEVEL_INFO, __VA_ARGS__)
#define LOG_INFO_STR(str) LOG_MESSAGE(LOG_LEVEL_INFO, "%s", str)
#endif
#else
#define LOG_INFO(...) do {} while(0)
#define LOG_INFO_STR(str) do {} while(0)
#endif

#if (DEBUG_WARN == YES)
#if ENABLE_DEBUG_MESSAGES_WITH_FILE_NAME
#define LOG_WARNING(...) LOG_MESSAGE_WITH_LOCATION(LOG_LEVEL_WARNING, debug_get_filename(__ASSERT_FILE_NAME), __func__, __LINE__, __VA_ARGS__)
#define LOG_WARNING_STR(str) LOG_MESSAGE_WITH_LOCATION(LOG_LEVEL_WARNING, debug_get_filename(__ASSERT_FILE_NAME), __func__, __LINE__, "%s", str)
#else
#define LOG_WARNING(...) LOG_MESSAGE(LOG_LEVEL_WARNING, __VA_ARGS__)
#define LOG_WARNING_STR(str) LOG_MESSAGE(LOG_LEVEL_WARNING, "%s", str)
#endif
#else
#define LOG_WARNING(...) do {} while(0)
#define LOG_WARNING_STR(str) do {} while(0)
#endif

#if (DEBUG_ERR == YES)
#if ENABLE_DEBUG_MESSAGES_WITH_FILE_NAME
#define LOG_ERROR(...) LOG_MESSAGE_WITH_LOCATION(LOG_LEVEL_ERROR, debug_get_filename(__ASSERT_FILE_NAME), __func__, __LINE__, __VA_ARGS__)
#define LOG_ERROR_STR(str) LOG_MESSAGE_WITH_LOCATION(LOG_LEVEL_ERROR, debug_get_filename(__ASSERT_FILE_NAME), __func__, __LINE__, "%s", str)
#else
#define LOG_ERROR(...) LOG_MESSAGE(LOG_LEVEL_ERROR, __VA_ARGS__)
#define LOG_ERROR_STR(str) LOG_MESSAGE(LOG_LEVEL_ERROR, "%s", str)
#endif
#else
#define LOG_ERROR(...) do {} while(0)
#define LOG_ERROR_STR(str) do {} while(0)
#endif

#if (DEBUG_CRITICAL == YES)
#if ENABLE_DEBUG_MESSAGES_WITH_FILE_NAME
#define LOG_CRITICAL(...) LOG_MESSAGE_WITH_LOCATION(LOG_LEVEL_CRITICAL, debug_get_filename(__ASSERT_FILE_NAME), __func__, __LINE__, __VA_ARGS__)
#define LOG_CRITICAL_STR(str) LOG_MESSAGE_WITH_LOCATION(LOG_LEVEL_CRITICAL, debug_get_filename(__ASSERT_FILE_NAME), __func__, __LINE__, "%s", str)
#else
#define LOG_CRITICAL(...) LOG_MESSAGE(LOG_LEVEL_CRITICAL, __VA_ARGS__)
#define LOG_CRITICAL_STR(str) LOG_MESSAGE(LOG_LEVEL_CRITICAL, "%s", str)
#endif
#else
#define LOG_CRITICAL(...) do {} while(0)
#define LOG_CRITICAL_STR(str) do {} while(0)
#endif

#if (DEBUG_ALWAYS == YES)
#if ENABLE_DEBUG_MESSAGES_WITH_FILE_NAME
#define LOG_ALWAYS(...) LOG_MESSAGE_WITH_LOCATION(LOG_LEVEL_ALWAYS, debug_get_filename(__ASSERT_FILE_NAME), __func__, __LINE__, __VA_ARGS__)
#define LOG_ALWAYS_STR(str) LOG_MESSAGE_WITH_LOCATION(LOG_LEVEL_ALWAYS, debug_get_filename(__ASSERT_FILE_NAME), __func__, __LINE__, "%s", str)
#else
#define LOG_ALWAYS(...) LOG_MESSAGE(LOG_LEVEL_ALWAYS, __VA_ARGS__)
#define LOG_ALWAYS_STR(str) LOG_MESSAGE(LOG_LEVEL_ALWAYS, "%s", str)
#endif
#else
#define LOG_ALWAYS(...) do {} while(0)
#define LOG_ALWAYS_STR(str) do {} while(0)
#endif

/* ========================================================================== */
/* Legacy Logging System (Backwards Compatibility) */
/* ==========================================================================*/

/* Color control for enhanced logging console subscriber */
#define USE_COLOR 1  /* Set to 0 to disable colors in elog_console_subscriber */
#define LOG_COLOR_BLACK  "30"
#define LOG_COLOR_RED    "31"
#define LOG_COLOR_GREEN  "32"
#define LOG_COLOR_BROWN  "33"
#define LOG_COLOR_BLUE   "34"
#define LOG_COLOR_PURPLE "35"
#define LOG_COLOR_CYAN   "36"
#define LOG_COLOR(COLOR) "\033[0;" COLOR "m"
#define LOG_BOLD(COLOR)  "\033[1;" COLOR "m"
#if USE_COLOR
#define LOG_RESET_COLOR  "\033[0m"
#define LOG_COLOR_E      LOG_COLOR(LOG_COLOR_RED)
#define LOG_COLOR_W      LOG_COLOR(LOG_COLOR_BROWN)
#define LOG_COLOR_I      LOG_COLOR(LOG_COLOR_GREEN)
#define LOG_COLOR_D      LOG_COLOR(LOG_COLOR_CYAN)
#else
#define LOG_COLOR_E      
#define LOG_COLOR_W      
#define LOG_COLOR_I      
#define LOG_COLOR_D      
#define LOG_RESET_COLOR  
#endif
#define LINE __LINE__

#ifdef __FILE_NAME__
#define __ASSERT_FILE_NAME __FILE_NAME__
#else /* __FILE_NAME__ */
#define __ASSERT_FILE_NAME __FILE__
#endif /* __FILE_NAME__ */

#define LOG_FORMAT(letter, format) \
  LOG_COLOR_##letter #letter ":" format LOG_RESET_COLOR "\n"

/* Legacy print macros now use enhanced logging system */
#if (DEBUG_INFO == YES)
#define printIF(format, ...) LOG_INFO(format, ##__VA_ARGS__)
#define printIF_STR(str) LOG_INFO_STR(str)
#else
#define printIF(format, ...)
#define printIF_STR(str)
#endif

#if (DEBUG_ERR == YES)
#define printERR(format, ...) LOG_ERROR(format, ##__VA_ARGS__)
#define printERR_STR(str) LOG_ERROR_STR(str)
#else
#define printERR(format, ...)
#define printERR_STR(str)
#endif

#if (DEBUG_LOG == YES)
#define printLOG(format, ...) LOG_DEBUG(format, ##__VA_ARGS__)
#define printSTR(str) LOG_DEBUG_STR(str)
#else
#define printLOG(format, ...)
#define printSTR(str)
#endif

#if (DEBUG_WARN == YES)
#define printWRN(format, ...) LOG_WARNING(format, ##__VA_ARGS__)
#define printWRN_STR(str) LOG_WARNING_STR(str)
#else
#define printWRN(format, ...)
#define printWRN_STR(str)
#endif

#if (DEBUG_CRITICAL == YES)
#define printCRITICAL(format, ...) LOG_CRITICAL(format, ##__VA_ARGS__)
#define printCRITICAL_STR(str) LOG_CRITICAL_STR(str)
#else
#define printCRITICAL(format, ...)
#define printCRITICAL_STR(str)
#endif

#if (DEBUG_ALWAYS == YES)
#define printALWAYS(format, ...) LOG_ALWAYS(format, ##__VA_ARGS__)
#define printALWAYS_STR(str) LOG_ALWAYS_STR(str)
#else
#define printALWAYS(format, ...)
#define printALWAYS_STR(str)
#endif

#if (DEBUG_TRACE == YES)
#define printTRACE(format, ...) LOG_TRACE(format, ##__VA_ARGS__)
#define printTRACE_STR(str) LOG_TRACE_STR(str)
#else
#define printTRACE(format, ...)
#define printTRACE_STR(str)
#endif

/**
 * @brief Update the RTOS_READY flag
 * @param ready: Boolean value indicating if RTOS is ready (1) or not (0)
 */
void elog_update_RTOS_ready(bool ready);

#endif /* APP_DEBUG_H_ */