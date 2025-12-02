/***********************************************************
* @file	eLog.c
* @author	Andy Chen (clgm216@gmail.com)
* @version	0.02
* @date	2024-09-10
* @brief  Enhanced logging system implementation inspired by uLog
*         INDEPENDENT IMPLEMENTATION - No external dependencies
* **********************************************************
* @copyright Copyright (c) 2025 TTK. All rights reserved.
* 
************************************************************/

#include "eLog.h"
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <inttypes.h> // For PRIu32 macro
#include "tx_api.h" // Include ThreadX API for _tx_thread_system_state
#include "tx_thread.h" // Include ThreadX thread header for _tx_thread_system_state

/* ========================================================================== */
/* Enhanced Logging Internal State */
/* ========================================================================== */

#if defined(ELOG_RTOS_TYPE)
volatile bool RTOS_READY = false; // Flag to indicate if RTOS is ready
#endif

/**
 * @brief Subscriber entry structure
 */
typedef struct {
  log_subscriber_t fn;
  log_level_t threshold;
  int active;
} subscriber_entry_t;

/* Static storage for subscribers */
static subscriber_entry_t s_subscribers[LOG_MAX_SUBSCRIBERS];
static int s_num_subscribers = 0;

/* Static message buffer for formatting */
static char s_message_buffer[LOG_MAX_MESSAGE_LENGTH];

#if (ELOG_THREAD_SAFE == 1)
/* Mutex for thread safety */
static elog_mutex_t s_log_mutex;
static int s_mutex_initialized = 0;
#endif

#define MAX_FILE_LOG_LEVELS 16

typedef struct {
    char filename[32];
    log_level_t threshold;
} FileLogLevelEntry;

static FileLogLevelEntry fileLogLevels[MAX_FILE_LOG_LEVELS];
static int fileLogLevelCount = 0;

/* ========================================================================== */
/* Enhanced Logging Core Implementation */
/* ========================================================================== */

/**
 * @brief Initialize the enhanced logging system
 */
void elog_init(void) {
  /* Clear all subscribers */
  for (int i = 0; i < LOG_MAX_SUBSCRIBERS; i++) {
    s_subscribers[i].fn = NULL;
    s_subscribers[i].threshold = LOG_LEVEL_ALWAYS;
    s_subscribers[i].active = 0;
  }
  s_num_subscribers = 0;

#if (ELOG_THREAD_SAFE == 1)
  /* Initialize mutex for thread safety */
  if (!s_mutex_initialized) {
    if (elogMutexCreate(&s_log_mutex) == ELOG_THREAD_OK) {
      s_mutex_initialized = 1;
      elogMutexGive(&s_log_mutex);  // Ensure mutex is available
    }
  }
#endif
}

/**
 * @brief Subscribe a function to receive log messages
 * @param fn: Function to call for each log message
 * @param threshold: Minimum level to send to this subscriber
 * @return Error code
 */
log_err_t elog_subscribe(log_subscriber_t fn, log_level_t threshold) {
  if (s_num_subscribers >= LOG_MAX_SUBSCRIBERS) {
    return LOG_ERR_SUBSCRIBERS_EXCEEDED;
  }
  
  /* Check if already subscribed */
  for (int i = 0; i < s_num_subscribers; i++) {
    if (s_subscribers[i].fn == fn && s_subscribers[i].active) {
      /* Update existing subscription */
      s_subscribers[i].threshold = threshold;
      return LOG_ERR_NONE;
    }
  }
  
  /* Add new subscriber */
  s_subscribers[s_num_subscribers].fn = fn;
  s_subscribers[s_num_subscribers].threshold = threshold;
  s_subscribers[s_num_subscribers].active = 1;
  s_num_subscribers++;
  
  return LOG_ERR_NONE;
}

/**
 * @brief Unsubscribe a function from receiving log messages
 * @param fn: Function to unsubscribe
 * @return Error code
 */
