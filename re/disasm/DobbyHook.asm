0x002389d0:  cbz      x0, #0x238a34
0x002389d4:  str      x30, [sp, #-0x30]!
0x002389d8:  stp      x22, x21, [sp, #0x10]
0x002389dc:  stp      x20, x19, [sp, #0x20]
0x002389e0:  mov      x20, x0
0x002389e4:  mov      x22, x1
0x002389e8:  mov      x21, x2
0x002389ec:  bl       #0x23bc80
0x002389f0:  mov      x8, xzr
0x002389f4:  sub      x19, x8, w0, sxtw
0x002389f8:  bl       #0x23bc80
0x002389fc:  sxtw     x1, w0
0x00238a00:  movz     w2, #0x4
0x00238a04:  and      x0, x19, x20
0x00238a08:  bl       #0x23bccc
0x00238a0c:  tbz      w0, #0, #0x238a20
0x00238a10:  bl       #0x23b6cc
0x00238a14:  mov      x1, x20
0x00238a18:  bl       #0x23b700
0x00238a1c:  cbz      x0, #0x238a3c
0x00238a20:  movn     w0, #0
0x00238a24:  ldp      x20, x19, [sp, #0x20]
0x00238a28:  ldp      x22, x21, [sp, #0x10]
0x00238a2c:  ldr      x30, [sp], #0x30
0x00238a30:  ret      
0x00238a34:  movn     w0, #0
0x00238a38:  ret      
0x00238a3c:  movz     w0, #0x138
0x00238a40:  bl       #0x26c330
0x00238a44:  mov      w1, wzr
0x00238a48:  mov      x2, x20
0x00238a4c:  mov      x19, x0
0x00238a50:  bl       #0x23bc5c
0x00238a54:  movz     w0, #0x40
0x00238a58:  bl       #0x26c330
0x00238a5c:  adrp     x8, #0x277000
0x00238a60:  movi     v0.2d, #0000000000000000
0x00238a64:  mov      x20, x0
0x00238a68:  ldr      x8, [x8, #0x4f0]
0x00238a6c:  str      x0, [x19, #8]
0x00238a70:  stp      xzr, x22, [x0, #0x30]
0x00238a74:  add      x9, x8, #0x10
0x00238a78:  ldr      x8, [x8, #0x18]
0x00238a7c:  stp      x9, x19, [x0]
0x00238a80:  stp      q0, q0, [x0, #0x10]
0x00238a84:  blr      x8
0x00238a88:  ldr      x8, [x20]
0x00238a8c:  mov      x0, x20
0x00238a90:  ldr      x8, [x8]
0x00238a94:  blr      x8
0x00238a98:  cbz      x21, #0x238aa4
0x00238a9c:  ldr      x8, [x19, #0x20]
0x00238aa0:  str      x8, [x21]
0x00238aa4:  mov      x0, x20
0x00238aa8:  bl       #0x238db8
0x00238aac:  bl       #0x23b6cc
0x00238ab0:  mov      x1, x19
0x00238ab4:  bl       #0x23b730
0x00238ab8:  mov      w0, wzr
0x00238abc:  ldp      x20, x19, [sp, #0x20]
0x00238ac0:  ldp      x22, x21, [sp, #0x10]
0x00238ac4:  ldr      x30, [sp], #0x30
0x00238ac8:  ret      
