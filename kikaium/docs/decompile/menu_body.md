# menu_body

- VA: `0x1db874`
- Size scanned: `0x1600`
- Pass A instructions: 1408
- Method: **2-command decompile** — (1) Capstone Pass A full disasm (2) Capstone Pass B field/RVA/string recovery

## Pass B — field LDR/STR map

| Offset | Meaning | Count |
|---|---|---|
| `0xc` | unknown | 4 |
| `0x10` | unknown | 22 |
| `0x14` | unknown | 14 |
| `0x18` | unknown | 3 |
| `0x28` | players_list / camera holder | 5 |
| `0x2c` | unknown | 1 |
| `0x30` | Photon.isLocal (ldrb) / nested | 2 |
| `0x60` | unknown | 16 |
| `0x64` | unknown | 2 |
| `0x68` | unknown | 1 |
| `0x70` | unknown | 4 |
| `0xce` | unknown | 11 |
| `0xe0` | unknown | 1 |
| `0x2c8` | unknown | 11 |
| `0x2d8` | unknown | 11 |

## Pass B — game RVAs (mov+movk)

_none_

## Pass B — strings (ADRP+ADD resolved only)

- `##settings_left`
- `Accent Color`
- `##settings_watermark`
- `Watermark`
- `##vis_left`
- `Enable Esp`
- `Box`
- `Box Type`
- `Box Color`
- `Corner Size`
- `Health Bar`
- `Distance`
- `Distance Color`
- `##vis_right_top`
- `Chams`
- `Enemy Chams`
- `Enemy Color`
- `Through Walls`
- `##misc_left`
- `Third Person`
- `##misc_right_top`
- `Inf Ammo`
- `Fire Rate`
- `Wallshot`
- `##misc_right_bottom`
- `##skins_panel`
- `Skin Changer`
- `##weapons_list`
- `##skins_list`
- `##rage_left`
- `Silent Aim`
- `Bone`
- `Fov Check`
- `Fov`
- `Fov Color`
- `Auto Fire`
- `Auto Wall`
- `No spread`
- `##rage_right_top`
- `Anti Aim`
- `Local Chams`
- `Local Color`
- `##vis_right_bottom`
- `World`
- `World Color`
- `Solid World Color`
- `Apply World Color`
- `Spin Speed`
- `Reverse Spin`
- `##rage_right_bottom`
- `Anti Aim Pitch`
- `Enable Anti Aim first`

## Pass B — calls (first 40)

- `0x1db8b0` → `0x1dcd64` (bl)
- `0x1db8b4` → `0x1fc534` (bl)
- `0x1db8bc` → `0x1fc534` (bl)
- `0x1db920` → `0x1db6e8` (bl)
- `0x1db924` → `0x217e14` (bl)
- `0x1db94c` → `0x1ff434` (bl)
- `0x1db974` → `0x1edacc` (bl)
- `0x1db9a0` → `0x209124` (bl)
- `0x1db9b8` → `0x217e84` (bl)
- `0x1db9bc` → `0x217e14` (bl)
- `0x1db9cc` → `0x1dda38` (bl)
- `0x1db9d0` → `0x217e14` (bl)
- `0x1db9e8` → `0x21512c` (bl)
- `0x1db9f8` → `0x21512c` (bl)
- `0x1db9fc` → `0x1db7e8` (bl)
- `0x1dba08` → `0x200d64` (bl)
- `0x1dba1c` → `0x1db6e8` (bl)
- `0x1dba20` → `0x217e14` (bl)
- `0x1dba38` → `0x1ff434` (bl)
- `0x1dba58` → `0x1edacc` (bl)
- `0x1dba84` → `0x209124` (bl)
- `0x1dba98` → `0x217e84` (bl)
- `0x1dba9c` → `0x217e14` (bl)
- `0x1dbab4` → `0x1dd198` (bl)
- `0x1dbad0` → `0x1dd198` (bl)
- `0x1dbae4` → `0x1dd198` (bl)
- `0x1dbaf8` → `0x1dd198` (bl)
- `0x1dbb0c` → `0x1dd198` (bl)
- `0x1dbb30` → `0x1db6e8` (bl)
- `0x1dbb34` → `0x217e14` (bl)
- `0x1dbb5c` → `0x1ff434` (bl)
- `0x1dbb80` → `0x1edacc` (bl)
- `0x1dbbac` → `0x209124` (bl)
- `0x1dbbc4` → `0x217e84` (bl)
- `0x1dbbc8` → `0x217e14` (bl)
- `0x1dbbdc` → `0x1dd198` (bl)
- `0x1dbbf4` → `0x1dd198` (bl)
- `0x1dbc20` → `0x1dd3cc` (bl)
- `0x1dbc34` → `0x1dda38` (bl)
- `0x1dbc50` → `0x1dd6e0` (bl)

## Pass A — full disasm

