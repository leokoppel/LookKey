#include "stdafx.h"
#include "Config.h"
#include "LookKey.h"


ConfigManager::ConfigManager(const PWSTR filename) :
    m_ini{}
{
    // Throw if filename is too large to store
    throw_if_fail(
        StringCchCopy(m_filename, ARRAYSIZE(m_filename), filename)
    );

    m_ini.SetUnicode();

    SI_Error se = m_ini.LoadFile(filename);
    if (se == SI_FILE)
    {
        //ignore
    }
    else
    {
        throw_if_fail(se);
    }

}


ConfigManager::~ConfigManager()
{
    SaveFile();
}


const WCHAR * ConfigManager::GetValue(CONFIG_KEY_STR & configKey) const
{
    return m_ini.GetValue(configKey.section, configKey.iniKey, configKey.default);
}

int ConfigManager::GetValue(CONFIG_KEY_INT & configKey) const
{
    return m_ini.GetLongValue(configKey.section, configKey.iniKey, configKey.default);
}

double ConfigManager::GetValue(CONFIG_KEY_DOUBLE & configKey) const
{
    return m_ini.GetDoubleValue(configKey.section, configKey.iniKey, configKey.default);
}

bool ConfigManager::GetValue(CONFIG_KEY_BOOL & configKey) const
{
    return m_ini.GetBoolValue(configKey.section, configKey.iniKey, configKey.default);
}



void ConfigManager::SetValue(CONFIG_KEY_STR & configKey, const PWSTR value, const PWSTR comment)
{
    throw_if_fail(
        m_ini.SetValue(configKey.section, configKey.iniKey, value, comment, true)
    ); 
}

void ConfigManager::SetValue(CONFIG_KEY_INT & configKey, int value, const PWSTR comment, bool useHex)
{
    throw_if_fail(
        m_ini.SetLongValue(configKey.section, configKey.iniKey, value, comment, useHex)
    );
}

void ConfigManager::SetValue(CONFIG_KEY_DOUBLE & configKey, double value, const PWSTR comment)
{
    throw_if_fail(
        m_ini.SetDoubleValue(configKey.section, configKey.iniKey, value, comment, true)
    );
}

void ConfigManager::SetValue(CONFIG_KEY_BOOL & configKey, bool value, const PWSTR comment)
{
    throw_if_fail(
        m_ini.SetBoolValue(configKey.section, configKey.iniKey, value, comment, true)
    );
}

void ConfigManager::SaveFile()
{
    m_ini.SaveFile(m_filename);

}

