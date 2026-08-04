0x001d76f0:  sub      sp, sp, #0x80
0x001d76f4:  stp      d9, d8, [sp, #0x30]
0x001d76f8:  stp      x29, x30, [sp, #0x40]
0x001d76fc:  stp      x24, x23, [sp, #0x50]
0x001d7700:  stp      x22, x21, [sp, #0x60]
0x001d7704:  stp      x20, x19, [sp, #0x70]
0x001d7708:  add      x29, sp, #0x40
0x001d770c:  mrs      x22, tpidr_el0
0x001d7710:  nop      
0x001d7714:  adr      x21, #0x27965c
0x001d7718:  ldr      x8, [x22, #0x28]
0x001d771c:  mov      x19, x1
0x001d7720:  mov      x20, x0
0x001d7724:  stur     x8, [x29, #-0x18]
0x001d7728:  ldarb    w8, [x21]
0x001d772c:  tbnz     w8, #0, #0x1d7760
0x001d7730:  bl       #0x26c4f0
0x001d7734:  cbz      x0, #0x1d7760
0x001d7738:  mov      x0, xzr
0x001d773c:  bl       #0x1ef77c
0x001d7740:  adrp     x0, #0x2e000
0x001d7744:  add      x0, x0, #0xebe
0x001d7748:  bl       #0x22e93c
0x001d774c:  bl       #0x1f2744
0x001d7750:  str      xzr, [x0, #0x20]
0x001d7754:  bl       #0x1daaec
0x001d7758:  movz     w8, #0x1
0x001d775c:  stlrb    w8, [x21]
0x001d7760:  ldarb    w8, [x21]
0x001d7764:  tbz      w8, #0, #0x1d79a0
0x001d7768:  bl       #0x1f2744
0x001d776c:  mov      x21, x0
0x001d7770:  sub      x3, x29, #0x1c
0x001d7774:  mov      x0, x20
0x001d7778:  mov      x1, x19
0x001d777c:  movz     w2, #0x3057
0x001d7780:  bl       #0x26c500
0x001d7784:  add      x3, sp, #0x20
0x001d7788:  mov      x0, x20
0x001d778c:  mov      x1, x19
0x001d7790:  movz     w2, #0x3056
0x001d7794:  bl       #0x26c500
0x001d7798:  ldur     s0, [x29, #-0x1c]
0x001d779c:  ldr      s1, [sp, #0x20]
0x001d77a0:  add      x1, sp, #0x10
0x001d77a4:  movz     w0, #0x1
0x001d77a8:  stp      xzr, xzr, [sp, #0x10]
0x001d77ac:  scvtf    s0, s0
0x001d77b0:  scvtf    s1, s1
0x001d77b4:  stp      s0, s1, [x21, #8]
0x001d77b8:  bl       #0x26c510
0x001d77bc:  nop      
0x001d77c0:  adr      x8, #0x279588
0x001d77c4:  ldp      x9, x10, [x8]
0x001d77c8:  orr      x11, x9, x10
0x001d77cc:  cbz      x11, #0x1d780c
0x001d77d0:  ldp      x11, x12, [sp, #0x10]
0x001d77d4:  sub      x9, x11, x9
0x001d77d8:  sub      x10, x12, x10
0x001d77dc:  scvtf    s0, x9
0x001d77e0:  scvtf    s1, x10
0x001d77e4:  adrp     x9, #0x37000
0x001d77e8:  ldr      s2, [x9, #0xf58]
0x001d77ec:  adrp     x9, #0x37000
0x001d77f0:  fmadd    s0, s1, s2, s0
0x001d77f4:  fmov     s1, #0.50000000
0x001d77f8:  fminnm   s1, s0, s1
0x001d77fc:  fcmp     s0, #0.0
0x001d7800:  ldr      s0, [x9, #0xea0]
0x001d7804:  fcsel    s0, s1, s0, gt
0x001d7808:  str      s0, [x21, #0x18]
0x001d780c:  ldr      q0, [sp, #0x10]
0x001d7810:  nop      
0x001d7814:  adr      x0, #0x27954c
0x001d7818:  str      q0, [x8]
0x001d781c:  bl       #0x26c4b0
0x001d7820:  adrp     x8, #0x279000
0x001d7824:  adrp     x9, #0x279000
0x001d7828:  ldr      s0, [x8, #0x650]
0x001d782c:  ldr      s1, [x9, #0x654]
0x001d7830:  mov      x0, x21
0x001d7834:  bl       #0x1ebd9c
0x001d7838:  adrp     x8, #0x279000
0x001d783c:  ldrb     w2, [x8, #0x658]
0x001d7840:  mov      x0, x21
0x001d7844:  mov      w1, wzr
0x001d7848:  bl       #0x1ebf60
0x001d784c:  nop      
0x001d7850:  adr      x0, #0x27954c
0x001d7854:  bl       #0x26c4e0
0x001d7858:  bl       #0x22eaf8
0x001d785c:  bl       #0x1f3570
0x001d7860:  adrp     x10, #0x279000
0x001d7864:  ldr      w8, [sp, #0x20]
0x001d7868:  ldur     w9, [x29, #-0x1c]
0x001d786c:  ldr      x0, [x10, #0x510]
0x001d7870:  adrp     x23, #0x279000
0x001d7874:  adrp     x24, #0x279000
0x001d7878:  str      w8, [x23, #0x598]
0x001d787c:  str      w9, [x24, #0x59c]
0x001d7880:  bl       #0x1da768
0x001d7884:  adrp     x8, #0x279000
0x001d7888:  ldrb     w8, [x8, #0x5a0]
0x001d788c:  cbz      w8, #0x1d7894
0x001d7890:  bl       #0x1e955c
0x001d7894:  adrp     x8, #0x279000
0x001d7898:  ldrb     w8, [x8, #0x5a1]
0x001d789c:  cbz      w8, #0x1d78fc
0x001d78a0:  adrp     x8, #0x279000
0x001d78a4:  ldrb     w8, [x8, #0x5a2]
0x001d78a8:  cbz      w8, #0x1d78fc
0x001d78ac:  ldr      s0, [x24, #0x59c]
0x001d78b0:  ldr      s1, [x23, #0x598]
0x001d78b4:  fmov     s2, #0.50000000
0x001d78b8:  nop      
0x001d78bc:  adr      x0, #0x278ff8
0x001d78c0:  scvtf    s0, s0
0x001d78c4:  scvtf    s1, s1
0x001d78c8:  fmul     s0, s0, s2
0x001d78cc:  fmul     s1, s1, s2
0x001d78d0:  stp      s0, s1, [sp, #0x10]
0x001d78d4:  bl       #0x1ed030
0x001d78d8:  mov      w21, w0
0x001d78dc:  bl       #0x1f288c
0x001d78e0:  adrp     x8, #0x279000
0x001d78e4:  fmov     s1, #2.00000000
0x001d78e8:  add      x1, sp, #0x10
0x001d78ec:  ldr      s0, [x8, #0x5a4]
0x001d78f0:  mov      w2, w21
0x001d78f4:  movz     w3, #0x40
0x001d78f8:  bl       #0x208f88
0x001d78fc:  adrp     x8, #0x279000
0x001d7900:  ldrb     w8, [x8, #0x548]
0x001d7904:  cbz      w8, #0x1d7978
0x001d7908:  bl       #0x1f288c
0x001d790c:  ldr      s0, [x24, #0x59c]
0x001d7910:  ldr      s1, [x23, #0x598]
0x001d7914:  fmov     s2, #0.50000000
0x001d7918:  add      x1, sp, #0x10
0x001d791c:  add      x2, sp, #8
0x001d7920:  orr      w3, wzr, #0xff0000ff
0x001d7924:  scvtf    s0, s0
0x001d7928:  scvtf    s1, s1
0x001d792c:  mov      x21, x0
0x001d7930:  fmul     s8, s0, s2
0x001d7934:  fmul     s9, s1, s2
0x001d7938:  fsub     s0, s8, s9
0x001d793c:  fadd     s1, s8, s9
0x001d7940:  stp      s0, s9, [sp, #0x10]
0x001d7944:  fmov     s0, #2.00000000
0x001d7948:  stp      s1, s9, [sp, #8]
0x001d794c:  bl       #0x208758
0x001d7950:  fsub     s0, s9, s9
0x001d7954:  fadd     s1, s9, s9
0x001d7958:  add      x1, sp, #0x10
0x001d795c:  add      x2, sp, #8
0x001d7960:  mov      x0, x21
0x001d7964:  orr      w3, wzr, #0xff0000ff
0x001d7968:  stp      s8, s0, [sp, #0x10]
0x001d796c:  fmov     s0, #2.00000000
0x001d7970:  stp      s8, s1, [sp, #8]
0x001d7974:  bl       #0x208758
0x001d7978:  bl       #0x1dac28
0x001d797c:  bl       #0x1db864
0x001d7980:  bl       #0x1fba6c
0x001d7984:  ldur     w2, [x29, #-0x1c]
0x001d7988:  ldr      w3, [sp, #0x20]
0x001d798c:  mov      w0, wzr
0x001d7990:  mov      w1, wzr
0x001d7994:  bl       #0x26c520
0x001d7998:  bl       #0x1f2764
0x001d799c:  bl       #0x22ef18
0x001d79a0:  adrp     x8, #0x279000
0x001d79a4:  mov      x0, x20
0x001d79a8:  mov      x1, x19
0x001d79ac:  ldr      x8, [x8, #0x578]
0x001d79b0:  blr      x8
0x001d79b4:  ldr      x8, [x22, #0x28]
0x001d79b8:  ldur     x9, [x29, #-0x18]
0x001d79bc:  cmp      x8, x9
0x001d79c0:  b.ne     #0x1d7a08
0x001d79c4:  ldp      x20, x19, [sp, #0x70]
0x001d79c8:  ldp      x22, x21, [sp, #0x60]
0x001d79cc:  ldp      x24, x23, [sp, #0x50]
0x001d79d0:  ldp      x29, x30, [sp, #0x40]
0x001d79d4:  ldp      d9, d8, [sp, #0x30]
0x001d79d8:  add      sp, sp, #0x80
0x001d79dc:  ret      
