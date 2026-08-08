# extraB_cb

- VA: `0x1d83cc`
- Size scanned: `0x40`
- Pass A instructions: 16
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

- `0x1d83e4` → `x8` (blr)

## Pass A — full disasm

```
001d83cc: stp      x29, x30, [sp, #-0x20]!
001d83d0: str      x19, [sp, #0x10]
001d83d4: mov      x29, sp
001d83d8: adrp     x8, #0x279000
001d83dc: mov      x19, x1
001d83e0: ldr      x8, [x8, #0x610]
001d83e4: blr      x8
001d83e8: adrp     x8, #0x279000
001d83ec: ldrb     w8, [x8, #0x618]
001d83f0: cbz      w8, #0x1d83f8
001d83f4: str      xzr, [x19]
001d83f8: ldr      x19, [sp, #0x10]
001d83fc: ldp      x29, x30, [sp], #0x20
001d8400: ret      
001d8404: adrp     x8, #0x279000
001d8408: ldrb     w8, [x8, #0x5a1]
```