```
001db874: stp      x29, x30, [sp, #0x60]
001db878: stp      x28, x27, [sp, #0x70]
001db87c: stp      x26, x25, [sp, #0x80]
001db880: stp      x24, x23, [sp, #0x90]
001db884: stp      x22, x21, [sp, #0xa0]
001db888: stp      x20, x19, [sp, #0xb0]
001db88c: add      x29, sp, #0x60
001db890: mrs      x22, tpidr_el0
001db894: ldr      x8, [x22, #0x28]
001db898: str      x8, [sp, #0x28]
001db89c: adrp     x8, #0x279000
001db8a0: ldrb     w8, [x8, #0x64]
001db8a4: cbz      w8, #0x1dcd24
001db8a8: adrp     x0, #0x2e000
001db8ac: add      x0, x0, #0x15
001db8b0: bl       #0x1dcd64
001db8b4: bl       #0x1fc534
001db8b8: fmov     s12, s0
001db8bc: bl       #0x1fc534
001db8c0: adrp     x23, #0x279000
001db8c4: fmov     s0, #8.00000000
001db8c8: fmov     s11, s1
001db8cc: ldr      s1, [x23, #0x60]
001db8d0: fmov     s2, #0.50000000
001db8d4: adrp     x8, #0x279000
001db8d8: ldr      w8, [x8, #0x68]
001db8dc: fmul     s10, s1, s0
001db8e0: cmp      w8, #1
001db8e4: fsub     s0, s12, s10
001db8e8: fsub     s1, s11, s10
001db8ec: fmul     s8, s0, s2
001db8f0: fmul     s9, s1, s2
001db8f4: b.le     #0x1dbb14
001db8f8: cmp      w8, #2
001db8fc: b.eq     #0x1dbfd0
001db900: cmp      w8, #3
001db904: b.eq     #0x1dc2a4
001db908: cmp      w8, #4
001db90c: b.ne     #0x1dcd20
001db910: fmov     s0, s8
001db914: fmov     s1, s11
001db918: adrp     x0, #0x31000
001db91c: add      x0, x0, #0x279
001db920: bl       #0x1db6e8
001db924: bl       #0x217e14
001db928: adrp     x24, #0x277000
001db92c: adrp     x25, #0x279000
001db930: mov      w26, #1
001db934: ldr      x24, [x24, #0x4a0]
001db938: ldr      x19, [x25, #0x7e8]
001db93c: ldr      x8, [x24]
001db940: ldr      x28, [x8, #0x1480]
001db944: strb     w26, [x28, #0xce]
001db948: cbnz     x19, #0x1db954
001db94c: bl       #0x1ff434
001db950: mov      x19, x0
001db954: adrp     x27, #0x279000
001db958: ldr      s1, [x23, #0x60]
001db95c: nop      
001db960: adr      x21, #0x279050
001db964: ldr      s0, [x27, #0x2d8]
001db968: ldr      x20, [x28, #0x2c8]
001db96c: mov      x0, x21
001db970: fmul     s9, s0, s1
001db974: bl       #0x1edacc
001db978: movi     d1, #0000000000000000
001db97c: fmov     s0, s9
001db980: mov      w3, w0
001db984: adrp     x4, #0x33000
001db988: add      x4, x4, #0x9fb
001db98c: add      x2, x28, #0x118
001db990: mov      x0, x20
001db994: mov      x1, x19
001db998: mov      x5, xzr
001db99c: mov      x6, xzr
001db9a0: bl       #0x209124
001db9a4: fmov     s12, #4.00000000
001db9a8: add      x0, sp, #0x10
001db9ac: str      wzr, [sp, #0x10]
001db9b0: fadd     s0, s9, s12
001db9b4: str      s0, [sp, #0x14]
001db9b8: bl       #0x217e84
001db9bc: bl       #0x217e14
001db9c0: adrp     x0, #0x2d000
001db9c4: add      x0, x0, #0x6a4
001db9c8: mov      x1, x21
001db9cc: bl       #0x1dda38
001db9d0: bl       #0x217e14
001db9d4: nop      
001db9d8: adr      x19, #0x279168
001db9dc: adrp     x1, #0x2d000
001db9e0: add      x1, x1, #0x17f
001db9e4: mov      x0, x19
001db9e8: bl       #0x21512c
001db9ec: adrp     x1, #0x36000
001db9f0: add      x1, x1, #0x646
001db9f4: mov      x0, x19
001db9f8: bl       #0x21512c
001db9fc: bl       #0x1db7e8
001dba00: movi     d0, #0000000000000000
001dba04: fmov     s1, s10
001dba08: bl       #0x200d64
001dba0c: fmov     s0, s8
001dba10: fmov     s1, s11
001dba14: adrp     x0, #0x33000
001dba18: add      x0, x0, #0x443
001dba1c: bl       #0x1db6e8
001dba20: bl       #0x217e14
001dba24: ldr      x8, [x24]
001dba28: ldr      x19, [x25, #0x7e8]
001dba2c: ldr      x21, [x8, #0x1480]
001dba30: strb     w26, [x21, #0xce]
001dba34: cbnz     x19, #0x1dba40
001dba38: bl       #0x1ff434
001dba3c: mov      x19, x0
001dba40: ldr      s0, [x27, #0x2d8]
001dba44: ldr      s1, [x23, #0x60]
001dba48: nop      
001dba4c: adr      x0, #0x279050
001dba50: ldr      x20, [x21, #0x2c8]
001dba54: fmul     s8, s0, s1
001dba58: bl       #0x1edacc
001dba5c: movi     d1, #0000000000000000
001dba60: fmov     s0, s8
001dba64: mov      w3, w0
001dba68: adrp     x4, #0x2f000
001dba6c: add      x4, x4, #0xc92
001dba70: add      x2, x21, #0x118
001dba74: mov      x0, x20
001dba78: mov      x1, x19
001dba7c: mov      x5, xzr
001dba80: mov      x6, xzr
001dba84: bl       #0x209124
001dba88: fadd     s0, s8, s12
001dba8c: add      x0, sp, #0x10
001dba90: str      wzr, [sp, #0x10]
001dba94: str      s0, [sp, #0x14]
001dba98: bl       #0x217e84
001dba9c: bl       #0x217e14
001dbaa0: nop      
001dbaa4: adr      x19, #0x27904c
001dbaa8: adrp     x0, #0x2f000
001dbaac: add      x0, x0, #0x526
001dbab0: mov      x1, x19
001dbab4: bl       #0x1dd198
001dbab8: ldrb     w8, [x19]
001dbabc: cbz      w8, #0x1dc528
001dbac0: adrp     x0, #0x31000
001dbac4: add      x0, x0, #0x951
001dbac8: nop      
001dbacc: adr      x1, #0x2792d0
001dbad0: bl       #0x1dd198
001dbad4: adrp     x0, #0x2e000
001dbad8: add      x0, x0, #0x49
001dbadc: nop      
001dbae0: adr      x1, #0x2792d1
001dbae4: bl       #0x1dd198
001dbae8: adrp     x0, #0x2e000
001dbaec: add      x0, x0, #0x4d
001dbaf0: nop      
001dbaf4: adr      x1, #0x2792d2
001dbaf8: bl       #0x1dd198
001dbafc: adrp     x0, #0x30000
001dbb00: add      x0, x0, #0x716
001dbb04: nop      
001dbb08: adr      x1, #0x2792d3
001dbb0c: bl       #0x1dd198
001dbb10: b        #0x1dc528
001dbb14: cbz      w8, #0x1dc530
001dbb18: cmp      w8, #1
001dbb1c: b.ne     #0x1dcd20
001dbb20: fmov     s0, s8
001dbb24: fmov     s1, s11
001dbb28: adrp     x0, #0x2d000
001dbb2c: add      x0, x0, #0x156
001dbb30: bl       #0x1db6e8
001dbb34: bl       #0x217e14
001dbb38: adrp     x21, #0x277000
001dbb3c: adrp     x24, #0x279000
001dbb40: ldr      x21, [x21, #0x4a0]
001dbb44: ldr      x19, [x24, #0x7e8]
001dbb48: ldr      x8, [x21]
001dbb4c: ldr      x26, [x8, #0x1480]
001dbb50: mov      w8, #1
001dbb54: strb     w8, [x26, #0xce]
001dbb58: cbnz     x19, #0x1dbb64
001dbb5c: bl       #0x1ff434
001dbb60: mov      x19, x0
001dbb64: adrp     x25, #0x279000
001dbb68: ldr      s1, [x23, #0x60]
001dbb6c: ldr      x20, [x26, #0x2c8]
001dbb70: ldr      s0, [x25, #0x2d8]
001dbb74: nop      
001dbb78: adr      x0, #0x279050
001dbb7c: fmul     s11, s0, s1
001dbb80: bl       #0x1edacc
001dbb84: movi     d1, #0000000000000000
001dbb88: fmov     s0, s11
001dbb8c: mov      w3, w0
001dbb90: adrp     x4, #0x34000
001dbb94: add      x4, x4, #0xad0
001dbb98: add      x2, x26, #0x118
001dbb9c: mov      x0, x20
001dbba0: mov      x1, x19
001dbba4: mov      x5, xzr
001dbba8: mov      x6, xzr
001dbbac: bl       #0x209124
001dbbb0: fmov     s0, #4.00000000
001dbbb4: add      x0, sp, #0x10
001dbbb8: str      wzr, [sp, #0x10]
001dbbbc: fadd     s0, s11, s0
001dbbc0: str      s0, [sp, #0x14]
001dbbc4: bl       #0x217e84
001dbbc8: bl       #0x217e14
001dbbcc: adrp     x0, #0x36000
001dbbd0: add      x0, x0, #0x611
001dbbd4: nop      
001dbbd8: adr      x1, #0x2795a0
001dbbdc: bl       #0x1dd198
001dbbe0: nop      
001dbbe4: adr      x19, #0x279790
001dbbe8: adrp     x0, #0x37000
001dbbec: add      x0, x0, #0x62c
001dbbf0: mov      x1, x19
001dbbf4: bl       #0x1dd198
001dbbf8: ldrb     w8, [x19]
001dbbfc: cbz      w8, #0x1dbcd0
001dbc00: nop      
001dbc04: adr      x19, #0x279178
001dbc08: adrp     x0, #0x37000
001dbc0c: add      x0, x0, #0xc7
001dbc10: nop      
001dbc14: adr      x2, #0x279180
001dbc18: mov      x1, x19
001dbc1c: mov      w3, #3
001dbc20: bl       #0x1dd3cc
001dbc24: adrp     x0, #0x2e000
001dbc28: add      x0, x0, #0x33
001dbc2c: nop      
001dbc30: adr      x1, #0x279198
001dbc34: bl       #0x1dda38
001dbc38: fmov     s0, #0.50000000
001dbc3c: fmov     s1, #5.00000000
001dbc40: adrp     x0, #0x32000
001dbc44: add      x0, x0, #0x91d
001dbc48: nop      
001dbc4c: adr      x1, #0x2791a8
001dbc50: bl       #0x1dd6e0
001dbc54: ldr      w8, [x19]
001dbc58: cmp      w8, #1
001dbc5c: b.ne     #0x1dbc80
001dbc60: adrp     x8, #0x37000
001dbc64: fmov     s1, #0.50000000
001dbc68: adrp     x0, #0x2d000
001dbc6c: add      x0, x0, #0xbd9
001dbc70: ldr      s0, [x8, #0xedc]
001dbc74: nop      
001dbc78: adr      x1, #0x2791ac
001dbc7c: bl       #0x1dd6e0
001dbc80: adrp     x0, #0x34000
001dbc84: add      x0, x0, #0xfb8
001dbc88: nop      
001dbc8c: adr      x1, #0x2791b0
001dbc90: bl       #0x1dd198
001dbc94: nop      
001dbc98: adr      x19, #0x279791
001dbc9c: adrp     x0, #0x35000
001dbca0: add      x0, x0, #0xa1b
001dbca4: mov      x1, x19
001dbca8: bl       #0x1dd198
001dbcac: ldrb     w8, [x19]
001dbcb0: cbz      w8, #0x1dbcd0
001dbcb4: movi     d0, #0000000000000000
001dbcb8: fmov     s1, #1.00000000
001dbcbc: adrp     x0, #0x32000
001dbcc0: add      x0, x0, #0x3dd
001dbcc4: nop      
001dbcc8: adr      x1, #0x2791b4
001dbccc: bl       #0x1dd6e0
001dbcd0: nop      
001dbcd4: adr      x19, #0x279792
001dbcd8: adrp     x0, #0x37000
001dbcdc: add      x0, x0, #0xc2e
001dbce0: mov      x1, x19
001dbce4: bl       #0x1dd198
001dbce8: ldrb     w8, [x19]
001dbcec: cbz      w8, #0x1dbd2c
001dbcf0: adrp     x0, #0x31000
001dbcf4: add      x0, x0, #0x272
001dbcf8: nop      
001dbcfc: adr      x1, #0x2791b8
001dbd00: bl       #0x1dda38
001dbd04: adrp     x0, #0x32000
001dbd08: add      x0, x0, #0x927
001dbd0c: nop      
001dbd10: adr      x1, #0x2791c8
001dbd14: bl       #0x1dda38
001dbd18: adrp     x0, #0x2f000
001dbd1c: add      x0, x0, #0xce
001dbd20: nop      
001dbd24: adr      x1, #0x279793
001dbd28: bl       #0x1dd198
001dbd2c: nop      
001dbd30: adr      x19, #0x279794
001dbd34: adrp     x0, #0x31000
001dbd38: add      x0, x0, #0x951
001dbd3c: mov      x1, x19
001dbd40: bl       #0x1dd198
001dbd44: ldrb     w8, [x19]
001dbd48: cbz      w8, #0x1dbd60
001dbd4c: adrp     x0, #0x30000
001dbd50: add      x0, x0, #0x6f6
001dbd54: nop      
001dbd58: adr      x1, #0x2791d8
001dbd5c: bl       #0x1dda38
001dbd60: nop      
001dbd64: adr      x19, #0x279795
001dbd68: adrp     x0, #0x30000
001dbd6c: add      x0, x0, #0xc09
001dbd70: mov      x1, x19
001dbd74: bl       #0x1dd198
001dbd78: ldrb     w8, [x19]
001dbd7c: cbz      w8, #0x1dbd94
001dbd80: adrp     x0, #0x37000
001dbd84: add      x0, x0, #0xd0
001dbd88: nop      
001dbd8c: adr      x1, #0x2791e8
001dbd90: bl       #0x1dda38
001dbd94: nop      
001dbd98: adr      x19, #0x279796
001dbd9c: adrp     x0, #0x30000
001dbda0: add      x0, x0, #0x701
001dbda4: mov      x1, x19
001dbda8: bl       #0x1dd198
001dbdac: ldrb     w8, [x19]
001dbdb0: cbz      w8, #0x1dbe18
001dbdb4: adrp     x0, #0x31000
001dbdb8: add      x0, x0, #0xde3
001dbdbc: nop      
001dbdc0: adr      x1, #0x279798
001dbdc4: nop      
001dbdc8: adr      x2, #0x2791f8
001dbdcc: mov      w3, #5
001dbdd0: bl       #0x1dd3cc
001dbdd4: adrp     x0, #0x35000
001dbdd8: add      x0, x0, #0x4f3
001dbddc: nop      
001dbde0: adr      x1, #0x279220
001dbde4: bl       #0x1dda38
001dbde8: fmov     s0, #0.50000000
001dbdec: fmov     s1, #5.00000000
001dbdf0: adrp     x0, #0x2d000
001dbdf4: add      x0, x0, #0x161
001dbdf8: nop      
001dbdfc: adr      x1, #0x279230
001dbe00: bl       #0x1dd6e0
001dbe04: adrp     x0, #0x31000
001dbe08: add      x0, x0, #0xdef
001dbe0c: nop      
001dbe10: adr      x1, #0x279234
001dbe14: bl       #0x1dd198
001dbe18: nop      
001dbe1c: adr      x19, #0x27979c
001dbe20: adrp     x0, #0x2f000
001dbe24: add      x0, x0, #0x514
001dbe28: mov      x1, x19
001dbe2c: bl       #0x1dd198
001dbe30: ldrb     w8, [x19]
001dbe34: cbz      w8, #0x1dbe7c
001dbe38: adrp     x0, #0x2e000
001dbe3c: add      x0, x0, #0x5b1
001dbe40: nop      
001dbe44: adr      x1, #0x279238
001dbe48: bl       #0x1dda38
001dbe4c: fmov     s0, #0.50000000
001dbe50: fmov     s1, #4.00000000
001dbe54: adrp     x0, #0x2f000
001dbe58: add      x0, x0, #0xc83
001dbe5c: nop      
001dbe60: adr      x1, #0x279248
001dbe64: bl       #0x1dd6e0
001dbe68: adrp     x0, #0x2d000
001dbe6c: add      x0, x0, #0x680
001dbe70: nop      
001dbe74: adr      x1, #0x27979d
001dbe78: bl       #0x1dd198
001dbe7c: bl       #0x1db7e8
001dbe80: movi     d0, #0000000000000000
001dbe84: fmov     s1, s10
001dbe88: bl       #0x200d64
001dbe8c: bl       #0x2011c8
001dbe90: fmov     s0, s8
001dbe94: fmov     s1, s9
001dbe98: adrp     x0, #0x37000
001dbe9c: add      x0, x0, #0x630
001dbea0: bl       #0x1db6e8
001dbea4: bl       #0x217e14
001dbea8: ldr      x8, [x21]
001dbeac: ldr      x19, [x24, #0x7e8]
001dbeb0: ldr      x26, [x8, #0x1480]
001dbeb4: mov      w8, #1
001dbeb8: strb     w8, [x26, #0xce]
001dbebc: cbnz     x19, #0x1dbec8
001dbec0: bl       #0x1ff434
001dbec4: mov      x19, x0
001dbec8: ldr      s0, [x25, #0x2d8]
001dbecc: ldr      s1, [x23, #0x60]
001dbed0: nop      
001dbed4: adr      x0, #0x279050
001dbed8: ldr      x20, [x26, #0x2c8]
001dbedc: fmul     s10, s0, s1
001dbee0: bl       #0x1edacc
001dbee4: movi     d1, #0000000000000000
001dbee8: fmov     s0, s10
001dbeec: mov      w3, w0
001dbef0: adrp     x4, #0x31000
001dbef4: add      x4, x4, #0x956
001dbef8: add      x2, x26, #0x118
001dbefc: mov      x0, x20
001dbf00: mov      x1, x19
001dbf04: mov      x5, xzr
001dbf08: mov      x6, xzr
001dbf0c: bl       #0x209124
001dbf10: fmov     s0, #4.00000000
001dbf14: add      x0, sp, #0x10
001dbf18: str      wzr, [sp, #0x10]
001dbf1c: fadd     s0, s10, s0
001dbf20: str      s0, [sp, #0x14]
001dbf24: bl       #0x217e84
001dbf28: bl       #0x217e14
001dbf2c: adrp     x0, #0x34000
001dbf30: add      x0, x0, #0xd7
001dbf34: nop      
001dbf38: adr      x1, #0x27979e
001dbf3c: bl       #0x1dd198
001dbf40: nop      
001dbf44: adr      x19, #0x2797a0
001dbf48: adrp     x0, #0x2d000
001dbf4c: add      x0, x0, #0x16e
001dbf50: nop      
001dbf54: adr      x2, #0x279250
001dbf58: mov      x1, x19
001dbf5c: mov      w3, #4
001dbf60: bl       #0x1dd3cc
001dbf64: adrp     x0, #0x35000
001dbf68: add      x0, x0, #0x500
001dbf6c: nop      
001dbf70: adr      x1, #0x279270
001dbf74: bl       #0x1dda38
001dbf78: adrp     x0, #0x34000
001dbf7c: add      x0, x0, #0xad4
001dbf80: nop      
001dbf84: adr      x1, #0x279280
001dbf88: bl       #0x1dd198
001dbf8c: ldr      w8, [x19]
001dbf90: adrp     x19, #0x279000
001dbf94: tst      w8, #0xfffffffd
001dbf98: b.ne     #0x1dc914
001dbf9c: adrp     x8, #0x279000
001dbfa0: ldr      x8, [x8, #0x510]
001dbfa4: ldr      w2, [x8, #0x2c]
001dbfa8: cmp      w2, #4
001dbfac: b.ne     #0x1dc8b4
001dbfb0: adrp     x8, #0x78000
001dbfb4: adrp     x1, #0x34000
001dbfb8: add      x1, x1, #0xae2
001dbfbc: ldr      q0, [x8, #0xe0]
001dbfc0: add      x0, sp, #0x10
001dbfc4: str      q0, [sp, #0x10]
001dbfc8: bl       #0x21512c
001dbfcc: b        #0x1dc8d0
001dbfd0: fmov     s0, s8
001dbfd4: fmov     s1, s11
001dbfd8: adrp     x0, #0x37000
001dbfdc: add      x0, x0, #0x652
001dbfe0: bl       #0x1db6e8
001dbfe4: bl       #0x217e14
001dbfe8: adrp     x21, #0x277000
001dbfec: adrp     x24, #0x279000
001dbff0: mov      w26, #1
001dbff4: ldr      x21, [x21, #0x4a0]
001dbff8: ldr      x19, [x24, #0x7e8]
001dbffc: ldr      x8, [x21]
001dc000: ldr      x27, [x8, #0x1480]
001dc004: strb     w26, [x27, #0xce]
001dc008: cbnz     x19, #0x1dc014
001dc00c: bl       #0x1ff434
001dc010: mov      x19, x0
001dc014: adrp     x25, #0x279000
001dc018: ldr      s1, [x23, #0x60]
001dc01c: ldr      x20, [x27, #0x2c8]
001dc020: ldr      s0, [x25, #0x2d8]
001dc024: nop      
001dc028: adr      x0, #0x279050
001dc02c: fmul     s11, s0, s1
001dc030: bl       #0x1edacc
001dc034: movi     d1, #0000000000000000
001dc038: fmov     s0, s11
001dc03c: mov      w3, w0
001dc040: adrp     x4, #0x32000
001dc044: add      x4, x4, #0xe89
001dc048: add      x2, x27, #0x118
001dc04c: mov      x0, x20
001dc050: mov      x1, x19
001dc054: mov      x5, xzr
001dc058: mov      x6, xzr
001dc05c: bl       #0x209124
001dc060: fmov     s12, #4.00000000
001dc064: add      x0, sp, #0x10
001dc068: str      wzr, [sp, #0x10]
001dc06c: fadd     s0, s11, s12
001dc070: str      s0, [sp, #0x14]
001dc074: bl       #0x217e84
001dc078: bl       #0x217e14
001dc07c: adrp     x0, #0x2d000
001dc080: add      x0, x0, #0x177
001dc084: nop      
001dc088: adr      x1, #0x2795d0
001dc08c: bl       #0x1dd198
001dc090: adrp     x0, #0x2e000
001dc094: add      x0, x0, #0x5d3
001dc098: nop      
001dc09c: adr      x1, #0x2795f8
001dc0a0: bl       #0x1dd198
001dc0a4: adrp     x0, #0x36000
001dc0a8: add      x0, x0, #0x9a
001dc0ac: nop      
001dc0b0: adr      x1, #0x2795fa
001dc0b4: bl       #0x1dd198
001dc0b8: bl       #0x1db7e8
001dc0bc: movi     d0, #0000000000000000
001dc0c0: fmov     s1, s10
001dc0c4: bl       #0x200d64
001dc0c8: bl       #0x2011c8
001dc0cc: fmov     s0, s8
001dc0d0: fmov     s1, s9
001dc0d4: adrp     x0, #0x2d000
001dc0d8: add      x0, x0, #0xbe5
001dc0dc: bl       #0x1db6e8
001dc0e0: bl       #0x217e14
001dc0e4: ldr      x8, [x21]
001dc0e8: ldr      x19, [x24, #0x7e8]
001dc0ec: ldr      x27, [x8, #0x1480]
001dc0f0: strb     w26, [x27, #0xce]
001dc0f4: cbnz     x19, #0x1dc100
001dc0f8: bl       #0x1ff434
001dc0fc: mov      x19, x0
001dc100: ldr      s0, [x25, #0x2d8]
001dc104: ldr      s1, [x23, #0x60]
001dc108: nop      
001dc10c: adr      x0, #0x279050
001dc110: ldr      x20, [x27, #0x2c8]
001dc114: fmul     s10, s0, s1
001dc118: bl       #0x1edacc
001dc11c: movi     d1, #0000000000000000
001dc120: fmov     s0, s10
001dc124: mov      w3, w0
001dc128: adrp     x4, #0x31000
001dc12c: add      x4, x4, #0x95c
001dc130: add      x2, x27, #0x118
001dc134: mov      x0, x20
001dc138: mov      x1, x19
001dc13c: mov      x5, xzr
001dc140: mov      x6, xzr
001dc144: bl       #0x209124
001dc148: fadd     s0, s10, s12
001dc14c: add      x0, sp, #0x10
001dc150: str      wzr, [sp, #0x10]
001dc154: str      s0, [sp, #0x14]
001dc158: bl       #0x217e84
001dc15c: bl       #0x217e14
001dc160: adrp     x0, #0x34000
001dc164: add      x0, x0, #0x537
001dc168: nop      
001dc16c: adr      x1, #0x2795cd
001dc170: bl       #0x1dd198
001dc174: adrp     x0, #0x30000
001dc178: add      x0, x0, #0xc12
001dc17c: nop      
001dc180: adr      x1, #0x2795ce
001dc184: bl       #0x1dd198
001dc188: adrp     x0, #0x2f000
001dc18c: add      x0, x0, #0x51d
001dc190: nop      
001dc194: adr      x1, #0x2795cf
001dc198: bl       #0x1dd198
001dc19c: nop      
001dc1a0: adr      x19, #0x279608
001dc1a4: adrp     x0, #0x37000
001dc1a8: add      x0, x0, #0x65e
001dc1ac: mov      x1, x19
001dc1b0: bl       #0x1dd198
001dc1b4: ldrb     w8, [x19]
001dc1b8: cbz      w8, #0x1dc1dc
001dc1bc: adrp     x0, #0x32000
001dc1c0: add      x0, x0, #0xe90
001dc1c4: nop      
001dc1c8: adr      x1, #0x27960c
001dc1cc: nop      
001dc1d0: adr      x2, #0x2792a8
001dc1d4: mov      w3, #3
001dc1d8: bl       #0x1dd3cc
001dc1dc: bl       #0x1db7e8
001dc1e0: bl       #0x217e14
001dc1e4: fmov     s0, s8
001dc1e8: fmov     s1, s9
001dc1ec: adrp     x0, #0x34000
001dc1f0: add      x0, x0, #0xe3
001dc1f4: bl       #0x1db6e8
001dc1f8: bl       #0x217e14
001dc1fc: ldr      x8, [x21]
001dc200: ldr      x19, [x24, #0x7e8]
001dc204: ldr      x26, [x8, #0x1480]
001dc208: mov      w8, #1
001dc20c: strb     w8, [x26, #0xce]
001dc210: cbnz     x19, #0x1dc21c
001dc214: bl       #0x1ff434
001dc218: mov      x19, x0
001dc21c: ldr      s0, [x25, #0x2d8]
001dc220: ldr      s1, [x23, #0x60]
001dc224: nop      
001dc228: adr      x0, #0x279050
001dc22c: ldr      x20, [x26, #0x2c8]
001dc230: fmul     s8, s0, s1
001dc234: bl       #0x1edacc
001dc238: movi     d1, #0000000000000000
001dc23c: fmov     s0, s8
001dc240: adrp     x21, #0x30000
001dc244: add      x21, x21, #0x194
001dc248: mov      w3, w0
001dc24c: add      x2, x26, #0x118
001dc250: mov      x0, x20
001dc254: mov      x1, x19
001dc258: mov      x4, x21
001dc25c: mov      x5, xzr
001dc260: mov      x6, xzr
001dc264: bl       #0x209124
001dc268: fmov     s0, #4.00000000
001dc26c: add      x0, sp, #0x10
001dc270: str      wzr, [sp, #0x10]
001dc274: fadd     s0, s8, s0
001dc278: str      s0, [sp, #0x14]
001dc27c: bl       #0x217e84
001dc280: bl       #0x217e14
001dc284: nop      
001dc288: adr      x1, #0x2795d1
001dc28c: nop      
001dc290: adr      x2, #0x2792c0
001dc294: mov      x0, x21
001dc298: mov      w3, #2
001dc29c: bl       #0x1ddc1c
001dc2a0: b        #0x1dcd18
001dc2a4: fmov     s0, s12
001dc2a8: fmov     s1, s11
001dc2ac: adrp     x0, #0x2d000
001dc2b0: add      x0, x0, #0x690
001dc2b4: bl       #0x1db6e8
001dc2b8: nop      
001dc2bc: adr      x19, #0x279712
001dc2c0: adrp     x0, #0x36000
001dc2c4: add      x0, x0, #0xa3
001dc2c8: mov      x1, x19
001dc2cc: bl       #0x1dd198
001dc2d0: ldrb     w8, [x19]
001dc2d4: cbz      w8, #0x1dc528
001dc2d8: adrp     x19, #0x279000
001dc2dc: nop      
001dc2e0: adr      x9, #0x1b067c
001dc2e4: ldrsw    x8, [x19, #0x7a8]
001dc2e8: mov      w0, #2
001dc2ec: mov      x1, xzr
001dc2f0: mov      w2, wzr
001dc2f4: ldr      w8, [x9, x8, lsl #2]
001dc2f8: str      w8, [sp, #0xc]
001dc2fc: bl       #0x22e8b0
001dc300: adrp     x0, #0x36000
001dc304: add      x0, x0, #0xb8a
001dc308: bl       #0x21500c
001dc30c: fmov     s0, #-1.00000000
001dc310: bl       #0x201000
001dc314: adrp     x8, #0x37000
001dc318: mov      w20, #-0x40800000
001dc31c: adrp     x0, #0x31000
001dc320: add      x0, x0, #0x963
001dc324: ldr      s0, [x8, #0xed8]
001dc328: add      x1, sp, #0x10
001dc32c: str      w20, [sp, #0x10]
001dc330: fmul     s8, s11, s0
001dc334: str      s8, [sp, #0x14]
001dc338: bl       #0x224a94
001dc33c: tbz      w0, #0, #0x1dc388
001dc340: mov      x21, xzr
001dc344: nop      
001dc348: adr      x23, #0x1b06f8
001dc34c: ldr      w8, [x19, #0x7a8]
001dc350: ldrsw    x9, [x23, x21, lsl #2]
001dc354: add      x3, sp, #0x10
001dc358: mov      w2, wzr
001dc35c: str      xzr, [sp, #0x10]
001dc360: cmp      x21, x8
001dc364: add      x0, x23, x9
001dc368: cset     w1, eq
001dc36c: bl       #0x218df4
001dc370: tbz      w0, #0, #0x1dc378
001dc374: str      w21, [x19, #0x7a8]
001dc378: add      x21, x21, #1
001dc37c: cmp      x21, #0x1f
001dc380: b.ne     #0x1dc34c
001dc384: bl       #0x224cd0
001dc388: bl       #0x2010dc
001dc38c: bl       #0x22e2b8
001dc390: adrp     x0, #0x2d000
001dc394: add      x0, x0, #0x69e
001dc398: bl       #0x21500c
001dc39c: fmov     s0, #-1.00000000
001dc3a0: bl       #0x201000
001dc3a4: adrp     x0, #0x30000
001dc3a8: add      x0, x0, #0x709
001dc3ac: add      x1, sp, #0x10
001dc3b0: str      w20, [sp, #0x10]
001dc3b4: str      s8, [sp, #0x14]
001dc3b8: bl       #0x224a94
001dc3bc: tbz      w0, #0, #0x1dc514
001dc3c0: add      x19, sp, #0xc
001dc3c4: nop      
001dc3c8: adr      x0, #0x279620
001dc3cc: adrp     x2, #0x77000
001dc3d0: add      x2, x2, #0xd60
001dc3d4: add      x1, sp, #0xc
001dc3d8: add      x3, sp, #0x10
001dc3dc: mov      x4, sp
001dc3e0: str      x19, [sp, #0x10]
001dc3e4: bl       #0x1da06c
001dc3e8: ldr      w8, [x0, #0x18]
001dc3ec: adrp     x0, #0x37000
001dc3f0: add      x0, x0, #0xdf
001dc3f4: add      x3, sp, #0x10
001dc3f8: mov      w2, wzr
001dc3fc: str      xzr, [sp, #0x10]
001dc400: cmp      w8, #0
001dc404: cset     w1, eq
001dc408: bl       #0x218df4
001dc40c: tbz      w0, #0, #0x1dc444
001dc410: ldr      w8, [sp, #0xc]
001dc414: nop      
001dc418: adr      x0, #0x279620
001dc41c: adrp     x2, #0x77000
001dc420: add      x2, x2, #0xd60
001dc424: add      x1, sp, #0xc
001dc428: cmn      w8, #1
001dc42c: add      x3, sp, #0x10
001dc430: mov      x4, sp
001dc434: csel     w20, wzr, w8, eq
001dc438: str      x19, [sp, #0x10]
001dc43c: bl       #0x1da06c
001dc440: stp      w20, wzr, [x0, #0x14]
001dc444: nop      
001dc448: adr      x8, #0x279738
001dc44c: ldp      x23, x24, [x8]
001dc450: cmp      x23, x24
001dc454: b.eq     #0x1dc510
001dc458: nop      
001dc45c: adr      x19, #0x279620
001dc460: add      x25, sp, #0xc
001dc464: adrp     x20, #0x77000
001dc468: add      x20, x20, #0xd60
001dc46c: ldr      w9, [sp, #0xc]
001dc470: ldr      w8, [x23, #0xc]
001dc474: cmn      w9, #1
001dc478: b.eq     #0x1dc488
001dc47c: cmp      w8, w9
001dc480: b.eq     #0x1dc494
001dc484: b        #0x1dc504
001dc488: sub      w8, w8, #0x46
001dc48c: cmp      w8, #0x13
001dc490: b.hi     #0x1dc504
001dc494: ldr      x21, [x23]
001dc498: add      x1, sp, #0xc
001dc49c: add      x3, sp, #0x10
001dc4a0: add      x4, sp, #0x27
001dc4a4: mov      x0, x19
001dc4a8: mov      x2, x20
001dc4ac: str      x25, [sp, #0x10]
001dc4b0: bl       #0x1da06c
001dc4b4: ldr      w8, [x23, #8]
001dc4b8: ldr      w9, [x0, #0x18]
001dc4bc: mov      x3, sp
001dc4c0: mov      x0, x21
001dc4c4: mov      w2, wzr
001dc4c8: str      xzr, [sp]
001dc4cc: cmp      w9, w8
001dc4d0: cset     w1, eq
001dc4d4: bl       #0x218df4
001dc4d8: tbz      w0, #0, #0x1dc504
001dc4dc: str      x25, [sp, #0x10]
001dc4e0: add      x1, sp, #0xc
001dc4e4: add      x3, sp, #0x10
001dc4e8: ldr      d8, [x23, #8]
001dc4ec: mov      x4, sp
001dc4f0: mov      x0, x19
001dc4f4: mov      x2, x20
001dc4f8: bl       #0x1da06c
001dc4fc: rev64    v0.2s, v8.2s
001dc500: stur     d0, [x0, #0x14]
001dc504: add      x23, x23, #0x10
001dc508: cmp      x23, x24
001dc50c: b.ne     #0x1dc46c
001dc510: bl       #0x224cd0
001dc514: bl       #0x2010dc
001dc518: mov      w0, #1
001dc51c: mov      x1, xzr
001dc520: mov      w2, #1
001dc524: bl       #0x22e8b0
001dc528: bl       #0x1db7e8
001dc52c: b        #0x1dcd20
001dc530: fmov     s0, s8
001dc534: fmov     s1, s11
001dc538: adrp     x0, #0x35000
001dc53c: add      x0, x0, #0xa03
001dc540: bl       #0x1db6e8
001dc544: bl       #0x217e14
001dc548: adrp     x24, #0x277000
001dc54c: adrp     x25, #0x279000
001dc550: ldr      x24, [x24, #0x4a0]
001dc554: ldr      x19, [x25, #0x7e8]
001dc558: ldr      x8, [x24]
001dc55c: ldr      x27, [x8, #0x1480]
001dc560: mov      w8, #1
001dc564: strb     w8, [x27, #0xce]
001dc568: cbnz     x19, #0x1dc574
001dc56c: bl       #0x1ff434
001dc570: mov      x19, x0
001dc574: adrp     x26, #0x279000
001dc578: ldr      s1, [x23, #0x60]
001dc57c: ldr      x20, [x27, #0x2c8]
001dc580: ldr      s0, [x26, #0x2d8]
001dc584: nop      
001dc588: adr      x0, #0x279050
001dc58c: fmul     s11, s0, s1
001dc590: bl       #0x1edacc
001dc594: movi     d1, #0000000000000000
001dc598: fmov     s0, s11
001dc59c: adrp     x21, #0x32000
001dc5a0: add      x21, x21, #0x3c7
001dc5a4: mov      w3, w0
001dc5a8: add      x2, x27, #0x118
001dc5ac: mov      x0, x20
001dc5b0: mov      x1, x19
001dc5b4: mov      x4, x21
001dc5b8: mov      x5, xzr
001dc5bc: mov      x6, xzr
001dc5c0: bl       #0x209124
001dc5c4: fmov     s0, #4.00000000
001dc5c8: add      x0, sp, #0x10
001dc5cc: str      wzr, [sp, #0x10]
001dc5d0: fadd     s0, s11, s0
001dc5d4: str      s0, [sp, #0x14]
001dc5d8: bl       #0x217e84
001dc5dc: bl       #0x217e14
001dc5e0: nop      
001dc5e4: adr      x1, #0x2795a1
001dc5e8: mov      x0, x21
001dc5ec: bl       #0x1dd198
001dc5f0: adrp     x0, #0x33000
001dc5f4: add      x0, x0, #0x9f6
001dc5f8: nop      
001dc5fc: adr      x1, #0x27970c
001dc600: nop      
001dc604: adr      x2, #0x279088
001dc608: mov      w3, #3
001dc60c: bl       #0x1dd3cc
001dc610: adrp     x0, #0x32000
001dc614: add      x0, x0, #0x913
001dc618: nop      
001dc61c: adr      x1, #0x2795a2
001dc620: bl       #0x1dd198
001dc624: mov      w8, #0x447a0000
001dc628: fmov     s0, #1.00000000
001dc62c: adrp     x0, #0x36000
001dc630: add      x0, x0, #0x96
001dc634: fmov     s1, w8
001dc638: nop      
001dc63c: adr      x1, #0x2795a4
001dc640: bl       #0x1dd6e0
001dc644: adrp     x0, #0x36000
001dc648: add      x0, x0, #0xb60
001dc64c: nop      
001dc650: adr      x1, #0x278ff8
001dc654: bl       #0x1dda38
001dc658: adrp     x0, #0x37000
001dc65c: add      x0, x0, #0xc0a
001dc660: nop      
001dc664: adr      x1, #0x2795e8
001dc668: nop      
001dc66c: adr      x2, #0x2790a0
001dc670: mov      w3, #2
001dc674: bl       #0x1dd3cc
001dc678: adrp     x0, #0x2f000
001dc67c: add      x0, x0, #0x50a
001dc680: nop      
001dc684: adr      x1, #0x2795ec
001dc688: bl       #0x1dd198
001dc68c: adrp     x0, #0x35000
001dc690: add      x0, x0, #0xa0f
001dc694: nop      
001dc698: adr      x1, #0x279710
001dc69c: bl       #0x1dd198
001dc6a0: nop      
001dc6a4: adr      x19, #0x279708
001dc6a8: adrp     x0, #0x2d000
001dc6ac: add      x0, x0, #0x676
001dc6b0: mov      x1, x19
001dc6b4: bl       #0x1dd198
001dc6b8: ldrb     w8, [x19]
001dc6bc: cbz      w8, #0x1dc6e0
001dc6c0: mov      w8, #0x42c80000
001dc6c4: fmov     s0, #1.00000000
001dc6c8: adrp     x0, #0x32000
001dc6cc: add      x0, x0, #0x3d2
001dc6d0: fmov     s1, w8
001dc6d4: nop      
001dc6d8: adr      x1, #0x2790b0
001dc6dc: bl       #0x1dd6e0
001dc6e0: adrp     x8, #0x279000
001dc6e4: ldr      w8, [x8, #0x5e8]
001dc6e8: cbz      w8, #0x1dc704
001dc6ec: adrp     x0, #0x31000
001dc6f0: add      x0, x0, #0x25f
001dc6f4: nop      
001dc6f8: adr      x1, #0x279618
001dc6fc: bl       #0x1dd198
001dc700: b        #0x1dc70c
001dc704: adrp     x8, #0x279000
001dc708: strb     wzr, [x8, #0x618]
001dc70c: bl       #0x1db7e8
001dc710: movi     d0, #0000000000000000
001dc714: fmov     s1, s10
001dc718: bl       #0x200d64
001dc71c: bl       #0x2011c8
001dc720: fmov     s0, s8
001dc724: fmov     s1, s9
001dc728: adrp     x0, #0x36000
001dc72c: add      x0, x0, #0x5fa
001dc730: bl       #0x1db6e8
001dc734: bl       #0x217e14
001dc738: ldr      x8, [x24]
001dc73c: ldr      x19, [x25, #0x7e8]
001dc740: ldr      x27, [x8, #0x1480]
001dc744: mov      w8, #1
001dc748: strb     w8, [x27, #0xce]
001dc74c: cbnz     x19, #0x1dc758
001dc750: bl       #0x1ff434
001dc754: mov      x19, x0
001dc758: ldr      s0, [x26, #0x2d8]
001dc75c: ldr      s1, [x23, #0x60]
001dc760: nop      
001dc764: adr      x0, #0x279050
001dc768: ldr      x20, [x27, #0x2c8]
001dc76c: fmul     s10, s0, s1
001dc770: bl       #0x1edacc
001dc774: movi     d1, #0000000000000000
001dc778: fmov     s0, s10
001dc77c: adrp     x21, #0x31000
001dc780: add      x21, x21, #0x269
001dc784: mov      w3, w0
001dc788: add      x2, x27, #0x118
001dc78c: mov      x0, x20
001dc790: mov      x1, x19
001dc794: mov      x4, x21
001dc798: mov      x5, xzr
001dc79c: mov      x6, xzr
001dc7a0: bl       #0x209124
001dc7a4: fmov     s0, #4.00000000
001dc7a8: add      x0, sp, #0x10
001dc7ac: str      wzr, [sp, #0x10]
001dc7b0: fadd     s0, s10, s0
001dc7b4: str      s0, [sp, #0x14]
001dc7b8: bl       #0x217e84
001dc7bc: bl       #0x217e14
001dc7c0: nop      
001dc7c4: adr      x19, #0x279778
001dc7c8: mov      x0, x21
001dc7cc: mov      x1, x19
001dc7d0: bl       #0x1dd198
001dc7d4: ldrb     w8, [x19]
001dc7d8: cbz      w8, #0x1dcbac
001dc7dc: adrp     x0, #0x36000
001dc7e0: add      x0, x0, #0xb6a
001dc7e4: nop      
001dc7e8: adr      x1, #0x27977c
001dc7ec: nop      
001dc7f0: adr      x2, #0x2790b8
001dc7f4: mov      w3, #5
001dc7f8: bl       #0x1dd3cc
001dc7fc: nop      
001dc800: adr      x19, #0x279780
001dc804: adrp     x0, #0x34000
001dc808: add      x0, x0, #0xacc
001dc80c: nop      
001dc810: adr      x2, #0x2790e0
001dc814: mov      x1, x19
001dc818: mov      w3, #6
001dc81c: bl       #0x1dd3cc
001dc820: mov      w8, #-0x3ccc0000
001dc824: mov      w9, #0x43340000
001dc828: adrp     x0, #0x37000
001dc82c: add      x0, x0, #0xc0f
001dc830: fmov     s0, w8
001dc834: fmov     s1, w9
001dc838: nop      
001dc83c: adr      x1, #0x279784
001dc840: bl       #0x1dd6e0
001dc844: ldr      w8, [x19]
001dc848: cmp      w8, #2
001dc84c: b.gt     #0x1dc864
001dc850: cmp      w8, #1
001dc854: b.eq     #0x1dcb34
001dc858: cmp      w8, #2
001dc85c: b.eq     #0x1dc874
001dc860: b        #0x1dcbac
001dc864: cmp      w8, #3
001dc868: b.eq     #0x1dcb6c
001dc86c: cmp      w8, #4
001dc870: b.ne     #0x1dcbac
001dc874: mov      w8, #0x43340000
001dc878: fmov     s0, #5.00000000
001dc87c: adrp     x0, #0x2f000
001dc880: add      x0, x0, #0xc76
001dc884: fmov     s1, w8
001dc888: nop      
001dc88c: adr      x1, #0x279114
001dc890: bl       #0x1dd6e0
001dc894: mov      w8, #0x42200000
001dc898: adrp     x0, #0x2e000
001dc89c: add      x0, x0, #0x598
001dc8a0: fmov     s0, #1.00000000
001dc8a4: nop      
001dc8a8: adr      x1, #0x279118
001dc8ac: fmov     s1, w8
001dc8b0: b        #0x1dcba8
001dc8b4: adrp     x8, #0x77000
001dc8b8: adrp     x1, #0x36000
001dc8bc: add      x1, x1, #0x61c
001dc8c0: ldr      q0, [x8, #0xfd0]
001dc8c4: add      x0, sp, #0x10
001dc8c8: str      q0, [sp, #0x10]
001dc8cc: bl       #0x21512c
001dc8d0: ldr      w8, [x19, #0x7a0]
001dc8d4: cbnz     w8, #0x1dc914
001dc8d8: adrp     x8, #0x37000
001dc8dc: fmov     s1, #10.00000000
001dc8e0: adrp     x0, #0x34000
001dc8e4: add      x0, x0, #0x52d
001dc8e8: ldr      s0, [x8, #0xf14]
001dc8ec: nop      
001dc8f0: adr      x1, #0x279284
001dc8f4: bl       #0x1dd6e0
001dc8f8: movi     d0, #0000000000000000
001dc8fc: fmov     s1, #10.00000000
001dc900: adrp     x0, #0x32000
001dc904: add      x0, x0, #0x3e8
001dc908: nop      
001dc90c: adr      x1, #0x279288
001dc910: bl       #0x1dd6e0
001dc914: adrp     x0, #0x2e000
001dc918: add      x0, x0, #0x3d
001dc91c: nop      
001dc920: adr      x1, #0x2795d4
001dc924: bl       #0x1dd198
001dc928: adrp     x0, #0x2d000
001dc92c: add      x0, x0, #0x16e
001dc930: nop      
001dc934: adr      x1, #0x2797a4
001dc938: nop      
001dc93c: adr      x2, #0x279250
001dc940: mov      w3, #4
001dc944: bl       #0x1dd3cc
001dc948: adrp     x0, #0x2f000
001dc94c: add      x0, x0, #0xd8
001dc950: nop      
001dc954: adr      x1, #0x27928c
001dc958: bl       #0x1dda38
001dc95c: ldr      w8, [x19, #0x7a0]
001dc960: cbnz     w8, #0x1dc9a0
001dc964: adrp     x8, #0x37000
001dc968: fmov     s1, #10.00000000
001dc96c: adrp     x0, #0x34000
001dc970: add      x0, x0, #0x52d
001dc974: ldr      s0, [x8, #0xf14]
001dc978: nop      
001dc97c: adr      x1, #0x27929c
001dc980: bl       #0x1dd6e0
001dc984: movi     d0, #0000000000000000
001dc988: fmov     s1, #10.00000000
001dc98c: adrp     x0, #0x32000
001dc990: add      x0, x0, #0x3e8
001dc994: nop      
001dc998: adr      x1, #0x2792a0
001dc99c: bl       #0x1dd6e0
001dc9a0: bl       #0x1db7e8
001dc9a4: bl       #0x217e14
001dc9a8: fmov     s0, s8
001dc9ac: fmov     s1, s9
001dc9b0: adrp     x0, #0x2e000
001dc9b4: add      x0, x0, #0x5c0
001dc9b8: bl       #0x1db6e8
001dc9bc: bl       #0x217e14
001dc9c0: ldr      x8, [x21]
001dc9c4: ldr      x19, [x24, #0x7e8]
001dc9c8: ldr      x21, [x8, #0x1480]
001dc9cc: mov      w8, #1
001dc9d0: strb     w8, [x21, #0xce]
001dc9d4: cbnz     x19, #0x1dc9e0
001dc9d8: bl       #0x1ff434
001dc9dc: mov      x19, x0
001dc9e0: ldr      s0, [x25, #0x2d8]
001dc9e4: ldr      s1, [x23, #0x60]
001dc9e8: nop      
001dc9ec: adr      x0, #0x279050
001dc9f0: ldr      x20, [x21, #0x2c8]
001dc9f4: fmul     s8, s0, s1
001dc9f8: bl       #0x1edacc
001dc9fc: movi     d1, #0000000000000000
001dca00: fmov     s0, s8
001dca04: mov      w3, w0
001dca08: adrp     x4, #0x34000
001dca0c: add      x4, x4, #0xfc0
001dca10: add      x2, x21, #0x118
001dca14: mov      x0, x20
001dca18: mov      x1, x19
001dca1c: mov      x5, xzr
001dca20: mov      x6, xzr
001dca24: bl       #0x209124
001dca28: fmov     s0, #4.00000000
001dca2c: add      x0, sp, #0x10
001dca30: str      wzr, [sp, #0x10]
001dca34: fadd     s0, s8, s0
001dca38: str      s0, [sp, #0x14]
001dca3c: bl       #0x217e84
001dca40: bl       #0x217e14
001dca44: adrp     x0, #0x36000
001dca48: add      x0, x0, #0xb7e
001dca4c: nop      
001dca50: adr      x1, #0x279014
001dca54: bl       #0x1dda38
001dca58: adrp     x0, #0x32000
001dca5c: add      x0, x0, #0x3f6
001dca60: nop      
001dca64: adr      x1, #0x279711
001dca68: bl       #0x1dd198
001dca6c: nop      
001dca70: adr      x1, #0x2792e0
001dca74: mov      w0, #0x15
001dca78: bl       #0x1edccc
001dca7c: nop      
001dca80: adr      x19, #0x2792f8
001dca84: mov      w0, #0x16
001dca88: mov      x1, x19
001dca8c: bl       #0x1edccc
001dca90: add      x1, sp, #0x10
001dca94: mov      w0, #0x17
001dca98: stp      xzr, xzr, [sp, #0x10]
001dca9c: bl       #0x1edccc
001dcaa0: mov      w0, #5
001dcaa4: mov      x1, x19
001dcaa8: bl       #0x1edccc
001dcaac: adrp     x8, #0x78000
001dcab0: ldr      s0, [x23, #0x60]
001dcab4: add      x1, sp, #0x10
001dcab8: ldr      d1, [x8, #0xd40]
001dcabc: mov      w0, #0xb
001dcac0: fmul     v0.2s, v1.2s, v0.s[0]
001dcac4: str      d0, [sp, #0x10]
001dcac8: bl       #0x1ee294
001dcacc: fmov     s0, #6.00000000
001dcad0: ldr      s1, [x23, #0x60]
001dcad4: mov      w0, #0xc
001dcad8: fmul     s0, s1, s0
001dcadc: bl       #0x1ee07c
001dcae0: bl       #0x1fc534
001dcae4: mov      w8, #0x42080000
001dcae8: ldr      s1, [x23, #0x60]
001dcaec: adrp     x0, #0x37000
001dcaf0: add      x0, x0, #0x640
001dcaf4: fmov     s2, w8
001dcaf8: add      x1, sp, #0x10
001dcafc: fmul     s1, s1, s2
001dcb00: stp      s0, s1, [sp, #0x10]
001dcb04: bl       #0x215d54
001dcb08: mov      w19, w0
001dcb0c: mov      w0, #2
001dcb10: bl       #0x1ee3ac
001dcb14: mov      w0, #4
001dcb18: bl       #0x1ede0c
001dcb1c: bl       #0x217e14
001dcb20: tbz      w19, #0, #0x1dcd18
001dcb24: adrp     x8, #0x279000
001dcb28: mov      w9, #1
001dcb2c: strb     w9, [x8, #0x5f9]
001dcb30: b        #0x1dcd18
001dcb34: mov      w8, #0x44610000
001dcb38: fmov     s0, #30.00000000
001dcb3c: adrp     x0, #0x37000
001dcb40: add      x0, x0, #0x621
001dcb44: fmov     s1, w8
001dcb48: nop      
001dcb4c: adr      x1, #0x279110
001dcb50: bl       #0x1dd6e0
001dcb54: adrp     x0, #0x33000
001dcb58: add      x0, x0, #0x436
001dcb5c: nop      
001dcb60: adr      x1, #0x279788
001dcb64: bl       #0x1dd198
001dcb68: b        #0x1dcbac
001dcb6c: mov      w8, #0x43340000
001dcb70: fmov     s0, #5.00000000
001dcb74: adrp     x0, #0x37000
001dcb78: add      x0, x0, #0xb0
001dcb7c: fmov     s1, w8
001dcb80: nop      
001dcb84: adr      x1, #0x27911c
001dcb88: bl       #0x1dd6e0
001dcb8c: adrp     x8, #0x37000
001dcb90: fmov     s1, #8.00000000
001dcb94: adrp     x0, #0x30000
001dcb98: add      x0, x0, #0xbfe
001dcb9c: ldr      s0, [x8, #0xf14]
001dcba0: nop      
001dcba4: adr      x1, #0x279120
001dcba8: bl       #0x1dd6e0
001dcbac: bl       #0x1db7e8
001dcbb0: bl       #0x217e14
001dcbb4: fmov     s0, s8
001dcbb8: fmov     s1, s9
001dcbbc: adrp     x0, #0x37000
001dcbc0: add      x0, x0, #0xc1a
001dcbc4: bl       #0x1db6e8
001dcbc8: bl       #0x217e14
001dcbcc: ldr      x8, [x24]
001dcbd0: ldr      x19, [x25, #0x7e8]
001dcbd4: ldr      x21, [x8, #0x1480]
001dcbd8: mov      w8, #1
001dcbdc: strb     w8, [x21, #0xce]
001dcbe0: cbnz     x19, #0x1dcbec
001dcbe4: bl       #0x1ff434
001dcbe8: mov      x19, x0
001dcbec: ldr      s0, [x26, #0x2d8]
001dcbf0: ldr      s1, [x23, #0x60]
001dcbf4: nop      
001dcbf8: adr      x0, #0x279050
001dcbfc: ldr      x20, [x21, #0x2c8]
001dcc00: fmul     s8, s0, s1
001dcc04: bl       #0x1edacc
001dcc08: movi     d1, #0000000000000000
001dcc0c: fmov     s0, s8
001dcc10: mov      w3, w0
001dcc14: adrp     x4, #0x36000
001dcc18: add      x4, x4, #0xb6f
001dcc1c: add      x2, x21, #0x118
001dcc20: mov      x0, x20
001dcc24: mov      x1, x19
001dcc28: mov      x5, xzr
001dcc2c: mov      x6, xzr
001dcc30: bl       #0x209124
001dcc34: fmov     s0, #4.00000000
001dcc38: add      x0, sp, #0x10
001dcc3c: str      wzr, [sp, #0x10]
001dcc40: fadd     s0, s8, s0
001dcc44: str      s0, [sp, #0x14]
001dcc48: bl       #0x217e84
001dcc4c: bl       #0x217e14
001dcc50: adrp     x8, #0x279000
001dcc54: ldrb     w8, [x8, #0x778]
001dcc58: cbz      w8, #0x1dcd04
001dcc5c: nop      
001dcc60: adr      x19, #0x27978c
001dcc64: adrp     x0, #0x36000
001dcc68: add      x0, x0, #0x60b
001dcc6c: nop      
001dcc70: adr      x2, #0x279128
001dcc74: mov      x1, x19
001dcc78: mov      w3, #7
001dcc7c: bl       #0x1dd3cc
001dcc80: ldr      w8, [x19]
001dcc84: cmp      w8, #4
001dcc88: b.lt     #0x1dcd18
001dcc8c: mov      w8, #-0x3d740000
001dcc90: mov      w9, #0x428c0000
001dcc94: adrp     x0, #0x2e000
001dcc98: add      x0, x0, #0x5a5
001dcc9c: fmov     s0, w8
001dcca0: fmov     s1, w9
001dcca4: nop      
001dcca8: adr      x1, #0x279160
001dccac: bl       #0x1dd6e0
001dccb0: adrp     x8, #0x279000
001dccb4: ldr      w8, [x8, #0x78c]
001dccb8: cmp      w8, #5
001dccbc: b.lt     #0x1dcd18
001dccc0: mov      w8, #0x430c0000
001dccc4: fmov     s0, #2.00000000
001dccc8: adrp     x0, #0x2d000
001dcccc: add      x0, x0, #0x14a
001dccd0: fmov     s1, w8
001dccd4: nop      
001dccd8: adr      x1, #0x279164
001dccdc: bl       #0x1dd6e0
001dcce0: mov      w8, #0x42200000
001dcce4: fmov     s0, #1.00000000
001dcce8: adrp     x0, #0x37000
001dccec: add      x0, x0, #0xbb
001dccf0: fmov     s1, w8
001dccf4: nop      
001dccf8: adr      x1, #0x279118
001dccfc: bl       #0x1dd6e0
001dcd00: b        #0x1dcd18
001dcd04: nop      
001dcd08: adr      x0, #0x279168
001dcd0c: adrp     x1, #0x2e000
001dcd10: add      x1, x1, #0x1d
001dcd14: bl       #0x21512c
001dcd18: bl       #0x1db7e8
001dcd1c: bl       #0x200978
001dcd20: bl       #0x1de0e4
001dcd24: ldr      x8, [x22, #0x28]
001dcd28: ldr      x9, [sp, #0x28]
001dcd2c: cmp      x8, x9
001dcd30: b.ne     #0x1dcd60
001dcd34: ldp      x20, x19, [sp, #0xb0]
001dcd38: ldr      d12, [sp, #0x30]
001dcd3c: ldp      x22, x21, [sp, #0xa0]
001dcd40: ldp      x24, x23, [sp, #0x90]
001dcd44: ldp      x26, x25, [sp, #0x80]
001dcd48: ldp      x28, x27, [sp, #0x70]
001dcd4c: ldp      x29, x30, [sp, #0x60]
001dcd50: ldp      d9, d8, [sp, #0x50]
001dcd54: ldp      d11, d10, [sp, #0x40]
001dcd58: add      sp, sp, #0xc0
001dcd5c: ret      
001dcd60: bl       #0x26c3b0
001dcd64: sub      sp, sp, #0xb0
001dcd68: stp      d9, d8, [sp, #0x60]
001dcd6c: stp      x29, x30, [sp, #0x70]
001dcd70: stp      x24, x23, [sp, #0x80]
001dcd74: stp      x22, x21, [sp, #0x90]
001dcd78: stp      x20, x19, [sp, #0xa0]
001dcd7c: add      x29, sp, #0x70
001dcd80: mrs      x20, tpidr_el0
001dcd84: ldr      x8, [x20, #0x28]
001dcd88: stur     x8, [x29, #-0x18]
001dcd8c: adrp     x8, #0x279000
001dcd90: ldrb     w8, [x8, #0x64]
001dcd94: cbz      w8, #0x1dd168
001dcd98: adrp     x9, #0x279000
001dcd9c: add      x9, x9, #0x68
001dcda0: mov      x19, x0
001dcda4: ldp      w8, w9, [x9]
001dcda8: adrp     x21, #0x279000
001dcdac: cmp      w8, w9
001dcdb0: b.ne     #0x1dcdc8
001dcdb4: fmov     s0, #1.00000000
001dcdb8: ldr      s1, [x21, #0x70]
001dcdbc: fcmp     s1, s0
001dcdc0: b.mi     #0x1dcdd4
001dcdc4: b        #0x1dce00
001dcdc8: adrp     x9, #0x279000
001dcdcc: add      x9, x9, #0x6c
001dcdd0: stp      w8, wzr, [x9]
001dcdd4: bl       #0x1f2744
001dcdd8: fmov     s0, #5.00000000
001dcddc: ldr      s1, [x0, #0x18]
001dcde0: ldr      s2, [x21, #0x70]
001dcde4: fmadd    s0, s1, s0, s2
001dcde8: fmov     s1, #1.00000000
001dcdec: fcmp     s0, s1
001dcdf0: str      s0, [x21, #0x70]
001dcdf4: b.le     #0x1dce00
001dcdf8: mov      w8, #0x3f800000
001dcdfc: str      w8, [x21, #0x70]
001dce00: adrp     x22, #0x279000
001dce04: ldr      s0, [x22, #0x60]
001dce08: str      q0, [sp]
001dce0c: bl       #0x1f2744
001dce10: movi     v0.2s, #0x3f, lsl #24
001dce14: adrp     x8, #0x78000
001dce18: ldr      d2, [x0, #8]
001dce1c: ldr      d1, [x8, #0xdd8]
001dce20: ldr      q3, [sp]
001dce24: adrp     x8, #0x78000
001dce28: adrp     x9, #0x279000
001dce2c: add      x9, x9, #0x74
001dce30: sub      x0, x29, #0x28
001dce34: fmul     v1.2s, v1.2s, v3.s[0]
001dce38: add      x2, sp, #0x30
001dce3c: mov      w1, #1
001dce40: fmul     v0.2s, v2.2s, v0.2s
001dce44: ldr      d2, [x8, #0xe10]
001dce48: str      xzr, [sp, #0x30]
001dce4c: mov      w24, #1
001dce50: stur     d1, [x29, #-0x20]
001dce54: fmla     v0.2s, v2.2s, v1.2s
001dce58: ldur     x8, [x29, #-0x20]
001dce5c: str      x8, [x9, #8]
001dce60: stur     d0, [x29, #-0x28]
001dce64: str      d0, [x9]
001dce68: bl       #0x1fe918
001dce6c: sub      x0, x29, #0x20
001dce70: mov      w1, #1
```
