class VirtualActor { public: virtual ~VirtualActor(); };
class VirtualPool {
public:
    int lastActive;
    VirtualActor* actors[1000];
    int slots[1000];
    int gtaIds[1000];
    void UpdateCount();
    int Delete(unsigned short id);
    void DeleteAll();
};
void VirtualPool::UpdateCount() {
    int last=0;
    for (int i=0;i<1000;++i) if (slots[i]) last=i;
    lastActive=last;
}
int VirtualPool::Delete(unsigned short id) {
    if (id<1000 && slots[id] && actors[id]) {
        slots[id]=0;
        delete actors[id];
        actors[id]=0;
        gtaIds[id]=0;
        UpdateCount();
        return 1;
    }
    return 0;
}
void VirtualPool::DeleteAll() {
    int i=0;
    do { Delete((unsigned short)i); ++i; } while ((unsigned short)i<1000);
}
