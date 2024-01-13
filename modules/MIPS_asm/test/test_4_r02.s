# SPL compiler generated assembly
.data
_prmpt: .asciiz "Enter an integer: "
_eol: .asciiz "\n"
.globl main
.text

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
addi $sp, $sp, -4
sw $ra, 0($sp)
jal read
lw $ra, 0($sp)
addi $sp, $sp, 4
move $t0, $v0
move $t1, $t0
move $t2, $t1
li $t3, 0
sw $t0, 36($sp)
sw $t1, 32($sp)
sw $t2, 28($sp)
sw $t3, 24($sp)
lw $t0, 28($sp)
lw $t1, 24($sp)
bgt $t0, $t1, label0
j label1
label0:
li $t0, 1
move $a0, $t0
addi $sp, $sp, -4
sw $ra, 0($sp)
jal write
lw $ra, 0($sp)
addi $sp, $sp, 4
sw $t0, 20($sp)
j label2
label1:
lw $t0, 32($sp)
move $t1, $t0
li $t2, 0
sw $t1, 16($sp)
sw $t2, 12($sp)
lw $t0, 16($sp)
lw $t1, 12($sp)
blt $t0, $t1, label3
j label4
label3:
li $t0, 1
li $t2, 0
sub $t1, $t2, $t0
move $a0, $t1
addi $sp, $sp, -4
sw $ra, 0($sp)
jal write
lw $ra, 0($sp)
addi $sp, $sp, 4
sw $t0, 8($sp)
sw $t1, 4($sp)
j label5
label4:
li $t0, 0
move $a0, $t0
addi $sp, $sp, -4
sw $ra, 0($sp)
jal write
lw $ra, 0($sp)
addi $sp, $sp, 4
label5:
label2:
li $t1, 0
move $sp, $fp
lw $ra, 44($sp)
lw $fp, 40($sp)
addi $sp, $sp, 48
move $v0, $t1
jr $ra