log_err_t elog_unsubscribe(log_subscriber_t fn) {
  for (int i = 0; i < s_num_subscribers; i++) {
    if (s_subscribers[i].fn == fn && s_subscribers[i].active) {
      s_subscribers[i].active = 0;
      return LOG_ERR_NONE;
    }
  }
  return LOG_ERR_NOT_SUBSCRIBED;
}

/**
 * @brief Get human-readable name for log level
 * @param level: Log level
 * @return String representation of level
 */
const char *elog_level_name(log_level_t level) {
  switch (level) {
    case LOG_LEVEL_TRACE:    return "TRACE";
    case LOG_LEVEL_DEBUG:    return "DEBUG";
    case LOG_LEVEL_INFO:     return "INFO";
    case LOG_LEVEL_WARNING:  return "WARNING";
    case LOG_LEVEL_ERROR:    return "ERROR";
    case LOG_LEVEL_CRITICAL: return "CRITICAL";
    case LOG_LEVEL_ALWAYS:   return "ALWAYS";
    default:                 return "UNKNOWN";
  }
}

/**
 * @brief Get the automatically calculated threshold level
 * @return The LOG_AUTO_THRESHOLD value
 */
log_level_t elog_get_auto_threshold(void) {
  return LOG_AUTO_THRESHOLD;
}

/**
 * @brief Send a formatted message to all subscribers
 * @param level: Severity level of the message
 * @param fmt: Printf-style format string
 * @param ...: Format arguments
 */
void elog_message(log_level_t level, const char *fmt, ...) {
  va_list args;
  
  /* Format the message */
  va_start(args, fmt);
  vsnprintf(s_message_buffer, sizeof(s_message_buffer), fmt, args);
  va_end(args);
  
  /* Send to all active subscribers */
  for (int i = 0; i < s_num_subscribers; i++) {
    if (s_subscribers[i].active && level >= s_subscribers[i].threshold) {
      s_subscribers[i].fn(level, s_message_buffer);
    }
  }
}

/**
 * @brief Send a formatted message with location info to all subscribers
 * @param level: Severity level of the message
 * @param file: Source file name
 * @param func: Function name
 * @param line: Line number
 * @param fmt: Printf-style format string
 * @param ...: Format arguments
 */
void elog_message_with_location(log_level_t level, const char *file, const char *func, int line, const char *fmt, ...) {
    const char *filename = debug_get_filename(file);
    log_level_t threshold = elog_get_file_threshold(filename);
    if (level < threshold) return; // Skip log if below file threshold

    va_list args;
    char temp_buffer[LOG_MAX_MESSAGE_LENGTH - 64];  /* Reserve space for location info */
  
    /* Format the user message first */
    va_start(args, fmt);
    vsnprintf(temp_buffer, sizeof(temp_buffer), fmt, args);
    va_end(args);
  
    /* Add location information - ensure null termination */
    int written = snprintf(s_message_buffer, sizeof(s_message_buffer), "[%s][%s][%d] %s", file, func, line, temp_buffer);
    if (written >= (int)sizeof(s_message_buffer)) {
      s_message_buffer[sizeof(s_message_buffer) - 1] = '\0';  /* Ensure null termination */
    }
  
    /* Send to all active subscribers */
    for (int i = 0; i < s_num_subscribers; i++) {
      if (s_subscribers[i].active && level >= s_subscribers[i].threshold) {
        s_subscribers[i].fn(level, s_message_buffer);
      }
    }
}

/* ========================================================================== */
/* Built-in Console Subscriber */
/* ========================================================================== */

/**
 * @brief Built-in console subscriber with color support
 * @param level: Severity level of the message
 * @param msg: Formatted message string
 */
