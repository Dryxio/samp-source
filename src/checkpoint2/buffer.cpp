#include "buffer.h"
void Buffer::Clear() { ClearStorage(false); }
Buffer::~Buffer() { Clear(); }
