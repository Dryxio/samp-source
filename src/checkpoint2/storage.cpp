#include <new>
#include "buffer.h"
Storage::~Storage() { if (capacity > 0) ::operator delete(data); }
void Storage::ClearStorage(bool keep) {
    if (capacity) {
        if (capacity > 512 && !keep) {
            ::operator delete(data);
            capacity = 0;
            data = 0;
        }
        count = 0;
    }
}