void elog_console_subscriber(log_level_t level, const char *msg) {
#if USE_COLOR
  /* Color codes for different log levels */
  const char* colors[] = {
    [LOG_LEVEL_TRACE]    = LOG_COLOR(LOG_COLOR_BLUE),     /* Blue for trace */
    [LOG_LEVEL_DEBUG]    = LOG_COLOR(LOG_COLOR_CYAN),     /* Cyan for debug */
    [LOG_LEVEL_INFO]     = LOG_COLOR(LOG_COLOR_GREEN),    /* Green for info */
    [LOG_LEVEL_WARNING]  = LOG_COLOR(LOG_COLOR_BROWN),    /* Brown/Yellow for warning */
    [LOG_LEVEL_ERROR]    = LOG_COLOR(LOG_COLOR_RED),      /* Red for error */
    [LOG_LEVEL_CRITICAL] = LOG_BOLD(LOG_COLOR_RED),       /* Bold Red for critical */
    [LOG_LEVEL_ALWAYS]   = LOG_BOLD("37")                 /* Bold White for always */
  };

  if (level >= LOG_LEVEL_TRACE && level <= LOG_LEVEL_ALWAYS) {
    printf("%s%s: %s%s\n", colors[level], elog_level_name(level), msg, LOG_RESET_COLOR);
  } else {
    printf("%s: %s\n", elog_level_name(level), msg);
  }
#else
  /* No color version */
  printf("%s: %s\n", elog_level_name(level), msg);
#endif
}

/* ========================================================================== */
/* Thread Safety Implementation */
/* ========================================================================== */

#if (ELOG_THREAD_SAFE == 1)

/**
 * @brief Create a mutex for logging synchronization
 * @param mutex: Pointer to mutex handle
 * @return Thread operation result
 */
elog_thread_result_t elogMutexCreate(elog_mutex_t *mutex) {
  if (!mutex) return ELOG_THREAD_ERROR;

#if (ELOG_RTOS_TYPE == ELOG_RTOS_FREERTOS)
  #ifdef INC_FREERTOS_H
    *mutex = xSemaphoreCreateMutex();
    return (*mutex != NULL) ? ELOG_THREAD_OK : ELOG_THREAD_ERROR;
  #else
    (void)mutex;
    return ELOG_THREAD_NOT_SUPPORTED;
  #endif
#elif (ELOG_RTOS_TYPE == ELOG_RTOS_THREADX)
  #ifdef TX_API_H
    UINT status = tx_mutex_create(mutex, "eLog_Mutex", TX_NO_INHERIT);
    return (status == TX_SUCCESS) ? ELOG_THREAD_OK : ELOG_THREAD_ERROR;
  #else
    (void)mutex;
    return ELOG_THREAD_NOT_SUPPORTED;
  #endif
#elif (ELOG_RTOS_TYPE == ELOG_RTOS_CMSIS)
  #ifdef CMSIS_OS_H_
    const osMutexAttr_t mutex_attr = {
      .name = "eLog_Mutex",
      .attr_bits = osMutexRecursive | osMutexPrioInherit,
      .cb_mem = NULL,
      .cb_size = 0
    };
    *mutex = osMutexNew(&mutex_attr);
    return (*mutex != NULL) ? ELOG_THREAD_OK : ELOG_THREAD_ERROR;
  #else
    (void)mutex;
    return ELOG_THREAD_NOT_SUPPORTED;
  #endif
#else
  /* Bare metal - no mutex needed */
  (void)mutex;
  return ELOG_THREAD_OK;
#endif
}

/**
 * @brief Take/lock a mutex with timeout
 * @param mutex: Pointer to mutex handle
 * @param timeout_ms: Timeout in milliseconds
 * @return Thread operation result
 */
