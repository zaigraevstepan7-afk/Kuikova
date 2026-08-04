#include "c_photon_player.h"

void *c_photon_player::player_prop(const char *key)
{
    if (!key)
        return nullptr;

    auto hashtable = this->m_pCustomProperties;
    if (!hashtable || !hashtable->klass || !hashtable->entries)
        return nullptr;

    for (int i = 0; i < hashtable->count; i++)
    {
        auto &h_entry = hashtable->entries->m_Items[i];
        if (h_entry.hashCode != 0 && h_entry.key)
        {
            auto h_key = h_entry.key->c_str();
            if (h_key && strcmp(h_key, key) == 0)
                return h_entry.value;
        }
    }

    return nullptr;
}

int c_photon_player::get_health()
{
    auto prop = (boxedValue<int32_t> *)player_prop(oxorany("health"));
    if (prop)
        return prop->value;
    return -1;
}