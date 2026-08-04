# Architecture — libhalalium.so

```mermaid
flowchart TB
  subgraph inject [Inject]
    kitty[AndKittyInjector --memfd]
    jni[JNI_OnLoad 0x1d69e4]
    th[pthread start 0x1d6afc]
  end

  subgraph install [egl_install 0x1d84cc]
    egl[DobbyHook eglSwapBuffers]
    inp[DobbyHook InputConsumer::consume]
    upd[DobbyHook PlayerController.Update RVA 0x8E7C40C]
    late[DobbyHook LateUpdate Update+0xB44]
    extra[DobbyHook secondary/tertiary/extra A/B]
    track[track hooks vector 0x2796d8]
  end

  subgraph frame [Per frame]
    eglcb[egl_callback 0x1d76f0]
    imgui[ImGui 1.92.7 + OpenGL3]
    wm[##watermark + ##wm_click]
    menu[Rage / Visuals / Skins / Settings]
  end

  subgraph tick [Per player tick]
    hooks[Halalium_Hooks_Update 0x1d7a0c]
    local[local @0x2795b0]
    esp[ESP / aim / chams gates]
  end

  subgraph bypass [Bypass thread]
    bp[Halalium_Bypass 0x1d90b8]
    dest[DobbyDestroy all]
    orig[orig OnStart]
    rehook[DobbyHook all again]
  end

  kitty --> jni --> th --> install
  egl --> eglcb --> imgui --> wm --> menu
  upd --> hooks --> local --> esp
  bp --> dest --> orig --> rehook
  track -.-> bp
```

## Data flow (Update)

```text
PlayerController* p
    ├─ photon = *(p + 0x160)
    │     └─ IsLocal = *(uint8_t*)(photon + 0x30)
    ├─ team   = *(uint8_t*)(p + 0x79)
    ├─ weaponry = *(p + 0x88)
    │     └─ weapon = *(weaponry + 0xA0)
    ├─ occlusion = *(p + 0xB8)
    ├─ mainCamera = *(p + 0xE8)
    └─ visible flag @ (p + 0xD8)   // forced for non-local in hook
```

## TypeInfo resolve (0.39.2)

```text
klass   = *(libil2cpp + TypeInfo_RVA)
statics = *(klass + 0x90)
instance= *(statics + 0x10)   // PlayerManager etc.
```