elog_thread_result_t elogMutexTake(elog_mutex_t *mutex, uint32_t timeout_ms) {
  if (!mutex) return ELOG_THREAD_ERROR;

  // Check if the system state is 0
  if (!RTOS_READY) {
    return ELOG_THREAD_OK;
  }

#if (ELOG_RTOS_TYPE == ELOG_RTOS_FREERTOS)
  #ifdef INC_FREERTOS_H
    BaseType_t result = xSemaphoreTake(*mutex, pdMS_TO_TICKS(timeout_ms));
    return (result == pdTRUE) ? ELOG_THREAD_OK : ELOG_THREAD_TIMEOUT;
  #else
    (void)mutex; (void)timeout_ms;
    return ELOG_THREAD_NOT_SUPPORTED;
  #endif
#elif (ELOG_RTOS_TYPE == ELOG_RTOS_THREADX)
  #ifdef TX_API_H
    UINT status = tx_mutex_get(mutex, timeout_ms * TX_TIMER_TICKS_PER_SECOND / 1000);
    if (status == TX_SUCCESS) return ELOG_THREAD_OK;
    else if (status == TX_NOT_AVAILABLE) return ELOG_THREAD_TIMEOUT;
    else return ELOG_THREAD_ERROR;
  #else
    (void)mutex; (void)timeout_ms;
    return ELOG_THREAD_NOT_SUPPORTED;
  #endif
#elif (ELOG_RTOS_TYPE == ELOG_RTOS_CMSIS)
  #ifdef CMSIS_OS_H_
    osStatus_t status = osMutexAcquire(*mutex, timeout_ms);
    if (status == osOK) return ELOG_THREAD_OK;
    else if (status == osErrorTimeout) return ELOG_THREAD_TIMEOUT;
    else return ELOG_THREAD_ERROR;
  #else
    (void)mutex; (void)timeout_ms;
    return ELOG_THREAD_NOT_SUPPORTED;
  #endif
#else
  /* Bare metal - no mutex needed */
  (void)mutex; (void)timeout_ms;
  return ELOG_THREAD_OK;
#endif
}

/**
 * @brief Give/unlock a mutex
 * @param mutex: Pointer to mutex handle
 * @return Thread operation result
 */
elog_thread_result_t elogMutexGive(elog_mutex_t *mutex) {
  if (!mutex) return ELOG_THREAD_ERROR;
  #if defined(ELOG_RTOS_TYPE)
  if (!RTOS_READY) {
    return ELOG_THREAD_OK;
  }
  #endif

#if (ELOG_RTOS_TYPE == ELOG_RTOS_FREERTOS)
  #ifdef INC_FREERTOS_H
    BaseType_t result = xSemaphoreGive(*mutex);
    return (result == pdTRUE) ? ELOG_THREAD_OK : ELOG_THREAD_ERROR;
  #else
    (void)mutex;
    return ELOG_THREAD_NOT_SUPPORTED;
  #endif
#elif (ELOG_RTOS_TYPE == ELOG_RTOS_THREADX)
  #ifdef TX_API_H
    UINT status = tx_mutex_put(mutex);
    return (status == TX_SUCCESS) ? ELOG_THREAD_OK : ELOG_THREAD_ERROR;
  #else
    (void)mutex;
    return ELOG_THREAD_NOT_SUPPORTED;
  #endif
#elif (ELOG_RTOS_TYPE == ELOG_RTOS_CMSIS)
  #ifdef CMSIS_OS_H_
    osStatus_t status = osMutexRelease(*mutex);
    return (status == osOK) ? ELOG_THREAD_OK : ELOG_THREAD_ERROR;
  #else
    (void)mutex;
    return ELOG_THREAD_NOT_SUPPORTED;
  #endif
#else
  /* Bare metal - no mutex needed */
  (void)mutex;
  return ELOG_THREAD_OK;
#endif
}

/**
 * @brief Delete a mutex
 * @param mutex: Pointer to mutex handle
 * @return Thread operation result
 */
elog_thread_result_t elogMutexDelete(elog_mutex_t *mutex) {
  if (!mutex) return ELOG_THREAD_ERROR;

#if (ELOG_RTOS_TYPE == ELOG_RTOS_FREERTOS)
  #ifdef INC_FREERTOS_H
    vSemaphoreDelete(*mutex);
    *mutex = NULL;
    return ELOG_THREAD_OK;
  #else
    (void)mutex;
    return ELOG_THREAD_NOT_SUPPORTED;
  #endif
#elif (ELOG_RTOS_TYPE == ELOG_RTOS_THREADX)
  #ifdef TX_API_H
    UINT status = tx_mutex_delete(mutex);
    return (status == TX_SUCCESS) ? ELOG_THREAD_OK : ELOG_THREAD_ERROR;
  #else
    (void)mutex;
    return ELOG_THREAD_NOT_SUPPORTED;
  #endif
#elif (ELOG_RTOS_TYPE == ELOG_RTOS_CMSIS)
  #ifdef CMSIS_OS_H_
    osStatus_t status = osMutexDelete(*mutex);
    *mutex = NULL;
    return (status == osOK) ? ELOG_THREAD_OK : ELOG_THREAD_ERROR;
  #else
    (void)mutex;
    return ELOG_THREAD_NOT_SUPPORTED;
  #endif
#else
  /* Bare metal - no mutex needed */
  (void)mutex;
  return ELOG_THREAD_OK;
#endif
}

