0x001d69e4:  sub      sp, sp, #0x50
0x001d69e8:  stp      x29, x30, [sp, #0x20]
0x001d69ec:  str      x21, [sp, #0x30]
0x001d69f0:  stp      x20, x19, [sp, #0x40]
0x001d69f4:  add      x29, sp, #0x20
0x001d69f8:  mrs      x21, tpidr_el0
0x001d69fc:  movz     w0, #0x8
0x001d6a00:  ldr      x8, [x21, #0x28]
0x001d6a04:  stur     x8, [x29, #-8]
0x001d6a08:  bl       #0x26c330
0x001d6a0c:  mov      x20, x0
0x001d6a10:  bl       #0x26c340
0x001d6a14:  movz     w0, #0x8
0x001d6a18:  bl       #0x26c330
0x001d6a1c:  mov      x3, x0
0x001d6a20:  str      x20, [x0]
0x001d6a24:  str      x0, [sp, #0x10]
0x001d6a28:  nop      
0x001d6a2c:  adr      x2, #0x1d6afc
0x001d6a30:  add      x0, sp, #8
0x001d6a34:  mov      x1, xzr
0x001d6a38:  bl       #0x26c350
0x001d6a3c:  cbnz     w0, #0x1d6a88
0x001d6a40:  add      x0, sp, #0x10
0x001d6a44:  str      xzr, [sp, #0x10]
0x001d6a48:  bl       #0x1d6d04
0x001d6a4c:  add      x0, sp, #8
0x001d6a50:  bl       #0x26c360
0x001d6a54:  add      x0, sp, #8
0x001d6a58:  bl       #0x26c370
0x001d6a5c:  ldr      x8, [x21, #0x28]
0x001d6a60:  ldur     x9, [x29, #-8]
0x001d6a64:  cmp      x8, x9
0x001d6a68:  b.ne     #0x1d6af8
0x001d6a6c:  movz     w0, #0x6
0x001d6a70:  movk     w0, #0x1, lsl #16
0x001d6a74:  ldp      x20, x19, [sp, #0x40]
0x001d6a78:  ldr      x21, [sp, #0x30]
0x001d6a7c:  ldp      x29, x30, [sp, #0x20]
0x001d6a80:  add      sp, sp, #0x50
0x001d6a84:  ret      
0x001d6a88:  ldr      x8, [x21, #0x28]
0x001d6a8c:  ldur     x9, [x29, #-8]
0x001d6a90:  cmp      x8, x9
0x001d6a94:  b.ne     #0x1d6af8
0x001d6a98:  adrp     x1, #0x2e000
0x001d6a9c:  add      x1, x1, #0x8af
0x001d6aa0:  bl       #0x26c380
0x001d6aa4:  mov      x19, x0
0x001d6aa8:  add      x0, sp, #8
0x001d6aac:  bl       #0x26c370
0x001d6ab0:  b        #0x1d6ae0
0x001d6ab4:  mov      x19, x0
0x001d6ab8:  mov      x0, x20
0x001d6abc:  bl       #0x26c390
0x001d6ac0:  b        #0x1d6ac8
0x001d6ac4:  mov      x19, x0
0x001d6ac8:  mov      x0, x20
0x001d6acc:  bl       #0x26c3a0
0x001d6ad0:  b        #0x1d6ae0
0x001d6ad4:  mov      x19, x0
0x001d6ad8:  add      x0, sp, #0x10
0x001d6adc:  bl       #0x1d6d04
0x001d6ae0:  ldr      x8, [x21, #0x28]
0x001d6ae4:  ldur     x9, [x29, #-8]
0x001d6ae8:  cmp      x8, x9
0x001d6aec:  b.ne     #0x1d6af8
0x001d6af0:  mov      x0, x19
0x001d6af4:  bl       #0x26841c
0x001d6af8:  bl       #0x26c3b0
