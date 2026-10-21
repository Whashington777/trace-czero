#include "icons.h"

namespace icons
{
    const char* to_utf8(tab icon)
    {
        switch (icon)
        {
        case tab::visuals:  return visuals;
        case tab::aim:      return aim;
        case tab::config:   return config;
        case tab::settings: return settings;
        default:            return aim;
        }
    }
}