/**
 * @brief Update the RTOS_READY flag
 * @param ready: Boolean value indicating if RTOS is ready (1) or not (0)
 */
void elog_update_RTOS_ready(bool ready) {
#if (ELOG_THREAD_SAFE == 1)
  RTOS_READY = ready;
#else
  (void)ready; // No RTOS, no action needed
#endif
}

/**
 * @brief Thread-safe version of log_message
 * @param level: Severity level of the message
 * @param fmt: Printf-style format string
 * @param ...: Format arguments
 */
void elog_message_safe(log_level_t level, const char *fmt, ...) {
  if (!s_mutex_initialized) {
    /* Fall back to non-thread-safe version */
    va_list args;
    va_start(args, fmt);
    vsnprintf(s_message_buffer, sizeof(s_message_buffer), fmt, args);
    va_end(args);

    for (int i = 0; i < s_num_subscribers; i++) {
      if (s_subscribers[i].active && level >= s_subscribers[i].threshold) {
        s_subscribers[i].fn(level, s_message_buffer);
      }
    }
    return;
  }

  /* Take mutex with timeout */
  if (elogMutexTake(&s_log_mutex, ELOG_MUTEX_TIMEOUT_MS) != ELOG_THREAD_OK) {
    return; /* Skip logging if can't get mutex */
  }

  va_list args;

  /* Format the message */
  va_start(args, fmt);
  vsnprintf(s_message_buffer, sizeof(s_message_buffer), fmt, args);
  va_end(args);

  /* Send to all active subscribers */
  for (int i = 0; i < s_num_subscribers; i++) {
    // printf("[DEBUG] Checking subscriber %d: active=%d, threshold=%d, level=%d.\n", i, s_subscribers[i].active, s_subscribers[i].threshold, level);
    if (s_subscribers[i].active && level >= s_subscribers[i].threshold) {
      // printf("[DEBUG] Sending message to subscriber %d.\n", i);
      s_subscribers[i].fn(level, s_message_buffer);
    }
  }

  /* Give mutex */
  if (elogMutexGive(&s_log_mutex) != ELOG_THREAD_OK) {
    // printf("[DEBUG] Failed to release mutex.\n");
  }
}

/**
 * @brief Thread-safe version of log_message_with_location
 * @param level: Severity level of the message
 * @param file: Source file name
 * @param func: Function name
 * @param line: Line number
 * @param fmt: Printf-style format string
 * @param ...: Format arguments
 */
