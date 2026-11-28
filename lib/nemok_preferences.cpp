// Nemo-K configuration preferences.
// Included from src/nemo-k.cpp.
//
// All configuration variables are listed once below with their Preferences
// key name, type and default value. Each variable becomes an enum constant
// (NemokPreferences::HOSTNAME, NemokPreferences::DELAY, ...), so:
// * a misspelled variable is a compile error, and
// * using a variable with the wrong type is a compile error, because string
//   and int variables are different enum types.
//
// Values are read from LittleFS once in begin() and kept in memory; getters
// are cheap. Setters write to LittleFS and update the in-memory value.
//
// Variables are stored under /nvs/nemok/<key>. Key names must be 8 characters
// or less and must match the web frontend that creates the filesystem.
// EspSigK keeps its own preferences under /nvs/EspSigK/ and is not handled here.

#include <Preferences.h>

#define NEMOK_PREFERENCES_NAMESPACE "nemok"

//  X(enum name, key name,   default value)
#define NEMOK_PREFERENCES_STRINGS(X)   \
    X(HOSTNAME, "hostname", "sensor")  \
    X(ADMPASS, "admpass", "")          \
    X(SSID, "ssid", "nemok")           \
    X(SSIDPASS, "ssidpass", "nemok")   \
    X(KEY, "key", "undefinedkey")      \
    X(SRVHOST, "srvhost", "nemok.org") \
    X(SRVTOKEN, "srvtoken", "")

#define NEMOK_PREFERENCES_INTS(X) \
    X(DELAY, "delay", 1000)       \
    X(SRVPORT, "srvport", 3000)   \
    X(ADCTANK0, "adctank0", 0)    \
    X(ADCTANK1, "adctank1", 0)

class NemokPreferences
{
public:
#define NEMOK_PREFERENCES_ENUM(name, key, defaultValue) name,
    enum StringKey
    {
        NEMOK_PREFERENCES_STRINGS(NEMOK_PREFERENCES_ENUM)
        STRING_KEY_COUNT
    };
    enum IntKey
    {
        NEMOK_PREFERENCES_INTS(NEMOK_PREFERENCES_ENUM)
        INT_KEY_COUNT
    };
#undef NEMOK_PREFERENCES_ENUM

    // Opens the Preferences store and reads all variables into memory
    void begin()
    {
        prefs.begin(NEMOK_PREFERENCES_NAMESPACE);

        for (int n = 0; n < STRING_KEY_COUNT; n++)
        {
            stringDefined[n] = prefs.isKey(stringKeyNames[n]);
            stringValues[n] = prefs.getString(stringKeyNames[n], stringDefaults[n]);
        }

        for (int n = 0; n < INT_KEY_COUNT; n++)
        {
            intDefined[n] = prefs.isKey(intKeyNames[n]);
            intValues[n] = prefs.getInt(intKeyNames[n], intDefaults[n]);
        }
    }

    // True if the variable exists in LittleFS (i.e. it was created for this
    // sensor or has been set since). Undefined variables return their default.
    bool isDefined(StringKey key) const
    {
        return stringDefined[key];
    }

    bool isDefined(IntKey key) const
    {
        return intDefined[key];
    }

    const String &get(StringKey key) const
    {
        return stringValues[key];
    }

    int32_t get(IntKey key) const
    {
        return intValues[key];
    }

    // Returns true if the value was written to LittleFS
    bool set(StringKey key, const String &value)
    {
        stringValues[key] = value;
        stringDefined[key] = true;
        return prefs.putString(stringKeyNames[key], value) == value.length();
    }

    bool set(IntKey key, int32_t value)
    {
        intValues[key] = value;
        intDefined[key] = true;
        return prefs.putInt(intKeyNames[key], value) == sizeof(int32_t);
    }

    // Preferences key name (e.g. "hostname"), for the web server forms
    static const char *keyName(StringKey key)
    {
        return stringKeyNames[key];
    }

    static const char *keyName(IntKey key)
    {
        return intKeyNames[key];
    }

private:
    Preferences prefs;
    String stringValues[STRING_KEY_COUNT];
    int32_t intValues[INT_KEY_COUNT];
    bool stringDefined[STRING_KEY_COUNT];
    bool intDefined[INT_KEY_COUNT];

#define NEMOK_PREFERENCES_KEY_NAME(name, key, defaultValue) key,
#define NEMOK_PREFERENCES_DEFAULT(name, key, defaultValue) defaultValue,
    static constexpr const char *stringKeyNames[STRING_KEY_COUNT] = {NEMOK_PREFERENCES_STRINGS(NEMOK_PREFERENCES_KEY_NAME)};
    static constexpr const char *stringDefaults[STRING_KEY_COUNT] = {NEMOK_PREFERENCES_STRINGS(NEMOK_PREFERENCES_DEFAULT)};
    static constexpr const char *intKeyNames[INT_KEY_COUNT] = {NEMOK_PREFERENCES_INTS(NEMOK_PREFERENCES_KEY_NAME)};
    static constexpr int32_t intDefaults[INT_KEY_COUNT] = {NEMOK_PREFERENCES_INTS(NEMOK_PREFERENCES_DEFAULT)};
#undef NEMOK_PREFERENCES_KEY_NAME
#undef NEMOK_PREFERENCES_DEFAULT
};

NemokPreferences nemokPrefs;
