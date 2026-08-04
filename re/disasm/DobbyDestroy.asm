0x0023b680:  stp      x30, x19, [sp, #-0x10]!
0x0023b684:  mov      x19, x0
0x0023b688:  bl       #0x23b6cc
0x0023b68c:  mov      x1, x19
0x0023b690:  bl       #0x23b700
0x0023b694:  cbz      x0, #0x23b6c0
0x0023b698:  ldr      w2, [x0, #0x12c]
0x0023b69c:  add      x1, x0, #0x2c
0x0023b6a0:  mov      x0, x19
0x0023b6a4:  bl       #0x26cd60
0x0023b6a8:  bl       #0x23b6cc
0x0023b6ac:  mov      x1, x19
0x0023b6b0:  bl       #0x23bbd8
0x0023b6b4:  mov      w0, wzr
0x0023b6b8:  ldp      x30, x19, [sp], #0x10
0x0023b6bc:  ret      
0x0023b6c0:  movn     w0, #0
0x0023b6c4:  ldp      x30, x19, [sp], #0x10
0x0023b6c8:  ret      