void elog_message_with_location_safe(log_level_t level, const char *file, const char *func, int line, const char *fmt, ...) {
  if (!s_mutex_initialized) {
    /* Fall back to non-thread-safe version */
    va_list args;
    char temp_buffer[LOG_MAX_MESSAGE_LENGTH - 64];  /* Reserve space for location info */
    
    va_start(args, fmt);
    vsnprintf(temp_buffer, sizeof(temp_buffer), fmt, args);
    va_end(args);
    
    int written = snprintf(s_message_buffer, sizeof(s_message_buffer), "[%s][%s][%d] %s", file, func, line, temp_buffer);
    if (written >= (int)sizeof(s_message_buffer)) {
      s_message_buffer[sizeof(s_message_buffer) - 1] = '\0';  /* Ensure null termination */
    }
    
    for (int i = 0; i < s_num_subscribers; i++) {
      if (s_subscribers[i].active && level >= s_subscribers[i].threshold) {
        s_subscribers[i].fn(level, s_message_buffer);
      }
    }
    return;
  }

  /* Take mutex with timeout */
  if (elogMutexTake(&s_log_mutex, ELOG_MUTEX_TIMEOUT_MS) != ELOG_THREAD_OK) {
    return; /* Skip logging if can't get mutex */
  }

  va_list args;
  char temp_buffer[LOG_MAX_MESSAGE_LENGTH - 64];  /* Reserve space for location info */
  
  /* Format the user message first */
  va_start(args, fmt);
  vsnprintf(temp_buffer, sizeof(temp_buffer), fmt, args);
  va_end(args);
  
  /* Add location information - ensure null termination */
  int written = snprintf(s_message_buffer, sizeof(s_message_buffer), "[%s][%s][%d] %s", file, func, line, temp_buffer);
  if (written >= (int)sizeof(s_message_buffer)) {
    s_message_buffer[sizeof(s_message_buffer) - 1] = '\0';  /* Ensure null termination */
  }
  
  /* Send to all active subscribers */
  for (int i = 0; i < s_num_subscribers; i++) {
    if (s_subscribers[i].active && level >= s_subscribers[i].threshold) {
      s_subscribers[i].fn(level, s_message_buffer);
    }
  }

  /* Give mutex */
  elogMutexGive(&s_log_mutex);
}

/**
 * @brief Thread-safe version of log_subscribe
 * @param fn: Function to call for each log message
 * @param threshold: Minimum level to send to this subscriber
 * @return Error code
 */
log_err_t elog_subscribe_safe(log_subscriber_t fn, log_level_t threshold) {
  if (!s_mutex_initialized) {
    /* Fall back to non-thread-safe version */
    return elog_subscribe(fn, threshold);
  }

  /* Take mutex with timeout */
  if (elogMutexTake(&s_log_mutex, ELOG_MUTEX_TIMEOUT_MS) != ELOG_THREAD_OK) {
    return LOG_ERR_SUBSCRIBERS_EXCEEDED; /* Return error if can't get mutex */
  }

  log_err_t result = LOG_ERR_SUBSCRIBERS_EXCEEDED;

  if (s_num_subscribers < LOG_MAX_SUBSCRIBERS) {
    /* Check if already subscribed */
    for (int i = 0; i < s_num_subscribers; i++) {
      if (s_subscribers[i].fn == fn && s_subscribers[i].active) {
        /* Update existing subscription */
        s_subscribers[i].threshold = threshold;
        result = LOG_ERR_NONE;
        goto exit;
      }
    }
    
    /* Add new subscriber */
    s_subscribers[s_num_subscribers].fn = fn;
    s_subscribers[s_num_subscribers].threshold = threshold;
    s_subscribers[s_num_subscribers].active = 1;
    s_num_subscribers++;
    result = LOG_ERR_NONE;
  }

exit:
  /* Give mutex */
  elogMutexGive(&s_log_mutex);
  return result;
}

/**
 * @brief Thread-safe version of log_unsubscribe
 * @param fn: Function to unsubscribe
 * @return Error code
 */
log_err_t elog_unsubscribe_safe(log_subscriber_t fn) {
  if (!s_mutex_initialized) {
    /* Fall back to non-thread-safe version */
    return elog_unsubscribe(fn);
  }

  /* Take mutex with timeout */
  if (elogMutexTake(&s_log_mutex, ELOG_MUTEX_TIMEOUT_MS) != ELOG_THREAD_OK) {
    return LOG_ERR_NOT_SUBSCRIBED; /* Return error if can't get mutex */
  }

  log_err_t result = LOG_ERR_NOT_SUBSCRIBED;

  for (int i = 0; i < s_num_subscribers; i++) {
    if (s_subscribers[i].fn == fn && s_subscribers[i].active) {
      s_subscribers[i].active = 0;
      result = LOG_ERR_NONE;
      break;
    }
  }

  /* Give mutex */
  elogMutexGive(&s_log_mutex);
  return result;
}

/**
 * @brief Get current task name (RTOS-specific)
 * @return Task name string or "UNKNOWN" if not available
 */
