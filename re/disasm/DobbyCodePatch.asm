0x0023bcf0:  stp      x30, x25, [sp, #-0x40]!
0x0023bcf4:  stp      x24, x23, [sp, #0x10]
0x0023bcf8:  stp      x22, x21, [sp, #0x20]
0x0023bcfc:  stp      x20, x19, [sp, #0x30]
0x0023bd00:  mov      x19, x0
0x0023bd04:  movz     w0, #0x27
0x0023bd08:  mov      w20, w2
0x0023bd0c:  mov      x22, x1
0x0023bd10:  bl       #0x26cc10
0x0023bd14:  mov      w23, w20
0x0023bd18:  sxtw     x20, w0
0x0023bd1c:  movz     w2, #0x7
0x0023bd20:  add      x21, x23, x19
0x0023bd24:  neg      x8, x20
0x0023bd28:  mov      x1, x20
0x0023bd2c:  and      x25, x8, x19
0x0023bd30:  and      x24, x21, x8
0x0023bd34:  mov      x0, x25
0x0023bd38:  bl       #0x26cc20
0x0023bd3c:  cmp      x25, x24
0x0023bd40:  b.ne     #0x23bd5c
0x0023bd44:  mov      x0, x19
0x0023bd48:  mov      x1, x22
0x0023bd4c:  mov      x2, x23
0x0023bd50:  bl       #0x26c420
0x0023bd54:  mov      x24, x25
0x0023bd58:  b        #0x23bd8c
0x0023bd5c:  mov      x0, x24
0x0023bd60:  mov      x1, x20
0x0023bd64:  movz     w2, #0x7
0x0023bd68:  bl       #0x26cc20
0x0023bd6c:  mov      x0, x19
0x0023bd70:  mov      x1, x22
0x0023bd74:  mov      x2, x23
0x0023bd78:  bl       #0x26c420
0x0023bd7c:  mov      x0, x25
0x0023bd80:  mov      x1, x20
0x0023bd84:  movz     w2, #0x5
0x0023bd88:  bl       #0x26cc20
0x0023bd8c:  mov      x0, x24
0x0023bd90:  mov      x1, x20
0x0023bd94:  movz     w2, #0x5
0x0023bd98:  bl       #0x26cc20
0x0023bd9c:  mov      x0, x19
0x0023bda0:  mov      x1, x21
0x0023bda4:  bl       #0x23bdc0
0x0023bda8:  ldp      x20, x19, [sp, #0x30]
0x0023bdac:  mov      w0, wzr
0x0023bdb0:  ldp      x22, x21, [sp, #0x20]
0x0023bdb4:  ldp      x24, x23, [sp, #0x10]
0x0023bdb8:  ldp      x30, x25, [sp], #0x40
0x0023bdbc:  ret      
