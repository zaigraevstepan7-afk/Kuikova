0x0023c5d8:  str      x30, [sp, #-0x20]!
0x0023c5dc:  stp      x20, x19, [sp, #0x10]
0x0023c5e0:  mov      x20, x0
0x0023c5e4:  mov      x0, xzr
0x0023c5e8:  mov      x19, x1
0x0023c5ec:  bl       #0x26ccf0
0x0023c5f0:  cbz      x0, #0x23c600
0x0023c5f4:  ldp      x20, x19, [sp, #0x10]
0x0023c5f8:  ldr      x30, [sp], #0x20
0x0023c5fc:  ret      
0x0023c600:  mov      x0, x20
0x0023c604:  mov      x1, x19
0x0023c608:  ldp      x20, x19, [sp, #0x10]
0x0023c60c:  ldr      x30, [sp], #0x20
0x0023c610:  b        #0x23bfa8
0x0023c614:  ldr      x0, [x0]