const char *elog_get_task_name(void) {
#if (ELOG_RTOS_TYPE == ELOG_RTOS_FREERTOS)
  #ifdef INC_FREERTOS_H
    TaskHandle_t current_task = xTaskGetCurrentTaskHandle();
    if (current_task != NULL) {
      return pcTaskGetName(current_task);
    }
  #endif
#elif (ELOG_RTOS_TYPE == ELOG_RTOS_THREADX)
  #ifdef TX_API_H
    TX_THREAD *current_thread = tx_thread_identify();
    if (current_thread != NULL) {
      return current_thread->tx_thread_name;
    }
  #endif
#elif (ELOG_RTOS_TYPE == ELOG_RTOS_CMSIS)
  #ifdef CMSIS_OS_H_
    const char *name = osThreadGetName(osThreadGetId());
    if (name != NULL) {
      return name;
    }
  #endif
#endif
  return "UNKNOWN";
}

/**
 * @brief Get current task ID (RTOS-specific)
 * @return Task ID or 0 if not available
 */
uint32_t elog_get_task_id(void) {
#if (ELOG_RTOS_TYPE == ELOG_RTOS_FREERTOS)
  #ifdef INC_FREERTOS_H
    TaskHandle_t current_task = xTaskGetCurrentTaskHandle();
    return (uint32_t)current_task;
  #endif
#elif (ELOG_RTOS_TYPE == ELOG_RTOS_THREADX)
  #ifdef TX_API_H
    TX_THREAD *current_thread = tx_thread_identify();
    if (current_thread != NULL) {
      return (uint32_t)current_thread;
    }
  #endif
#elif (ELOG_RTOS_TYPE == ELOG_RTOS_CMSIS)
  #ifdef CMSIS_OS_H_
    osThreadId_t id = osThreadGetId();
    return (uint32_t)id;
  #endif
#endif
  return 0;
}

/**
 * @brief Console subscriber with thread information
 * @param level: Severity level of the message
 * @param msg: Formatted message string
 */
void elog_console_subscriber_with_thread(log_level_t level, const char *msg) {
#if USE_COLOR
  /* Color codes for different log levels */
  const char* colors[] = {
    [LOG_LEVEL_TRACE]    = LOG_COLOR(LOG_COLOR_BLUE),     /* Blue for trace */
    [LOG_LEVEL_DEBUG]    = LOG_COLOR(LOG_COLOR_CYAN),     /* Cyan for debug */
    [LOG_LEVEL_INFO]     = LOG_COLOR(LOG_COLOR_GREEN),    /* Green for info */
    [LOG_LEVEL_WARNING]  = LOG_COLOR(LOG_COLOR_BROWN),    /* Brown/Yellow for warning */
    [LOG_LEVEL_ERROR]    = LOG_COLOR(LOG_COLOR_RED),      /* Red for error */
    [LOG_LEVEL_CRITICAL] = LOG_BOLD(LOG_COLOR_RED),       /* Bold Red for critical */
    [LOG_LEVEL_ALWAYS]   = LOG_BOLD("37")                 /* Bold White for always */
  };
  
  if (level >= LOG_LEVEL_TRACE && level <= LOG_LEVEL_ALWAYS) {
    printf("%s%s[%s]: %s%s\n", colors[level], elog_level_name(level), elog_get_task_name(), msg, LOG_RESET_COLOR);
  } else {
    printf("%s[%s]: %s\n", elog_level_name(level), elog_get_task_name(), msg);
  }
#else
  /* No color version */
  printf("%s[%s]: %s\n", elog_level_name(level), elog_get_task_name(), msg);
#endif
}

#endif /* ELOG_THREAD_SAFE */

/* ========================================================================== */
/* Thread-Safe Function Implementations (with fallbacks) */
/* ========================================================================== */

#if (ELOG_THREAD_SAFE == 0)
/* Fallback implementations when threading is disabled - just call regular versions */

void elog_message_safe(log_level_t level, const char *fmt, ...) {
  va_list args;
  va_start(args, fmt);
  vsnprintf(s_message_buffer, sizeof(s_message_buffer), fmt, args);
  va_end(args);
  
  for (int i = 0; i < s_num_subscribers; i++) {
    if (s_subscribers[i].active && level >= s_subscribers[i].threshold) {
      s_subscribers[i].fn(level, s_message_buffer);
    }
  }
}

