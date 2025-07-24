# eLog - Enhanced Logging System for Embedded MCU Projects

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![Platform](https## 🧵 RTOS Threading Configuration

> ⚠️ **CONFIGURATION ORDER WARNING**: All custom `#define` statements must be placed **AFTER line 28** in `eLog.h` where RTOS type constants (`ELOG_RTOS_FREERTOS`, `ELOG_RTOS_THREADX`, etc.) are defined. Incorrect placement will result in "undefined identifier" compilation errors.

### Thread Safety Optionsimg.shields.io/badge/Platform-ARM%20Cortex--M-blue.svg)](https://developer.arm.com/architectures/cpu-architecture/m-profile)
[![C Standard](https://img.shields.io/badge/C-C99-green.svg)](https://en.wikipedia.org/wiki/C99)

A comprehensive, lightweight, and feature-rich logging system designed specifically for embedded microcontroller projects. Inspired by uLog but significantly enhanced with modern features, backwards compatibility, and embedded-first design.

## 🚀 Features

### ✨ Core Capabilities
- **Multiple Subscribers**: Support for up to 6 concurrent logging outputs (console, file, memory buffer, network, etc.)
- **Compile-time Optimization**: Individual log levels can be disabled at compile time for minimal footprint
- **Color Support**: Built-in ANSI color coding for better terminal debugging experience  
- **Location Information**: Optional file/function/line information for debugging
- **Zero External Dependencies**: Only requires standard C library (stdio.h, stdarg.h, string.h)

### 🔒 RTOS Threading Support (NEW!)
- **Thread-Safe Operations**: Mutex-protected logging operations for multi-threaded environments
- **Multiple RTOS Support**: FreeRTOS, Azure ThreadX, CMSIS-RTOS compatibility
- **Task Information**: Automatic task name and ID integration in log messages
- **Timeout Protection**: Configurable mutex timeouts to prevent blocking
- **Graceful Fallback**: Automatic fallback to non-threaded mode when RTOS unavailable
- **Zero Overhead**: Thread safety can be completely disabled for single-threaded applications

### 🎯 Embedded-Specific Features
- **Backwards Compatibility**: Drop-in replacement for existing `printIF()`, `printERR()`, `printLOG()` macros
- **Memory Efficient**: 128-byte message buffer, optimized for resource-constrained systems
- **RTOS Ready**: Thread-safe design suitable for FreeRTOS, ThreadX, and other RTOSs
- **MCU Error Codes**: Comprehensive set of error codes for common MCU subsystems
- **Auto-threshold Calculation**: Intelligent threshold detection based on enabled debug levels

### 📊 Performance Benefits
- **Compile-time Elimination**: Disabled log levels generate zero code
- **Subscriber Pattern**: Efficient message distribution to multiple outputs
- **Minimal Runtime Overhead**: Optimized for real-time embedded applications

## 🏆 Comparison with Original uLog

| Feature | eLog | uLog | Advantage |
|---------|------|------|-----------|
| **Thread Safety** | ✅ Multi-RTOS | ❌ None | **eLog** |
| **Compile-time Optimization** | ✅ Per-level | ❌ All-or-nothing | **eLog** |
| **Backwards Compatibility** | ✅ Full | ❌ None | **eLog** |
| **Built-in Console Colors** | ✅ ANSI colors | ❌ User provides | **eLog** |
| **MCU Error Codes** | ✅ Comprehensive | ❌ None | **eLog** |
| **Auto-threshold** | ✅ Smart detection | ❌ Manual only | **eLog** |
| **Location Info** | ✅ File/line/func | ❌ User adds | **eLog** |
| **Legacy Integration** | ✅ Seamless | ❌ Requires migration | **eLog** |
| **Task Information** | ✅ Auto-detect | ❌ None | **eLog** |
| **Memory Usage** | 128B buffer | 120B buffer | **eLog** |
| **Max Subscribers** | 6 (configurable) | 6 (configurable) | Tie |

## 📦 Quick Start

### Installation
Simply copy `eLog.h` and `eLog.c` to your project and include them in your build system.

```c
#include "eLog.h"

int main() {
    // Initialize with console output and automatic threshold
    LOG_INIT_WITH_CONSOLE_AUTO();
    
    // Your existing code works unchanged!
    printIF("System initialized");           // Legacy macro
    printERR("Error code: 0x%02X", COMM_ERR_I2C);
    
    // Or use enhanced logging
    LOG_INFO("Battery level: %d%%", battery_level);
    LOG_ERROR("Sensor failure: 0x%02X", SENSOR_ERR_NOT_FOUND);
    
    return 0;
}
```

### Configuration
Configure debug levels in `eLog.h`:

> ⚠️ **IMPORTANT**: Any custom configuration must be placed **AFTER line 28** in `eLog.h` where the RTOS type constants are defined. Placing configuration before this line will cause compilation errors due to undefined RTOS constants.

```c
#define DEBUG_INFO YES      /* Information messages */
#define DEBUG_WARN YES      /* Warning messages */
#define DEBUG_ERR  YES      /* Error messages */
#define DEBUG_LOG  YES      /* Debug messages */
#define DEBUG_TRACE NO      /* Trace messages (verbose) */
#define DEBUG_CRITICAL YES  /* Critical errors */
#define DEBUG_ALWAYS YES    /* Always logged messages */
```

## 🛠️ Advanced Usage

### Custom Subscribers
```c
void my_file_logger(log_level_t level, const char *msg) {
    FILE *log_file = fopen("system.log", "a");
    fprintf(log_file, "[%s] %s\n", log_level_name(level), msg);
    fclose(log_file);
}

// Subscribe custom logger for ERROR and above
LOG_SUBSCRIBE(my_file_logger, LOG_LEVEL_ERROR);
```

### Multiple Output Destinations
```c
LOG_INIT();

// Console for all messages
LOG_SUBSCRIBE(log_console_subscriber, LOG_LEVEL_DEBUG);

// File for errors only
LOG_SUBSCRIBE(my_file_logger, LOG_LEVEL_ERROR);

// Network for critical alerts
LOG_SUBSCRIBE(my_network_logger, LOG_LEVEL_CRITICAL);

// Memory buffer for debugging
LOG_SUBSCRIBE(my_memory_logger, LOG_LEVEL_TRACE);
```

### Error Code Integration
```c
// Comprehensive MCU error codes included
if (i2c_status != HAL_OK) {
    LOG_ERROR("I2C communication failed: 0x%02X", COMM_ERR_I2C);
}

if (battery_voltage < MIN_VOLTAGE) {
    LOG_WARNING("Low battery: 0x%02X", PWR_ERR_LOW_VOLTAGE);
}

// Critical system errors
if (stack_overflow_detected) {
    LOG_CRITICAL("Stack overflow detected: 0x%02X", CRITICAL_ERR_STACK);
}
```

## 📚 Documentation

### Log Levels
- `LOG_LEVEL_TRACE` (100): Function entry/exit, detailed flow
- `LOG_LEVEL_DEBUG` (101): Variable values, state changes  
- `LOG_LEVEL_INFO` (102): Normal operation events
- `LOG_LEVEL_WARNING` (103): Recoverable errors, performance issues
- `LOG_LEVEL_ERROR` (104): Serious problems requiring attention
- `LOG_LEVEL_CRITICAL` (105): System failures, unrecoverable errors
- `LOG_LEVEL_ALWAYS` (106): Essential system messages

### Available Macros

#### Enhanced Logging
```c
LOG_TRACE("Function entry: %s", __func__);
LOG_DEBUG("Variable x = %d", x);
LOG_INFO("System ready");
LOG_WARNING("Performance degraded");  
LOG_ERROR("Operation failed: 0x%02X", error_code);
LOG_CRITICAL("System failure");
LOG_ALWAYS("Boot complete");
```

#### Legacy Compatibility
```c
printTRACE("Trace message");
printLOG("Debug message");
printIF("Info message");
printWRN("Warning message");
printERR("Error message");
printCRITICAL("Critical message");
printALWAYS("Always logged");
```

### Error Code Categories
- **System Errors** (0x10-0x1F): Core system operations
- **Communication** (0x20-0x3F): UART, I2C, SPI, CAN, BLE, WiFi, Ethernet
- **Sensors** (0x40-0x5F): Accelerometer, gyroscope, pressure, temperature
- **Power Management** (0x60-0x7F): Voltage, current, thermal, charging
- **Storage** (0x80-0x9F): Flash, EEPROM, SD card operations
- **Application** (0xA0-0xBF): App-specific functionality
- **Hardware** (0xC0-0xDF): GPIO, timers, ADC, DAC, PWM
- **RTOS** (0xE0-0xEF): Tasks, queues, semaphores, mutexes
- **Critical** (0xF0-0xFF): Stack, heap, hard faults, assertions

## � RTOS Threading Configuration

### Thread Safety Options
```c
/* Enable/disable thread safety */
#define ELOG_THREAD_SAFE 1              /* 1=enabled, 0=disabled */

/* RTOS selection */
#define ELOG_RTOS_TYPE ELOG_RTOS_FREERTOS
// Options: ELOG_RTOS_FREERTOS, ELOG_RTOS_THREADX, ELOG_RTOS_CMSIS, ELOG_RTOS_NONE

/* Mutex timeout configuration */
#define ELOG_MUTEX_TIMEOUT_MS 100       /* Timeout in milliseconds */
```

### RTOS Integration Examples

#### FreeRTOS Setup
```c
#define ELOG_THREAD_SAFE 1
#define ELOG_RTOS_TYPE ELOG_RTOS_FREERTOS

#include "FreeRTOS.h"
#include "eLog.h"

void task1(void *pvParameters) {
    LOG_INIT_WITH_THREAD_INFO();  /* Initialize with task name in logs */
    
    for(;;) {
        LOG_INFO("Task 1 is running");
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

void task2(void *pvParameters) {
    for(;;) {
        LOG_DEBUG("Task 2 processing data");
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}
```

#### ThreadX Setup
```c
#define ELOG_THREAD_SAFE 1
#define ELOG_RTOS_TYPE ELOG_RTOS_THREADX

#include "tx_api.h"
#include "eLog.h"

void thread_entry(ULONG thread_input) {
    LOG_INIT_WITH_CONSOLE_AUTO();
    
    while(1) {
        LOG_INFO("ThreadX thread [%s] executing", elog_get_task_name());
        tx_thread_sleep(100);
    }
}
```

#### Bare Metal (No RTOS)
```c
#define ELOG_THREAD_SAFE 0              /* Disable threading for bare metal */
#define ELOG_RTOS_TYPE ELOG_RTOS_NONE

#include "eLog.h"

int main(void) {
    LOG_INIT_WITH_CONSOLE_AUTO();
    LOG_INFO("Bare metal application started");
    
    while(1) {
        // Main loop
        LOG_DEBUG("Main loop iteration");
    }
}
```

### Thread-Safe API Functions
```c
/* Thread-safe versions (automatically used when ELOG_THREAD_SAFE=1) */
void log_message_safe(log_level_t level, const char *fmt, ...);
void log_message_with_location_safe(log_level_t level, const char *file, const char *func, int line, const char *fmt, ...);
log_err_t log_subscribe_safe(log_subscriber_t fn, log_level_t threshold);
log_err_t log_unsubscribe_safe(log_subscriber_t fn);

/* Task information functions */
const char *elog_get_task_name(void);    /* Get current task name */
uint32_t elog_get_task_id(void);         /* Get current task ID */

/* Thread-aware console subscriber */
void log_console_subscriber_with_thread(log_level_t level, const char *msg);
```

### Performance Considerations
- **Thread Safety Overhead**: ~50-100 CPU cycles per log call (mutex operations)
- **Memory Overhead**: Additional ~32-64 bytes for mutex storage
- **Timeout Behavior**: Logging calls will timeout and skip if mutex cannot be acquired
- **RTOS Integration**: Minimal impact on existing RTOS task scheduling

## �🔧 Configuration Options

```c
/* Subscriber configuration */
#define LOG_MAX_SUBSCRIBERS 6           /* Maximum concurrent outputs */

/* Message buffer size */
#define LOG_MAX_MESSAGE_LENGTH 128      /* Buffer size for formatted messages */

/* Color support */
#define USE_COLOR 1                     /* Enable ANSI colors in console */

/* Location information */
#define ENABLE_DEBUG_MESSAGES_WITH_FILE_NAME 0  /* Include file/line info */
```

## 💾 Memory Usage

### Base System
- **ROM**: ~2-4KB (depending on enabled features)
- **RAM**: ~200 bytes static allocation  
- **Stack**: ~128 bytes per log call (message buffer)

### With Thread Safety (ELOG_THREAD_SAFE=1)
- **Additional ROM**: ~1-2KB (RTOS abstraction layer)
- **Additional RAM**: ~32-64 bytes (mutex storage)
- **Runtime Overhead**: ~50-100 CPU cycles per log call (mutex operations)

### Memory Optimization Tips
- Set `ELOG_THREAD_SAFE=0` for single-threaded applications
- Reduce `LOG_MAX_MESSAGE_LENGTH` for memory-constrained systems
- Disable unused log levels at compile time
- Use `LOG_MAX_SUBSCRIBERS` to limit subscriber array size

## 🧪 Testing

The library includes comprehensive examples in `eLog_example.c` and `eLog_example_rtos.c`:

### Basic Examples
- Basic logging demonstration
- Multiple subscriber setup  
- Error code usage
- Legacy compatibility testing
- Performance testing
- Memory usage validation

### RTOS Threading Examples (NEW!)
- Thread safety demonstration
- Multi-task logging scenarios
- Task information integration
- RTOS-specific feature testing
- Mutex timeout behavior
- Performance impact measurement

## 🚀 Performance

- **Zero overhead** for disabled log levels (compile-time elimination)
- **Minimal runtime cost** for enabled levels
- **Efficient subscriber pattern** for multiple outputs
- **Optimized for embedded** real-time constraints

## 🤝 Contributing

Contributions are welcome! Please follow these guidelines:

1. **Code Style**: Follow existing coding conventions
2. **Documentation**: Update README for new features
3. **Testing**: Test on multiple embedded platforms
4. **Backwards Compatibility**: Maintain existing API compatibility

### Development Setup
```bash
git clone <your-repo>
cd eLog
# Include in your embedded project build system
```

## 📄 License

MIT License - see [LICENSE](LICENSE) file for details.

## 🙏 Acknowledgments

- Inspired by [uLog](https://github.com/rdpoor/ulog) by R. Dunbar Poor
- Enhanced for embedded systems and production use
- Designed for STM32, ESP32, and other ARM Cortex-M platforms

## 📞 Support

- **Issues**: Please use GitHub issues for bug reports and feature requests
- **Documentation**: See examples in `eLog_example.c`
- **Community**: Contributions and feedback welcome

---

**eLog** - Making embedded debugging easier, one log message at a time! 🚀
