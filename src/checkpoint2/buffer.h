// Minimal layouts inferred from R5. Names describe this reconstruction,
// not recovered original source identifiers.
class Storage {
public:
    void* data;
    unsigned count,capacity;
    ~Storage();
    void ClearStorage(bool keep);
};
class Buffer : public Storage {
public:
    ~Buffer();
    void Clear();
};
class Layer : public Buffer {
public:
    char gap[8];
    bool active;
    ~Layer();
    void Release();
};
