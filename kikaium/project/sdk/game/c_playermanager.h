#pragma once
using namespace structs;
class c_player_controller;
class c_material;

#pragma pack(1)
class c_playermanager {
    char pad[0x24];
    public:
    int m_iUnk; //0x24
    monoDictionary<int, c_player_controller*> *players_by_id; //0x28
    monoDictionary<int, int> *unk; //0x30
    void *hash_set; //0x38
    monoDictionary<int, c_material*> *material; //0x40
    void *hashset2; //0x48;
    void *action; //0x50
    void *type; //0x58
    bool m_bUnk; //0x60
    char pad1[0x7];
    c_player_controller *__BackingField;//0x68
    c_player_controller *local_player; //0x70
    monoDictionary<char, void *>* pools; //0x78
};
#pragma pack()
