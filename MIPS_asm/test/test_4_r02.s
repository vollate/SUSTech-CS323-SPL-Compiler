# SPL compiler generated assembly
.data
_prmpt: .asciiz "Enter an integer: "
_eol: .asciiz "\n"
.globl main
.text
j main

read:
li $v0, 4
la $a0, _prmpt
syscall
li $v0, 5
syscall
jr $ra

write:
li $v0, 1
syscall
li $v0, 4
la $a0, _eol
syscall
move $v0, $0
jr $ra

main:
addi $sp, $sp, -48
sw $ra, 44($sp)
sw $fp, 40($sp)
move $fp, $sp
# new var v2, store in reg $t0
addi $sp, $sp, -4
sw $ra, 0($sp)
jal read
lw $ra, 0($sp)
addi $sp, $sp, 4
move $t0, $v0
# new var v0, store in reg $t1
move $t1, $t0
# new var v3, store in reg $t2
move $t2, $t1
# new var v4, store in reg $t3
li $t3, 0
sw $t0, 36($sp) # spill v2 from reg $t0
sw $t1, 32($sp) # spill v0 from reg $t1
sw $t2, 28($sp) # spill v3 from reg $t2
sw $t3, 24($sp) # spill v4 from reg $t3
lw $t0, 28($sp) # load v3 to reg $t0
lw $t1, 24($sp) # load v4 to reg $t1
bgt $t0, $t1, label0
j label1
label0:
# new var v6, store in reg $t0
li $t0, 1
move $a0, $t0
addi $sp, $sp, -4
sw $ra, 0($sp)
jal write
lw $ra, 0($sp)
addi $sp, $sp, 4
sw $t0, 20($sp) # spill v6 from reg $t0
j label2
label1:
# new var v7, store in reg $t0
lw $t1, 32($sp) # load v0 to reg $t1
move $t0, $t1
# new var v8, store in reg $t2
li $t2, 0
sw $t0, 16($sp) # spill v7 from reg $t0
sw $t2, 12($sp) # spill v8 from reg $t2
lw $t0, 16($sp) # load v7 to reg $t0
lw $t1, 12($sp) # load v8 to reg $t1
blt $t0, $t1, label3
j label4
label3:
# new var v11, store in reg $t0
li $t0, 1
# new var v10, store in reg $t1
li $t2, 0
sub $t1, $t2, $t0
move $a0, $t1
addi $sp, $sp, -4
sw $ra, 0($sp)
jal write
lw $ra, 0($sp)
addi $sp, $sp, 4
sw $t0, 8($sp) # spill v11 from reg $t0
sw $t1, 4($sp) # spill v10 from reg $t1
j label5
label4:
# new var v13, store in reg $t0
li $t0, 0
move $a0, $t0
addi $sp, $sp, -4
sw $ra, 0($sp)
jal write
lw $ra, 0($sp)
addi $sp, $sp, 4
label5:
label2:
# new var v14, store in reg $t1
li $t1, 0
move $sp, $fp
lw $ra, 44($sp)
lw $fp, 40($sp)
addi $sp, $sp, 48
move $v0, $t1
jr $ra
