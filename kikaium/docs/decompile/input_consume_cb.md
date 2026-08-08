# input_consume_cb

- VA: `0x1d760c`
- Size scanned: `0xe0`
- Pass A instructions: 56
- Method: **2-command decompile** — (1) Capstone Pass A full disasm (2) Capstone Pass B field/RVA/string recovery

## Pass B — field LDR/STR map

| Offset | Meaning | Count |
|---|---|---|
| `0x10` | unknown | 2 |

## Pass B — game RVAs (mov+movk)

_none_

## Pass B — strings (ADRP+ADD resolved only)

_none_

## Pass B — calls (first 40)

- `0x1d7628` → `x8` (blr)
- `0x1d7644` → `0x26c490` (bl)
- `0x1d7654` → `0x26c4a0` (bl)
- `0x1d7664` → `0x26c4b0` (bl)
- `0x1d7670` → `0x26c4c0` (bl)
- `0x1d7684` → `0x26c4d0` (bl)
- `0x1d76c0` → `0x26c4e0` (bl)
- `0x1d76e4` → `0x26c4e0` (bl)

## Pass A — full disasm

```
001d760c: stp      x29, x30, [sp, #-0x30]!
001d7610: str      x21, [sp, #0x10]
001d7614: stp      x20, x19, [sp, #0x20]
001d7618: mov      x29, sp
001d761c: adrp     x8, #0x279000
001d7620: mov      x20, x5
001d7624: ldr      x8, [x8, #0x580]
001d7628: blr      x8
001d762c: mov      x19, x0
001d7630: cbz      x20, #0x1d76c4
001d7634: cbnz     x19, #0x1d76c4
001d7638: ldr      x20, [x20]
001d763c: cbz      x20, #0x1d76c4
001d7640: mov      x0, x20
001d7644: bl       #0x26c490
001d7648: cmp      w0, #2
001d764c: b.ne     #0x1d76c4
001d7650: mov      x0, x20
001d7654: bl       #0x26c4a0
001d7658: mov      w21, w0
001d765c: nop      
001d7660: adr      x0, #0x27954c
001d7664: bl       #0x26c4b0
001d7668: mov      x0, x20
001d766c: mov      x1, xzr
001d7670: bl       #0x26c4c0
001d7674: adrp     x8, #0x279000
001d7678: str      s0, [x8, #0x650]
001d767c: mov      x0, x20
001d7680: mov      x1, xzr
001d7684: bl       #0x26c4d0
001d7688: mov      w8, #0xfd
001d768c: adrp     x9, #0x279000
001d7690: ands     w8, w21, w8
001d7694: str      s0, [x9, #0x654]
001d7698: b.eq     #0x1d76ac
001d769c: cmp      w8, #1
001d76a0: b.ne     #0x1d76b8
001d76a4: mov      w8, wzr
001d76a8: b        #0x1d76b0
001d76ac: mov      w8, #1
001d76b0: adrp     x9, #0x279000
001d76b4: strb     w8, [x9, #0x658]
001d76b8: nop      
001d76bc: adr      x0, #0x27954c
001d76c0: bl       #0x26c4e0
001d76c4: mov      x0, x19
001d76c8: ldp      x20, x19, [sp, #0x20]
001d76cc: ldr      x21, [sp, #0x10]
001d76d0: ldp      x29, x30, [sp], #0x30
001d76d4: ret      
001d76d8: mov      x19, x0
001d76dc: nop      
001d76e0: adr      x0, #0x27954c
001d76e4: bl       #0x26c4e0
001d76e8: mov      x0, x19
```
