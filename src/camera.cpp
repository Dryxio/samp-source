// External objects are resolved by the matching verifier, never inferred
// from bytes that happen to differ. Their backing values are checked in R5.
extern "C" {
extern float* cameraZoom;
extern float* aspectRatio;
extern float localZoom;
extern float localAspect;
extern float remoteZoom[210];
extern float remoteAspect[210];

void __stdcall StoreLocalCamera() {
    localZoom = *cameraZoom;
    localAspect = *aspectRatio;
}
void __stdcall RestoreLocalCamera() {
    *cameraZoom = localZoom;
    *aspectRatio = localAspect;
}
void __stdcall SetPlayerCamera(unsigned char id, float zoom, float aspect) {
    remoteZoom[id] = zoom;
    remoteAspect[id] = aspect;
}
float __stdcall GetLocalAspect() {
    return *aspectRatio - 1.0f;
}
float __stdcall GetLocalZoom() {
    return (*cameraZoom - 35.0f) / 35.0f;
}
void __stdcall SetRemoteCamera(unsigned char id) {
    float zoom = remoteZoom[id] * 35.0f;
    zoom += 35.0f;
    *cameraZoom = zoom;
    *aspectRatio = remoteAspect[id] + 1.0f;
}
}
