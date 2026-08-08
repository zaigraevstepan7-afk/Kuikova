# Menu layout (ImGui ids from .rodata)

Halalium uses a **left column + two right panels** pattern per tab (`##_left`, `##_right_top`, `##_right_bottom`), plus a persistent watermark.

```text
┌─────────────────────────────────────────────┐
│ ##watermark  Lemming / t.me/lemminghack     │
│              [##wm_click hit target]        │
├──────────┬──────────────────────────────────┤
│ ##tab_bar│                                  │
│ Rage     │  ##rage_left  │ ##rage_right_*   │
│ Visuals  │  ##vis_left   │ ##vis_right_*    │
│ Misc     │  ##misc_left  │ ##misc_right_*   │
│ Settings │  ##settings_* │ watermark opts   │
│ Skins    │  ##skins_* / ##weapons_list      │
└──────────┴──────────────────────────────────┘
```

Open flag: byte `@0x279064` toggled by `##wm_click` (menu starts closed until watermark tap).
