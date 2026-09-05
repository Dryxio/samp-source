#include "buffer.h"
void Layer::Release() { active = false; Clear(); }
Layer::~Layer() { Release(); }
