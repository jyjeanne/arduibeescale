/**
 * CONFIGURATION TEMPLATE FOR ArduiBeeScale
 *
 * Project: ArduiBeeScale (Arduino Beehive Scale)
 * Maintained by: Jeremy JEANNE
 * License: GNU GPLv3
 *
 * SETUP INSTRUCTIONS:
 * 1. Copy this file: config_template.h -> config.h
 * 2. Edit config.h with your actual credentials
 * 3. DO NOT commit config.h to version control
 * 4. Add "config.h" to your .gitignore file
 *
 * This separation ensures sensitive credentials (like API keys) are never
 * accidentally exposed in public repositories.
 *
 * See SETUP_GUIDE.md for detailed configuration instructions.
 */

#ifndef CONFIG_H
#define CONFIG_H

// ============================================
// API CONFIGURATION
// ============================================

/**
 * Your API key from https://beta.btree.at/setting/profile/api
 *
 * Example: "a1b2c3d4e5f6a7b8c9d0e1f2a3b4c5d6"
 *
 * WARNING: Do not share this key publicly!
 */
const char API_KEY[] PROGMEM = "ENTER_YOUR_API_KEY_HERE";

// ============================================
// GSM/NETWORK CONFIGURATION
// ============================================

/**
 * APN (Access Point Name) for your cellular provider.
 *
 * Common APNs:
 * - Vodafone Austria: "vodafone.at"
 * - HOT/T-Mobile Austria: "webaut"
 * - A1: "A1.net"
 * - UPC/Magenta: "upc.at"
 */
const char APN_CONFIG[] PROGMEM = "webaut";

/**
 * MQTT/Device identifier - must be created in your bTree account.
 * See: https://beta.btree.at/table/scale_data
 */
const char DEVICE_IDENT[] PROGMEM = "ScaleHouse";

/**
 * Timezone for server-side timestamp (PHP format).
 * Common timezones: "Europe/Vienna", "Europe/Berlin", "UTC"
 * See: http://www.php.net/manual/en/timezones.php
 */
const char TIMEZONE_CONFIG[] PROGMEM = "Europe/Vienna";

// ============================================
// API ENDPOINT CONFIGURATION
// ============================================

/**
 * bTree API endpoint
 * Production: "api.btree.at"
 * Testing: "requestb.in" (with custom URL)
 */
const char API_URL[] PROGMEM = "api.btree.at";
const char API_PATH[] PROGMEM = "/api/v1/external";

/**
 * Action type for data submission
 * "CREATE" - Normal submission
 * "CREATE_DEMO" - Test mode (no data saved)
 */
const char API_ACTION[] PROGMEM = "CREATE";

#endif  // CONFIG_H
