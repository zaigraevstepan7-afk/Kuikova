# wm_click_site

- VA: `0x1db464`
- Size scanned: `0x200`
- Pass A instructions: 128
- Method: **2-command decompile** — (1) Capstone Pass A full disasm (2) Capstone Pass B field/RVA/string recovery

## Pass B — field LDR/STR map

| Offset | Meaning | Count |
|---|---|---|
| `0x10` | unknown | 1 |
| `0x14` | unknown | 2 |
| `0x18` | unknown | 1 |
| `0x20` | unknown | 1 |
| `0x28` | players_list / camera holder | 2 |
| `0x30` | Photon.isLocal (ldrb) / nested | 2 |
| `0x64` | unknown | 2 |
| `0xce` | unknown | 1 |
| `0x2c8` | unknown | 1 |

## Pass B — game RVAs (mov+movk)

_none_

## Pass B — strings (ADRP+ADD resolved only)

- `##wm_click`

## Pass B — calls (first 40)

- `0x1db4c0` → `0x1f17e0` (bl)
- `0x1db4d8` → `0x21544c` (bl)
- `0x1db50c` → `0x2089a0` (bl)
- `0x1db5c0` → `0x2088d0` (bl)
- `0x1db61c` → `0x209068` (bl)
- `0x1db65c` → `0x209124` (bl)

## Pass A — full disasm

```
001db464: adrp     x1, #0x2f000
001db468: add      x1, x1, #0x52d
001db46c: ldr      x8, [x8, #0x4a0]
001db470: mov      x2, xzr
001db474: strb     wzr, [sp, #0x14]
001db478: strb     wzr, [sp, #0x10]
001db47c: ldr      x8, [x8]
001db480: ldr      x0, [x8, #0x1480]
001db484: ldr      x8, [x0, #0x28]
001db488: ldp      s0, s1, [x0, #0x30]
001db48c: strb     w19, [x0, #0xce]
001db490: ldr      x19, [x0, #0x2c8]
001db494: fmov     s2, w8
001db498: lsr      x9, x8, #0x20
001db49c: str      x8, [sp, #0x30]
001db4a0: str      x8, [sp, #0x18]
001db4a4: fadd     s0, s0, s2
001db4a8: fmov     s2, w9
001db4ac: fadd     s3, s1, s2
001db4b0: fmadd    s15, s1, s13, s2
001db4b4: stp      s0, s3, [sp, #0x28]
001db4b8: ldr      x8, [sp, #0x28]
001db4bc: str      x8, [sp, #0x20]
001db4c0: bl       #0x1f17e0
001db4c4: mov      w1, w0
001db4c8: add      x0, sp, #0x18
001db4cc: add      x2, sp, #0x14
001db4d0: add      x3, sp, #0x10
001db4d4: mov      w4, #0x10
001db4d8: bl       #0x21544c
001db4dc: tbz      w0, #0, #0x1db4f0
001db4e0: adrp     x8, #0x279000
001db4e4: ldrb     w9, [x8, #0x64]
001db4e8: eor      w9, w9, #1
001db4ec: strb     w9, [x8, #0x64]
001db4f0: fmov     s0, s10
001db4f4: mov      w3, #0xb0b
001db4f8: add      x1, sp, #0x30
001db4fc: add      x2, sp, #0x28
001db500: mov      x0, x19
001db504: movk     w3, #0xd60d, lsl #16
001db508: mov      w4, wzr
001db50c: bl       #0x2089a0
001db510: fmov     v0.2s, #1.00000000
001db514: nop      
001db518: adr      x8, #0x279050
001db51c: ldur     d1, [x8, #4]
001db520: mov      w10, #0x437f0000
001db524: ldrb     w9, [sp, #0x14]
001db528: dup      v4.2s, w10
001db52c: movi     v5.2s, #0x3f, lsl #24
001db530: add      x1, sp, #0x30
001db534: fcmlt    v3.2s, v1.2s, #0.0
001db538: cmp      w9, #0
001db53c: adrp     x9, #0x78000
001db540: add      x9, x9, #0xe08
001db544: fcmgt    v2.2s, v1.2s, v0.2s
001db548: add      x2, sp, #0x28
001db54c: mov      x0, x19
001db550: mov      w4, wzr
001db554: bif      v0.8b, v1.8b, v2.8b
001db558: fmov     s1, #1.00000000
001db55c: ldr      s2, [x8]
001db560: cset     w8, eq
001db564: fcmp     s2, s1
001db568: bic      v0.8b, v0.8b, v3.8b
001db56c: fmov     s3, w10
001db570: fmla     v5.2s, v4.2s, v0.2s
001db574: fcsel    s0, s1, s2, gt
001db578: fcmp     s2, #0.0
001db57c: ldr      s2, [x9, w8, uxtw #2]
001db580: adrp     x8, #0x78000
001db584: ldr      d4, [x8, #0xd10]
001db588: fmadd    s2, s2, s3, s13
001db58c: fcvtzs   v1.2s, v5.2s
001db590: fcsel    s0, s9, s0, mi
001db594: fcvtzs   w8, s2
001db598: ushl     v1.2s, v1.2s, v4.2s
001db59c: fmadd    s0, s0, s3, s13
001db5a0: mov      w9, v1.s[1]
001db5a4: fcvtzs   w10, s0
001db5a8: fmov     s0, s10
001db5ac: orr      w8, w9, w8, lsl #24
001db5b0: fmov     w9, s1
001db5b4: fmov     s1, s8
001db5b8: orr      w9, w9, w10
001db5bc: orr      w3, w8, w9
001db5c0: bl       #0x2088d0
001db5c4: ldr      s0, [sp, #0x30]
001db5c8: add      x8, sp, #0x40
001db5cc: mov      w24, #0xe8e8
001db5d0: mov      w25, #0x7e7e
001db5d4: mov      x21, xzr
001db5d8: add      x22, x8, #0x18
001db5dc: fadd     s10, s14, s0
001db5e0: ldp      s2, s0, [sp]
001db5e4: movk     w24, #0xffec, lsl #16
001db5e8: movk     w25, #0xff86, lsl #16
001db5ec: fmul     s8, s2, s13
001db5f0: fadd     s14, s2, s11
001db5f4: fmsub    s12, s0, s13, s15
001db5f8: cbz      x21, #0x1db624
001db5fc: ldr      s0, [sp]
001db600: add      x1, sp, #8
001db604: mov      x0, x19
001db608: mov      w2, #0x26ffffff
001db60c: mov      w3, #6
001db610: fmadd    s0, s0, s13, s10
001db614: stp      s0, s15, [sp, #8]
001db618: fmov     s0, s8
001db61c: bl       #0x209068
001db620: fadd     s10, s14, s10
001db624: ldp      s0, s9, [x22, #-8]
001db628: movi     d1, #0000000000000000
001db62c: ldrb     w8, [x22]
001db630: ldp      x4, x1, [x22, #-0x18]
001db634: add      x2, sp, #8
001db638: mov      x0, x19
001db63c: mov      x5, xzr
001db640: fsub     s0, s9, s0
001db644: cmp      w8, #0
001db648: mov      x6, xzr
001db64c: csel     w3, w25, w24, eq
001db650: fmadd    s0, s0, s13, s10
001db654: stp      s0, s12, [sp, #8]
001db658: ldr      s0, [sp, #4]
001db65c: bl       #0x209124
001db660: fadd     s0, s11, s9
```
