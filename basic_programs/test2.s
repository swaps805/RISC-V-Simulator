.data  
.dword 0xf123456789abcdef,  
.word 0xf123adcd, 0x34a; this is a comment


.text

lui x1, 0x10

lw x2, 0(x1)
srli x3, x2, 4
srai x4, x2, 4
sb x3, 32(x1)






