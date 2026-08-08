0x001d760c:  stp      x29, x30, [sp, #-0x30]!
0x001d7610:  str      x21, [sp, #0x10]
0x001d7614:  stp      x20, x19, [sp, #0x20]
0x001d7618:  mov      x29, sp
0x001d761c:  adrp     x8, #0x279000
0x001d7620:  mov      x20, x5
0x001d7624:  ldr      x8, [x8, #0x580]
0x001d7628:  blr      x8
0x001d762c:  mov      x19, x0
0x001d7630:  cbz      x20, #0x1d76c4
0x001d7634:  cbnz     x19, #0x1d76c4
0x001d7638:  ldr      x20, [x20]
0x001d763c:  cbz      x20, #0x1d76c4
0x001d7640:  mov      x0, x20
0x001d7644:  bl       #0x26c490
0x001d7648:  cmp      w0, #2
0x001d764c:  b.ne     #0x1d76c4
0x001d7650:  mov      x0, x20
0x001d7654:  bl       #0x26c4a0
0x001d7658:  mov      w21, w0
0x001d765c:  nop      
0x001d7660:  adr      x0, #0x27954c
0x001d7664:  bl       #0x26c4b0
0x001d7668:  mov      x0, x20
0x001d766c:  mov      x1, xzr
0x001d7670:  bl       #0x26c4c0
0x001d7674:  adrp     x8, #0x279000
0x001d7678:  str      s0, [x8, #0x650]
0x001d767c:  mov      x0, x20
0x001d7680:  mov      x1, xzr
0x001d7684:  bl       #0x26c4d0
0x001d7688:  movz     w8, #0xfd
0x001d768c:  adrp     x9, #0x279000
0x001d7690:  ands     w8, w21, w8
0x001d7694:  str      s0, [x9, #0x654]
0x001d7698:  b.eq     #0x1d76ac
0x001d769c:  cmp      w8, #1
0x001d76a0:  b.ne     #0x1d76b8
0x001d76a4:  mov      w8, wzr
0x001d76a8:  b        #0x1d76b0
0x001d76ac:  movz     w8, #0x1
0x001d76b0:  adrp     x9, #0x279000
0x001d76b4:  strb     w8, [x9, #0x658]
0x001d76b8:  nop      
0x001d76bc:  adr      x0, #0x27954c
0x001d76c0:  bl       #0x26c4e0
0x001d76c4:  mov      x0, x19
0x001d76c8:  ldp      x20, x19, [sp, #0x20]
0x001d76cc:  ldr      x21, [sp, #0x10]
0x001d76d0:  ldp      x29, x30, [sp], #0x30
0x001d76d4:  ret      
