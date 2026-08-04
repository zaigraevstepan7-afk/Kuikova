# libhalalium.so — полный статический реверс

**Бинарник:** `re/bin/libhalalium.so`  
**BuildID:** `12532fca99debbaa836dbbea6e5cceec95f5bbbb`  
**SONAME:** `libhalalium.so`  
**Arch:** ELF64 aarch64, dynamically linked, **stripped**  
**Размер:** 2 584 440 байт (~2.5 MiB)  
**Версия (строка в .rodata):** `t.me/lemminghack, 0.39.2`  
**Цель:** Android / Unity IL2CPP — пакет `com.axlebolt.standoff2` (Standoff 2), билд логики под **0.39.2**

---

## 1. Что это такое (вердикт)

Это **инжектируемый native-чит (internal)** для Standoff 2:

| Слой | Технология |
|------|------------|
| Инъекция | `AndKittyInjector` + `inj.sh` (`--memfd`), вход через `JNI_OnLoad` |
| Хуки | **Dobby** (`DobbyHook` / `DobbyDestroy` / `DobbySymbolResolver` / `DobbyCodePatch`) |
| Рендер/меню | **Dear ImGui 1.92.7** + `imgui_impl_opengl3`, хук `eglSwapBuffers` |
| Ввод | хук `android::InputConsumer::consume` из `libinput.so` |
| Игровые хуки | абсолютные RVA от базы `libunity`/`libil2cpp` (слот `@0x2794f0`) |
| Шейдеры chams | AssetBundle **`padla`** → `/sdcard/Android/data/com.axlebolt.standoff2/files/padla` (`fresnel` / `shador`) |
| Бренд | watermark **Lemming**, telegram `t.me/lemminghack` |

Полный исходник из одного `.so` **невосстановим 1:1** (stripped, ~148 581 insn, ~2400 функций-кандидатов, куча ImGui/libc++).  
Восстановлено: архитектура, точки входа, таблица хуков, глобалы, поля игрока, фичи UI, псевдокод ключевых функций, SDK оффсетов 0.39.2.

---

## 2. ELF / зависимости

**NEEDED:** `libandroid.so`, `liblog.so`, `libEGL.so`, `libGLESv3.so`, `libOpenSLES.so`, `libm.so`, `libdl.so`, `libc.so`

**FLAGS:** `BIND_NOW`

**Секции (важное):**

| Section | VA | Size |
|---------|-----|------|
| `.rodata` | `0x2cde0` | `0x18d09c` |
| `.text` | `0x1d6890` | `0x95a18` |
| `.plt` | `0x26c2b0` | `0x1340` |
| `.bss`/data globals page | `0x279000+` | — |

**Экспорты (не C++ ABI):**  
`JNI_OnLoad` `@0x1d69e4`, `DobbyHook` `@0x2389d0`, `DobbyDestroy`, `DobbyGetVersion`, `DobbyCodePatch`, `DobbySymbolResolver`

**Импорты (частота BL→PLT, топ):** `memcpy`, `abort`, `realloc`, `malloc`, GL API, `pthread_*`, `dlsym`, `fopen`/`fread`, `__android_log_print`, `eglGetCurrentContext`, `eglQuerySurface`, `AMotionEvent_getX/Y`, …

`.init_array` в файле нули — заполняется релокациями при загрузке.

---

## 3. Точка входа — `JNI_OnLoad` (`0x1d69e4`)

```text
JNI_OnLoad:
  operator new(8)                         ; PLT _Znwm
  __thread_struct ctor
  pthread_create(…, start_routine=0x1d6afc, arg=…)
  return JNI_VERSION_1_6  (0x00010006)
```

Поток стартует на `0x1d6afc` → дальше инициализация/поиск базы игры → `egl_install` и треды с именами **`Halalium_Hooks`** / **`Halalium_Bypass`** (строки в `.rodata`).

---

## 4. Установщик хуков — `egl_install` (`0x1d84cc`)

Аннотированный дизасм: [`disasm/egl_install_annotated.asm`](../disasm/egl_install_annotated.asm)

### 4.1 Символьные хуки (DobbySymbolResolver → DobbyHook)

| # | Библиотека / символ | Callback | Orig slot |
|---|---------------------|----------|-----------|
| 1 | `libEGL.so` / `eglSwapBuffers` | `0x1d76f0` | `0x279578` |
| 2 | `libinput.so` / `android::InputConsumer::consume` (Itanium mangled) | `0x1d760c` | `0x279580` |

### 4.2 Хуки по RVA от `game_base = *(0x2794f0)`

| # | RVA | Callback | Orig slot | Роль |
|---|-----|----------|-----------|------|
| 3 | `0x8E7C40C` | `0x1d7a0c` | `0x2795a8` | **PlayerController.Update** → тело `Halalium_Hooks` |
| 4 | `0x8E0085C` | `0x1d81fc` | `0x2795d8` | secondary |
| 5 | `0x79FE5E0` (fallback `0x147E970`) | `0x1d8404` | `0x279520` | tertiary |
| 6 | `Update+0xB44` = `0x8E7CF50` | `0x1d7ec4` | `0x2795f0` | **LateUpdate** |
| 7 | `0x8D663EC` | `0x1d82a0` | `0x279600` | extra A |
| 8 | `0x8D2B2B0` | `0x1d83cc` | `0x279610` | extra B |

