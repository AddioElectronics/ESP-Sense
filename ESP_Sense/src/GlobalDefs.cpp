#include "GlobalDefs.h"


const char* config_source_strings[5]		= { "default", "fs", "eeprom", "backupfs", "firmware" };
const char* wifi_power_strings[12]			= {"19.5dBm", "19dBm", "18.5dBm", "17dBm", "15dBm", "13dBm", "11dBm", "8.5dBm", "7dBm", "5dBm", "2dBm", "-1dBm"};
const char* esp_updater_strings[4]			= { "usb", "http", "com", "all" };
const char* ota_rollbackMode_strings[3]		= { "off", "auto", "manual" };
const char* esp_autobackup_strings[4]		= { "no", "fs", "eeprom", "all" };
const char* wifi_mode_strings[4]			= { "def", "sta", "ap", "apsta"};
const char* uart_parity_strings[3]			= { "none", "even", "odd" };

ConfigSource config_source_values[5]		= { ConfigSource::CFG_DEFAULT, ConfigSource::CFG_FILESYSTEM, ConfigSource::CFG_EEPROM, ConfigSource::CFG_BACKUP_FILESYSTEM, ConfigSource::CFG_FIRMWARE };
wifi_power_t wifi_power_values[12]			= { WIFI_POWER_19_5dBm ,WIFI_POWER_19dBm, WIFI_POWER_18_5dBm, WIFI_POWER_17dBm, WIFI_POWER_15dBm, WIFI_POWER_13dBm, WIFI_POWER_11dBm , WIFI_POWER_8_5dBm , WIFI_POWER_7dBm, WIFI_POWER_5dBm, WIFI_POWER_2dBm, WIFI_POWER_MINUS_1dBm };
UART_DATABITS uart_databits_values[4]		= { UART_DATABITS::DATABITS_5, UART_DATABITS::DATABITS_6, UART_DATABITS::DATABITS_7, UART_DATABITS::DATABITS_8 };
UART_PARITY uart_parity_values[3]			= { UART_PARITY::PARITY_NONE, UART_PARITY::PARITY_EVEN, UART_PARITY::PARITY_ODD};
UART_STOPBITS uart_stopbits_values[2]		= { UART_STOPBITS::STOPBITS_1, UART_STOPBITS::STOPBITS_2};

