// Existing Microsoft strsafe implementations, emitted independently of IME MsgProc.
#include "d3d9/common/dxstdafx.h"
#include <strsafe.h>

// Compile-only address holders ensure the three real inline functions are emitted.
// These holders are scaffolding, not original R5 storage, and must not be rooted
// or credited. They have constant initialization and no CRT initializer.
extern HRESULT (__stdcall * const r5EmitStringLengthWorkerW)(const WCHAR*,size_t,size_t*) = &StringLengthWorkerW;
extern HRESULT (__stdcall * const r5EmitStringCatWorkerW)(WCHAR*,size_t,const WCHAR*) = &StringCatWorkerW;
extern HRESULT (__stdcall * const r5EmitStringCchCatW)(WCHAR*,size_t,const WCHAR*) = &StringCchCatW;
