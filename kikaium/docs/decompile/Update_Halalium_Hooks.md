# Update_Halalium_Hooks

- VA: `0x1d7a10`
- Size scanned: `0x4b0`
- Pass A instructions: 300
- Method: **2-command decompile** — (1) Capstone Pass A full disasm (2) Capstone Pass B field/RVA/string recovery

## Pass B — field LDR/STR map

| Offset | Meaning | Count |
|---|---|---|
| `0xc` | unknown | 1 |
| `0x10` | unknown | 2 |
| `0x21` | unknown | 1 |
| `0x28` | players_list / camera holder | 1 |
| `0x2c` | unknown | 1 |
| `0x30` | Photon.isLocal (ldrb) / nested | 2 |
| `0x50` | unknown | 1 |
| `0x60` | unknown | 1 |
| `0x68` | unknown | 1 |
| `0x70` | unknown | 1 |
| `0x79` | Player.team (byte) | 2 |
| `0x88` | Player.weaponry | 2 |
| `0x98` | Player.movement | 1 |
| `0xa0` | Weaponry.weapon OR arms | 2 |
| `0xa8` | Player.hit / weapon params | 1 |
| `0xb0` | unknown | 1 |
| `0xd8` | Player.visible (strb) | 1 |
| `0xe8` | Player.main_camera | 1 |
| `0x160` | Player.photon_player | 3 |
| `0x168` | unknown | 2 |
| `0x1a4` | unknown | 2 |

## Pass B — game RVAs (mov+movk)

- `w9` = `0x8c69cb0` @ `0x1d7acc`
- `w9` = `0x8e8ae24` @ `0x1d7b0c`
- `w9` = `0x5fb7bc0` @ `0x1d7da8`

## Pass B — strings (ADRP+ADD resolved only)

- `Halalium_Hooks`
- `[ping] photon_raw=%d prop_raw=%d published=%d attempts=%llu`

## Pass B — calls (first 40)

- `0x1d7a34` → `x8` (blr)
- `0x1d7a4c` → `0x1d8968` (bl)
- `0x1d7a54` → `0x1d8a80` (bl)
- `0x1d7a6c` → `0x26c530` (bl)
- `0x1d7ab0` → `0x268190` (bl)
- `0x1d7ad4` → `x8` (blr)
- `0x1d7b1c` → `x8` (blr)
- `0x1d7b68` → `0x1d8928` (bl)
- `0x1d7bb0` → `0x26c530` (bl)
- `0x1d7c10` → `0x26c540` (bl)
- `0x1d7c18` → `0x1d9b94` (bl)
- `0x1d7c64` → `0x1d93dc` (bl)
- `0x1d7c78` → `0x1d9b94` (bl)
- `0x1d7ca4` → `0x1d9c30` (bl)
- `0x1d7cb0` → `0x1d9c30` (bl)
- `0x1d7cd0` → `0x1d9c54` (bl)
- `0x1d7d04` → `0x1d9c00` (bl)
- `0x1d7d14` → `0x1d9bd4` (bl)
- `0x1d7d20` → `0x1d9c00` (bl)
- `0x1d7db8` → `x8` (blr)
- `0x1d7dc0` → `0x1d9e00` (bl)
- `0x1d7e80` → `0x1df078` (bl)

## Pass A — full disasm

