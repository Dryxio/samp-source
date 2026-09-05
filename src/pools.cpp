// Minimal layouts reconstructed from R5 machine-code accesses.
// Names/semantic leads: samp-reversed; layouts checked against frozen R5.
// Ordinary C++ only: no copied instructions, inline assembly, or _emit.
#pragma pack(push, 1)
class PlayerPool {
public:
    char prefix[0x2a];
    int slots[1004];
    char gap[0xfb0];
    struct NetPlayer { char prefix[0x10]; void* remote; } *players[1004];
    int GetSlotState(unsigned short id);
    void* GetAt(unsigned short id);
};
int PlayerPool::GetSlotState(unsigned short id) {
    if (id >= 1004) return 0;
    return slots[id];
}
void* PlayerPool::GetAt(unsigned short id) {
    if (id > 1004 || !players[id]) return 0;
    return players[id]->remote;
}
class LocalPlayer {
public:
    char prefix[0x94];
    unsigned short key3, key2, key1;
    int AreKeysChanged(unsigned short a, unsigned short b, unsigned short c);
};
int LocalPlayer::AreKeysChanged(unsigned short a, unsigned short b, unsigned short c) {
    if (a == key1 && b == key2 && c == key3) return 0;
    return 1;
}
#pragma pack(pop)