После каждого game-хука вызывается трекер `0x1d917c` (кладёт `{target, cb, orig*}` в вектор `@0x2796d8` для Bypass).

В хвосте `egl_install` (`0x1d8708+`) грузятся **встроенные RIFF/WAV** (`0x37fd2`, `0x42266`, `0x52af8`) через хелпер `0x1eac8c` — отсюда зависимость `libOpenSLES.so` и лог `Sound %d loaded and player created`.

**Всего прямых `bl DobbyHook`:** 9 сайтов  
`0x1d8508, 0x1d8534, 0x1d8568, 0x1d85ac, 0x1d8618, 0x1d8658, 0x1d86b0, 0x1d86f4, 0x1d9140`

PLT-обёртки:

| PLT | Символ |
|-----|--------|
| `0x26c550` | `DobbySymbolResolver` |
| `0x26c560` | `DobbyHook` |
| `0x26c590` | `DobbyDestroy` |
| `0x26c540` | `__android_log_print` |

---

## 5. Ключевые callback’и

### 5.1 `egl_callback` (`0x1d76f0`) — кадр меню

1. Once-flag `@0x27965c`: `eglGetCurrentContext` → init ImGui / OpenGL3 backend.  
2. `eglQuerySurface` ширина/высота (`EGL_WIDTH=0x3057`, `EGL_HEIGHT=0x3056`).  
3. `NewFrame` → отрисовка watermark / меню → `Render` → вызов **оригинала** `eglSwapBuffers` через слот `0x279578`.

Меню открывается **не** Insert/RightAlt как основной путь, а кликом по невидимой кнопке **`##wm_click`** на окне **`##watermark`** (флаг `@0x279064`).

ImGui id в бинаре (раскладка меню):

| Tab | Child regions |
|-----|----------------|
| Rage | `##rage_left`, `##rage_right_top`, `##rage_right_bottom` |
| Visuals | `##vis_left`, `##vis_right_top`, `##vis_right_bottom` |
| Misc | `##misc_left`, `##misc_right_top`, `##misc_right_bottom` |
| Settings | `##settings_left`, `##settings_watermark` |
| Skins | `##skins_panel`, `##skins_list`, `##weapons_list` |
| Chrome | `##tab_bar`, `##tabbtn`, `##watermark`, `##wm_click` |

Полный список: [`extracted/imgui_ids.json`](../extracted/imgui_ids.json), таблица строк: [`docs/FEATURE_MAP.md`](FEATURE_MAP.md).

### 5.2 `input_callback` (`0x1d760c`)

Прокидывает touch в ImGui (`AInputEvent_getType` / `AMotionEvent_getAction` / `getX` / `getY`), затем оригинал `InputConsumer::consume`.

### 5.3 `Halalium_Hooks_Update` (`0x1d7a0c`)

```c
// x0 = PlayerController*
void Halalium_Hooks_Update(PlayerController* player) {
    orig_Update(player);                    // blr *[0x2795a8]
    PhotonPlayer* ph = player->photon;      // ldr [x19,#0x160]
    if (!ph) return;
    // feature tick helpers via config @0x2794f8
    if (ph->IsLocal) {                      // ldrb [ph,#0x30]  ← dump: PhotonPlayer.IsLocal
        g_local = player;                   // str @0x2795b0
        // rate-limited work (~250 units via steady_clock::now)
        // ...
    } else {
        player->visible = 1;                // strb #1,[player,#0xd8]
        // team compare player+0x79 vs local+0x79
        // ESP/aim path if enemy
    }
}
```

**Подтверждённые поля PlayerController (LDR в коде = dump.cs):**

| Offset | Смысл |
|--------|--------|
| `+0x79` | team (byte) |
| `+0x88` | WeaponryController* |
| `+0xB8` | Occlusion controller |
| `+0xD8` | visibility bool (чит форсит) |
| `+0xE8` | PlayerMainCamera |
| `+0x160` | PhotonPlayer* |

`PhotonPlayer.IsLocal` = `+0x30` (dump TypeDefIndex 20130).

### 5.4 `Halalium_Bypass` (`0x1d90b8`)

```c
void Halalium_Bypass(void* self) {
    __android_log_print(ANDROID_LOG_INFO, "Halalium_Bypass", ...);
    for (hook : tracked_hooks @0x2796d8)   // step 0x18
        DobbyDestroy(hook.target);
    ret = orig_OnStart(self);              // blr *[0x2796f0]
    for (hook : tracked_hooks)
        DobbyHook(...);                    // log "bypas hok result %d"
    return ret;
}
```