void elog_message_with_location_safe(log_level_t level, const char *file, const char *func, int line, const char *fmt, ...) {
  va_list args;
  char temp_buffer[LOG_MAX_MESSAGE_LENGTH - 64];  /* Reserve space for location info */
  
  va_start(args, fmt);
  vsnprintf(temp_buffer, sizeof(temp_buffer), fmt, args);
  va_end(args);
  
  int written = snprintf(s_message_buffer, sizeof(s_message_buffer), "[%s][%s][%d] %s", file, func, line, temp_buffer);
  if (written >= (int)sizeof(s_message_buffer)) {
    s_message_buffer[sizeof(s_message_buffer) - 1] = '\0';  /* Ensure null termination */
  }
  
  for (int i = 0; i < s_num_subscribers; i++) {
    if (s_subscribers[i].active && level >= s_subscribers[i].threshold) {
      s_subscribers[i].fn(level, s_message_buffer);
    }
  }
}

log_err_t elog_subscribe_safe(log_subscriber_t fn, log_level_t threshold) {
  return elog_subscribe(fn, threshold);
}

log_err_t elog_unsubscribe_safe(log_subscriber_t fn) {
  return elog_unsubscribe(fn);
}

#endif /* ELOG_THREAD_SAFE == 0 */

#if (ELOG_THREAD_SAFE == 0)
#include <pthread.h>
static pthread_mutex_t s_log_mutex = PTHREAD_MUTEX_INITIALIZER;

elog_thread_result_t elogMutexCreate(elog_mutex_t *mutex) {
  (void)mutex; // Mutex is statically initialized
  return ELOG_THREAD_OK;
}

elog_thread_result_t elogMutexTake(elog_mutex_t *mutex, uint32_t timeout_ms) {
  (void)timeout_ms; // Timeout not supported in bare-metal implementation
  if (pthread_mutex_lock(&s_log_mutex) == 0) {
    return ELOG_THREAD_OK;
  }
  return ELOG_THREAD_ERROR;
}

elog_thread_result_t elogMutexGive(elog_mutex_t *mutex) {
  (void)mutex; // Mutex is statically initialized
  if (pthread_mutex_unlock(&s_log_mutex) == 0) {
    return ELOG_THREAD_OK;
  }
  return ELOG_THREAD_ERROR;
}

elog_thread_result_t elogMutexDelete(elog_mutex_t *mutex) {
  (void)mutex; // Mutex is statically initialized
  return ELOG_THREAD_OK;
}
#endif

log_err_t elog_set_file_threshold(const char *filename, log_level_t threshold) {
    if (!filename) return LOG_ERR_INVALID_LEVEL;
    for (int i = 0; i < fileLogLevelCount; ++i) {
        if (strcmp(fileLogLevels[i].filename, filename) == 0) {
            fileLogLevels[i].threshold = threshold;
            return LOG_ERR_NONE;
        }
    }
    if (fileLogLevelCount < MAX_FILE_LOG_LEVELS) {
        strncpy(fileLogLevels[fileLogLevelCount].filename, filename, sizeof(fileLogLevels[fileLogLevelCount].filename) - 1);
        fileLogLevels[fileLogLevelCount].filename[sizeof(fileLogLevels[fileLogLevelCount].filename) - 1] = '\0';
        fileLogLevels[fileLogLevelCount].threshold = threshold;
        fileLogLevelCount++;
        return LOG_ERR_NONE;
    }
    return LOG_ERR_SUBSCRIBERS_EXCEEDED;
}

log_level_t elog_get_file_threshold(const char *filename) {
    if (!filename) return LOG_AUTO_THRESHOLD;
    for (int i = 0; i < fileLogLevelCount; ++i) {
        if (strcmp(fileLogLevels[i].filename, filename) == 0) {
            return fileLogLevels[i].threshold;
        }
    }
    return LOG_AUTO_THRESHOLD;
}
