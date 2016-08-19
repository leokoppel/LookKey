#pragma once
#include "../simpleini/SimpleIni.h"

const PWSTR INI_FILENAME = L"lookkey.ini";

typedef struct {
    PWSTR section;
    PWSTR iniKey;
    PWSTR default;
} const CONFIG_KEY_STR;

typedef struct {
    PWSTR section;
    PWSTR iniKey;
    long default;
} const CONFIG_KEY_INT;

typedef struct {
    PWSTR section;
    PWSTR iniKey;
    double default;
} const CONFIG_KEY_DOUBLE;

typedef struct {
    PWSTR section;
    PWSTR iniKey;
    bool default;
} const CONFIG_KEY_BOOL;




class ConfigManager
{
public:
    ConfigManager(const PWSTR filename = INI_FILENAME);
    ~ConfigManager();

    const WCHAR * GetString(const PWSTR key, const PWSTR default) const;
    double GetDouble(const PWSTR key, const double default) const;
    int GetInt(const PWSTR key, const int default) const;
    bool GetBool(const PWSTR key, const bool default) const;

    void SetString(const PWSTR key, const PWSTR value, const PWSTR comment = NULL);
    void SetDouble(const PWSTR key, const double value, const PWSTR comment = NULL);
    void SetInt(const PWSTR key, const long value, const PWSTR comment = NULL);
    void SetBool(const PWSTR key, const bool value, const PWSTR comment = NULL);

    const WCHAR * GetValue(CONFIG_KEY_STR & configKey) const;
    int GetValue(CONFIG_KEY_INT & configKey) const;
    double GetValue(CONFIG_KEY_DOUBLE & configKey) const;
    bool GetValue(CONFIG_KEY_BOOL & configKey) const;

    void SetValue(CONFIG_KEY_STR & configKey, const PWSTR value, const PWSTR comment = NULL);
    void SetValue(CONFIG_KEY_INT & configKey, int value, const PWSTR comment = NULL, bool useHex = false);
    void SetValue(CONFIG_KEY_DOUBLE & configKey, double value, const PWSTR comment = NULL);
    void SetValue(CONFIG_KEY_BOOL & configKey, bool value, const PWSTR comment = NULL);

    void SaveFile();

private:
    WCHAR m_filename[255];
    CSimpleIni m_ini;
};



inline void throw_if_fail(SI_Error se)
{
    if (se < 0)
    {
        throw _com_error(E_FAIL);
    }
}

