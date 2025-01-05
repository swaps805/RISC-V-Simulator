.data 
.dword 0xabefc
.byte 0x56
.word 0x12345678
.dword -1

.text
addi x5, x5, 0
lui x4, 0x10
addi x5, x5, -1 
sd x5, 2(x4) 
addi x5, x5, 1

