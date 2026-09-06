# Larger independent UI/render lot — base source, ready probe

Seven real CNewPlayerTags functions total1894 unique bytes: constructor6CCF0/38,
dtor6CD20/41, End6CD50/33, DeleteDeviceObjects6D650/38,
RestoreDeviceObjects6D680/42, Begin6D6B0/246, Draw6D0A0/1456.
All unaccepted against latest checked reliability-preview baseline238956.
Do not confuse following6D7B0/73 with End; it belongs to another owner's lifetime.
NewPlayerTags End is6CD50, actual SpriteEnd +StateBlockApply.

closure_player_tag_health_proposed.cpp copies genuine base implementation into
new TU with three external arrays. No algorithm changes before first probe.
Actual class size12: device0,StateBlock4,Sprite8, already declared in existing
newplayertags.h. Header unchanged. Constructor/lifetime use true D3DXSprite COM
and source SAFE_DELETE(sprite) behavior (ordinary delete, not guessed Release).

Three source vertex arrays80 each verified entirely against PE:
HealthBarBDRVertices1 RVA102890, HealthBarBGVertices11028E0,
HealthBarVertices1102930. All four vertices preserve x,y,z,rhw,color initial
values; source payload equality checked before compiling. Separate minimal
storeTUs prevent main.h/RakNet sentinel pollution. No data coverage credited ascode.

Matrices genuine BSS64: matView140D70, matProj140DB0. Original D3DXVec3Project
argument positions establish them and real SetTransform hook74880/78 confirms
writes for enumVIEW2/PROJECTION3; WORLD256 writes third matrix140DF0/64.
Optional independently useful closure_hook_transform_proposed.cpp copies this
actual base hook method and third matrix store, +78unique, establishing the real
producer in this lot. Requires no new fake vtable/instance; existing full COM
interface declaration is used. The full IDirect3DDevice9Hook vtable is not claimed.

New actual D3DXVec3Project import thunkC5C80/6→IAT E5514 must be verified from
pinned import library and original import name/module. No proprietary routine
body is substituted; dynamic import provider, matching established D3DX pattern.
Existing D3DXCreateSprite thunkC5C74 and CRT __ftol2/operator-delete providers
already accepted. No other direct-function dependency missing for these7 bodies.
COM sites: deviceGetViewport C0, SetRenderStateE4, SetTextureStageState10C,
SetVertexShader170, SetPixelShader1AC, SetFVF164, DrawPrimitiveUP14C;
stateblockCapture10/Apply14 andSpriteBegin20/End2C/Lost30/Reset34.

Direct025 credit0. Its older name+bar renderer and different initialization are
less faithful than current dashr base for these exact R5 functions. The larger
name-label755 method is excluded; it needs separate R5 adaptation. Newcalls do
not reach parked Object694/Sprite272. No EH or jump tables expected in this lot.

Principal health1894 +matrixproducer78 =1972 source bytes. The six-byte
D3DXVec3Project thunk is an imported dependency and contributes zero source credit.
Provider/payload/range evidence: player-tag-health-manifest.json (health1894).