```
001d7a10: stp      x29, x30, [sp, #0x10]
001d7a14: stp      x26, x25, [sp, #0x20]
001d7a18: stp      x24, x23, [sp, #0x30]
001d7a1c: stp      x22, x21, [sp, #0x40]
001d7a20: stp      x20, x19, [sp, #0x50]
001d7a24: add      x29, sp, #0x10
001d7a28: adrp     x8, #0x279000
001d7a2c: mov      x19, x0
001d7a30: ldr      x8, [x8, #0x5a8]
001d7a34: blr      x8
001d7a38: ldr      x8, [x19, #0x160]
001d7a3c: cbz      x8, #0x1d7df8
001d7a40: adrp     x21, #0x279000
001d7a44: mov      x1, x19
001d7a48: ldr      x0, [x21, #0x4f8]
001d7a4c: bl       #0x1d8968
001d7a50: ldr      x0, [x21, #0x4f8]
001d7a54: bl       #0x1d8a80
001d7a58: ldr      x8, [x19, #0x160]
001d7a5c: ldrb     w8, [x8, #0x30]
001d7a60: cbz      w8, #0x1d7af0
001d7a64: adrp     x8, #0x279000
001d7a68: str      x19, [x8, #0x5b0]
001d7a6c: bl       #0x26c530
001d7a70: mov      x23, #0x34db
001d7a74: adrp     x8, #0x279000
001d7a78: movk     x23, #0xd7b6, lsl #16
001d7a7c: movk     x23, #0xde82, lsl #32
001d7a80: movk     x23, #0x431b, lsl #48
001d7a84: smulh    x9, x0, x23
001d7a88: asr      x10, x9, #0x12
001d7a8c: add      x22, x10, x9, lsr #63
001d7a90: ldr      x9, [x8, #0x5b8]
001d7a94: sub      x9, x22, x9
001d7a98: cmp      x9, #0xfa
001d7a9c: b.lo     #0x1d7c14
001d7aa0: nop      
001d7aa4: adr      x1, #0x279660
001d7aa8: mov      w0, #1
001d7aac: str      x22, [x8, #0x5b8]
001d7ab0: bl       #0x268190
001d7ab4: adrp     x8, #0x279000
001d7ab8: adrp     x24, #0x279000
001d7abc: ldr      x8, [x8, #0x4f0]
001d7ac0: cbz      x8, #0x1d7b60
001d7ac4: mov      w9, #0x9cb0
001d7ac8: mov      x0, xzr
001d7acc: movk     w9, #0x8c6, lsl #16
001d7ad0: add      x8, x8, x9
001d7ad4: blr      x8
001d7ad8: mov      w8, #0x270f
001d7adc: sub      w9, w0, #1
001d7ae0: str      w0, [x24, #8]
001d7ae4: cmp      w9, w8
001d7ae8: csinv    w26, w0, wzr, lo
001d7aec: b        #0x1d7b64
001d7af0: ldr      x8, [x21, #0x4f8]
001d7af4: ldr      x8, [x8, #8]
001d7af8: cbz      x8, #0x1d7df8
001d7afc: adrp     x8, #0x279000
001d7b00: mov      w9, #0xae24
001d7b04: mov      w10, #1
001d7b08: ldr      x8, [x8, #0x4f0]
001d7b0c: movk     w9, #0x8e8, lsl #16
001d7b10: mov      x0, x19
001d7b14: strb     w10, [x19, #0xd8]
001d7b18: add      x8, x8, x9
001d7b1c: blr      x8
001d7b20: ldr      x8, [x21, #0x4f8]
001d7b24: ldrb     w9, [x19, #0x79]
001d7b28: ldr      x8, [x8, #8]
001d7b2c: ldrb     w8, [x8, #0x79]
001d7b30: cmp      w9, w8
001d7b34: b.eq     #0x1d7df8
001d7b38: adrp     x8, #0x279000
001d7b3c: ldr      x0, [x8, #0x538]
001d7b40: mov      x1, x19
001d7b44: ldp      x20, x19, [sp, #0x50]
001d7b48: ldp      x22, x21, [sp, #0x40]
001d7b4c: ldp      x24, x23, [sp, #0x30]
001d7b50: ldp      x26, x25, [sp, #0x20]
001d7b54: ldp      x29, x30, [sp, #0x10]
001d7b58: ldr      d8, [sp], #0x60
001d7b5c: b        #0x1ea560
001d7b60: mov      w26, #-1
001d7b64: ldr      x0, [x19, #0x160]
001d7b68: bl       #0x1d8928
001d7b6c: mov      w20, w0
001d7b70: adrp     x8, #0x279000
001d7b74: cmp      w26, #0
001d7b78: adrp     x25, #0x279000
001d7b7c: str      w0, [x8, #0xc]
001d7b80: b.le     #0x1d7b8c
001d7b84: mov      w8, #1
001d7b88: b        #0x1d7ba4
001d7b8c: cmp      w20, #1
001d7b90: b.lt     #0x1d7bcc
001d7b94: mov      w8, #0x270f
001d7b98: cmp      w20, w8
001d7b9c: csel     w26, w20, w8, lo
001d7ba0: mov      w8, #2
001d7ba4: adrp     x9, #0x279000
001d7ba8: str      w26, [x25, #0x10]
001d7bac: str      w8, [x9, #0x668]
001d7bb0: bl       #0x26c530
001d7bb4: smulh    x8, x0, x23
001d7bb8: asr      x9, x8, #0x12
001d7bbc: add      x8, x9, x8, lsr #63
001d7bc0: nop      
001d7bc4: adr      x9, #0x279670
001d7bc8: stlr     x8, [x9]
001d7bcc: adrp     x8, #0x279000
001d7bd0: ldr      x9, [x8, #0x5c0]
001d7bd4: sub      x9, x22, x9
001d7bd8: lsr      x9, x9, #3
001d7bdc: cmp      x9, #0x271
001d7be0: b.lo     #0x1d7c14
001d7be4: str      x22, [x8, #0x5c0]
001d7be8: adrp     x8, #0x279000
001d7bec: adrp     x1, #0x35000
001d7bf0: add      x1, x1, #0x1fd
001d7bf4: ldr      w3, [x24, #8]
001d7bf8: ldr      w5, [x25, #0x10]
001d7bfc: ldr      x6, [x8, #0x660]
001d7c00: adrp     x2, #0x36000
001d7c04: add      x2, x2, #0x337
001d7c08: mov      w0, #4
001d7c0c: mov      w4, w20
001d7c10: bl       #0x26c540
001d7c14: mov      x0, x19
001d7c18: bl       #0x1d9b94
001d7c1c: movi     d0, #0000000000000000
001d7c20: tbz      w0, #0, #0x1d7c44
001d7c24: ldr      x8, [x19, #0x88]
001d7c28: cbz      x8, #0x1d7c4c
001d7c2c: ldr      x8, [x8, #0xa0]
001d7c30: cbz      x8, #0x1d7c4c
001d7c34: ldr      x8, [x8, #0x168]
001d7c38: cbz      x8, #0x1d7c4c
001d7c3c: ldr      s0, [x8, #0x1a4]
001d7c40: scvtf    s0, s0
001d7c44: adrp     x8, #0x279000
001d7c48: str      s0, [x8, #0x5c8]
001d7c4c: adrp     x8, #0x279000
001d7c50: ldrb     w8, [x8, #0x5a1]
001d7c54: cbz      w8, #0x1d7c6c
001d7c58: ldr      x8, [x21, #0x4f8]
001d7c5c: ldr      x0, [x8, #8]
001d7c60: cbz      x0, #0x1d7c6c
001d7c64: bl       #0x1d93dc
001d7c68: b        #0x1d7c74
001d7c6c: adrp     x8, #0x279000
001d7c70: strb     wzr, [x8, #0x5cc]
001d7c74: mov      x0, x19
001d7c78: bl       #0x1d9b94
001d7c7c: tbz      w0, #0, #0x1d7d24
001d7c80: ldr      x8, [x19, #0x88]
001d7c84: cbz      x8, #0x1d7d24
001d7c88: ldr      x21, [x8, #0xa0]
001d7c8c: cbz      x21, #0x1d7d24
001d7c90: adrp     x8, #0x279000
001d7c94: ldrb     w8, [x8, #0x5cd]
001d7c98: cbz      w8, #0x1d7cb4
001d7c9c: add      x0, x21, #0x120
001d7ca0: mov      w1, #0x45
001d7ca4: bl       #0x1d9c30
001d7ca8: add      x0, x21, #0x128
001d7cac: mov      w1, #0x45
001d7cb0: bl       #0x1d9c30
001d7cb4: adrp     x8, #0x279000
001d7cb8: ldr      x20, [x21, #0x168]
001d7cbc: ldrb     w8, [x8, #0x5ce]
001d7cc0: cbz      w8, #0x1d7cd4
001d7cc4: adrp     x8, #0x37000
001d7cc8: add      x0, x21, #0x108
001d7ccc: ldr      s0, [x8, #0xf4c]
001d7cd0: bl       #0x1d9c54
001d7cd4: cbz      x20, #0x1d7d24
001d7cd8: ldr      s0, [x20, #0x1a4]
001d7cdc: adrp     x8, #0x279000
001d7ce0: adrp     x9, #0x279000
001d7ce4: ldrb     w8, [x8, #0x5cf]
001d7ce8: scvtf    s0, s0
001d7cec: str      s0, [x9, #0x5c8]
001d7cf0: cbz      w8, #0x1d7d24
001d7cf4: adrp     x8, #0x37000
001d7cf8: add      x0, x20, #0x2dc
001d7cfc: ldr      s8, [x8, #0xf50]
001d7d00: fmov     s0, s8
001d7d04: bl       #0x1d9c00
001d7d08: mov      w1, #0x869f
001d7d0c: add      x0, x20, #0x264
001d7d10: movk     w1, #1, lsl #16
001d7d14: bl       #0x1d9bd4
001d7d18: fmov     s0, s8
001d7d1c: add      x0, x20, #0x258
001d7d20: bl       #0x1d9c00
001d7d24: adrp     x8, #0x279000
001d7d28: ldrb     w8, [x8, #0x5d0]
001d7d2c: cbz      w8, #0x1d7d74
001d7d30: ldr      x9, [x19, #0x98]
001d7d34: cbz      x9, #0x1d7d74
001d7d38: ldr      x8, [x9, #0xb0]
001d7d3c: cbz      x8, #0x1d7d74
001d7d40: ldr      x10, [x9, #0xa8]
001d7d44: cbz      x10, #0x1d7d74
001d7d48: ldr      x9, [x10, #0x50]
001d7d4c: cbz      x9, #0x1d7d74
001d7d50: mov      w11, #0x3c00
001d7d54: movk     w11, #0x461c, lsl #16
001d7d58: dup      v0.2s, w11
001d7d5c: stur     d0, [x10, #0x2c]
001d7d60: mov      w10, #0x999a
001d7d64: movk     w10, #0x40d9, lsl #16
001d7d68: str      xzr, [x8, #0x68]
001d7d6c: str      wzr, [x8, #0x70]
001d7d70: str      w10, [x9, #0x60]
001d7d74: adrp     x8, #0x279000
001d7d78: ldrb     w8, [x8, #0x5d2]
001d7d7c: cbz      w8, #0x1d7dbc
001d7d80: ldr      x8, [x19, #0xe8]
001d7d84: cbz      x8, #0x1d7dbc
001d7d88: ldr      x8, [x8, #0x28]
001d7d8c: cbz      x8, #0x1d7dbc
001d7d90: ldr      x0, [x8, #0x30]
001d7d94: cbz      x0, #0x1d7dbc
001d7d98: adrp     x8, #0x279000
001d7d9c: mov      w9, #0x7bc0
001d7da0: mov      x1, xzr
001d7da4: ldr      x8, [x8, #0x4f0]
001d7da8: movk     w9, #0x5fb, lsl #16
001d7dac: add      x8, x8, x9
001d7db0: adrp     x9, #0x37000
001d7db4: ldr      s0, [x9, #0xe90]
001d7db8: blr      x8
001d7dbc: mov      x0, x19
001d7dc0: bl       #0x1d9e00
001d7dc4: adrp     x8, #0x279000
001d7dc8: ldrb     w8, [x8, #0x5d4]
001d7dcc: cbz      w8, #0x1d7df8
001d7dd0: adrp     x8, #0x279000
001d7dd4: ldr      x0, [x8, #0x538]
001d7dd8: mov      x1, x19
001d7ddc: ldp      x20, x19, [sp, #0x50]
001d7de0: ldp      x22, x21, [sp, #0x40]
001d7de4: ldp      x24, x23, [sp, #0x30]
001d7de8: ldp      x26, x25, [sp, #0x20]
001d7dec: ldp      x29, x30, [sp, #0x10]
001d7df0: ldr      d8, [sp], #0x60
001d7df4: b        #0x1ea940
001d7df8: ldp      x20, x19, [sp, #0x50]
001d7dfc: ldp      x22, x21, [sp, #0x40]
001d7e00: ldp      x24, x23, [sp, #0x30]
001d7e04: ldp      x26, x25, [sp, #0x20]
001d7e08: ldp      x29, x30, [sp, #0x10]
001d7e0c: ldr      d8, [sp], #0x60
001d7e10: ret      
001d7e14: stp      x29, x30, [sp, #-0x20]!
001d7e18: stp      x20, x19, [sp, #0x10]
001d7e1c: mov      x29, sp
001d7e20: mov      x19, x1
001d7e24: mov      x20, x0
001d7e28: cbz      x1, #0x1d7e98
001d7e2c: adrp     x8, #0x279000
001d7e30: ldr      x8, [x8, #0x4f8]
001d7e34: ldr      x1, [x8, #8]
001d7e38: cbz      x1, #0x1d7e98
001d7e3c: adrp     x8, #0x279000
001d7e40: ldr      w8, [x8, #0x5e8]
001d7e44: cbnz     w8, #0x1d7e74
001d7e48: adrp     x8, #0x279000
001d7e4c: ldrb     w8, [x8, #0x5a1]
001d7e50: cbz      w8, #0x1d7e74
001d7e54: adrp     x8, #0x279000
001d7e58: ldrb     w8, [x8, #0x5ec]
001d7e5c: cbz      w8, #0x1d7e74
001d7e60: adrp     x8, #0x279000
001d7e64: ldrb     w8, [x8, #0x5cc]
001d7e68: cbz      w8, #0x1d7e74
001d7e6c: mov      w8, #1
001d7e70: strb     w8, [x19, #0x21]
001d7e74: adrp     x8, #0x279000
001d7e78: mov      x2, x19
001d7e7c: ldr      x0, [x8, #0x500]
001d7e80: bl       #0x1df078
001d7e84: adrp     x8, #0x279000
001d7e88: mov      x0, x20
001d7e8c: ldr      x2, [x8, #0x5e0]
001d7e90: mov      x1, x19
001d7e94: b        #0x1d7eb8
001d7e98: adrp     x8, #0x279000
001d7e9c: adrp     x9, #0x279000
001d7ea0: mov      x0, x20
001d7ea4: ldr      x8, [x8, #0x500]
001d7ea8: mov      x1, x19
001d7eac: str      xzr, [x8]
001d7eb0: ldr      x2, [x9, #0x5e0]
001d7eb4: str      wzr, [x8, #8]
001d7eb8: ldp      x20, x19, [sp, #0x10]
001d7ebc: ldp      x29, x30, [sp], #0x20
```
