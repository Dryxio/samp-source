// R5 layouts and loop bodies checked against frozen machine code.
// Semantic leads: samp-reversed/saco/net/actorpool.cpp; see provenance.json.
#pragma pack(push, 1)
class ActorPool {
public:
    int lastActive;
    void* actors[1000];
    int slots[1000];
    int gtaIds[1000];
    ActorPool();
    void UpdateCount();
    unsigned short FindIDFromGtaPtr(int gtaId);
    int Delete(unsigned short id); // external R5 dependency, not implemented
    void DeleteAll();
};
#pragma pack(pop)

ActorPool::ActorPool() {
    int i = 1000;
    unsigned long* p = (unsigned long*)this;
    do {
        p[0x3e9] = 0;
        p[1] = 0;
        p[0x7d1] = 0;
        --i;
        ++p;
    } while (i != 0);
    lastActive = 0;
}

void ActorPool::UpdateCount() {
    int last = 0;
    for (int i = 0; i < 1000; ++i) {
        if (slots[i]) last = i;
    }
    lastActive = last;
}

unsigned short ActorPool::FindIDFromGtaPtr(int gtaId) {
    int i = 0;
    if (lastActive >= 0) {
        do {
            if (gtaId == gtaIds[i]) return (unsigned short)i;
            ++i;
        } while (i <= lastActive);
    }
    return 0xffff;
}

void ActorPool::DeleteAll() {
    int i = 0;
    do {
        Delete((unsigned short)i);
        ++i;
    } while ((unsigned short)i < 1000);
}
