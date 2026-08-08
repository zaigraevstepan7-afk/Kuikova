0x001d7ec4:  sub      sp, sp, #0xa0
0x001d7ec8:  stp      d9, d8, [sp, #0x30]
0x001d7ecc:  stp      x29, x30, [sp, #0x40]
0x001d7ed0:  stp      x28, x27, [sp, #0x50]
0x001d7ed4:  stp      x26, x25, [sp, #0x60]
0x001d7ed8:  stp      x24, x23, [sp, #0x70]
0x001d7edc:  stp      x22, x21, [sp, #0x80]
0x001d7ee0:  stp      x20, x19, [sp, #0x90]
0x001d7ee4:  add      x29, sp, #0x40
0x001d7ee8:  mrs      x26, tpidr_el0
0x001d7eec:  mov      x19, x0
0x001d7ef0:  ldr      x8, [x26, #0x28]
0x001d7ef4:  stur     x8, [x29, #-0x18]
0x001d7ef8:  adrp     x8, #0x279000
0x001d7efc:  ldr      x8, [x8, #0x5f0]
0x001d7f00:  cbz      x8, #0x1d7f0c
0x001d7f04:  mov      x0, x19
0x001d7f08:  blr      x8
0x001d7f0c:  adrp     x8, #0x279000
0x001d7f10:  ldr      x8, [x8, #0x4f8]
0x001d7f14:  ldr      x8, [x8, #8]
0x001d7f18:  cmp      x8, x19
0x001d7f1c:  b.eq     #0x1d7f54
0x001d7f20:  ldr      x8, [x26, #0x28]
0x001d7f24:  ldur     x9, [x29, #-0x18]
0x001d7f28:  cmp      x8, x9
0x001d7f2c:  b.ne     #0x1d81b8
0x001d7f30:  ldp      x20, x19, [sp, #0x90]
0x001d7f34:  ldp      x22, x21, [sp, #0x80]
0x001d7f38:  ldp      x24, x23, [sp, #0x70]
0x001d7f3c:  ldp      x26, x25, [sp, #0x60]
0x001d7f40:  ldp      x28, x27, [sp, #0x50]
0x001d7f44:  ldp      x29, x30, [sp, #0x40]
0x001d7f48:  ldp      d9, d8, [sp, #0x30]
0x001d7f4c:  add      sp, sp, #0xa0
0x001d7f50:  ret      