Смысл: временно снять хуки на чувствительном участке античита, вызвать оригинал, повесить обратно.  
(Детали обхода античита здесь намеренно не расширяются — зафиксирован только механизм.)

---

## 6. Фичи (строки UI в `.rodata`)

Полный список из бинаря:

- Enable Esp, Silent Aim, Rage, Anti Aim, Anti Aim Pitch  
- No spread, Auto Fire, Auto Wall, Through Walls, Wallshot, Inf Ammo  
- Skin Changer (`Skin Changer: Swapped to weapon %d (skin %d)`), скины вроде `AKR_MirageMenace`, `MAC10_WildRage`, …  
- Chams / Local Chams / Enemy Chams (+ `fresnel`/`shador`/`padla`)  
- Spin, Fov Check, Fov Color, scope fov, Health Bar, Bone, Box, Third Person, Watermark  

Артефакт: [`extracted/profile.json`](../extracted/profile.json), [`strings/interesting.txt`](../strings/interesting.txt).

---

## 7. Глобалы страницы `0x279xxx` (восстановленные)

| VA | Назначение |
|----|------------|
| `0x279064` | `menu_open` (byte), toggle `##wm_click` |
| `0x2794f0` | `game_base` (база для RVA-хуков) |
| `0x2794f8` | указатель конфига/фич |
| `0x279520` | orig tertiary |
| `0x279578` | orig `eglSwapBuffers` |
| `0x279580` | orig `InputConsumer::consume` |
| `0x2795a8` | orig `PlayerController.Update` |
| `0x2795b0` | local `PlayerController*` |
| `0x2795b8` | last tick (rate limit) |
| `0x2795d8` | orig secondary |
| `0x2795f0` | orig LateUpdate |
| `0x279600` / `0x279610` | orig extra A/B |
| `0x27965c` | ImGui once-init flag |
| `0x2796d8` | vector begin tracked hooks |
| `0x2796f0` | orig OnStart (bypass) |

---

## 8. SDK оффсетов 0.39.2

См. [`sdk/Offsets_0.39.2.h`](../sdk/Offsets_0.39.2.h).

Критично:

```text
Il2CppClass.static_fields = 0x90   // НЕ 0x60
PlayerManager TypeInfo     = 0xAC5E190
local player               = PlayerManager + 0x70
players list               = PlayerManager + 0x28
```

Корреляция с `okaakka/dump (1).cs` + `script.json` (лежит в архиве `okaakka.txt` на `main`).

---

## 9. Как запускается (из комплекта)

```text
padla          -> /sdcard/Android/data/com.axlebolt.standoff2/files/padla
libhalalium.so -> /data/local/tmp/  (часто через memfd)
inj.sh:
  AndKittyInjector --package com.axlebolt.standoff2 --libs libhalalium.so --memfd --delay 2000000
```

---

## 10. Статистика RE

| Метрика | Значение |
|---------|----------|
| Инструкций в `.text` | 148 581 |
| Кандидатов функций | ~2 408 |
| DobbyHook call sites | 9 |
| Feature-строк UI | 25+ |
| ImGui `##` ids | 9+ |
| Аннотированных функций | JNI / egl_install / egl_cb / input / Update / LateUpdate / Bypass / Dobby* |
| String xrefs (ключевые) | см. `extracted/string_xrefs_fixed.json` |

---

## 11. Артефакты в репозитории

```text
re/
  bin/libhalalium.so
  docs/FULL_RE.md              ← этот файл
  docs/ARCHITECTURE.md
  recovered/halalium_skeleton.cpp
  sdk/Offsets_0.39.2.h
  disasm/*_annotated.asm
  decompile/*.md
  extracted/*.json
  tools/deep_re.py
  tools/fix_xrefs.py
```

---

## 12. Чего физически нельзя вытащить дальше статикой

1. **Полный исход ImGui-меню** (~сотни KB скомпилированного Dear ImGui) — бессмысленно декомпилировать целиком.  
2. **Точные тела Silent Aim / Wallshot / Skin Instantiate** без динамического трейса (флаги меню → разрозненные ветки, нет символов).  
3. **Актуальные RVA после патча игры** — привязаны к **0.39.2**; новый `dump.cs`/`script.json` обязателен.  
4. **Расшифровка/обфускация имён методов в dump** (Axlebolt) — многие `RVA: -1`.  
5. **`.init_array` contents** — только после релокаций в памяти.

Дальше имеет смысл только: Frida/gdb на девайсе, или пересборка своего инжекта по `recovered/halalium_skeleton.cpp` + `Offsets_0.39.2.h` (как сделано в ветках Kikaium/Melodium).

---

## 13. Итог одной фразой

**`libhalalium.so` — stripped aarch64 internal для Standoff 2 0.39.2 (Lemming/Halalium): Dobby-хуки EGL+Input+Player Update/LateUpdate, ImGui-меню с watermark-toggle, ESP/aim/chams/skins, bypass-поток, шейдеры из padla.**
