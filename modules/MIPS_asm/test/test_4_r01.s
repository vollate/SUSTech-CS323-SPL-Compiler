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

hanoi:
addi $sp, $sp, -120
sw $ra, 116($sp)
sw $fp, 112($sp)
move $fp, $sp
move $t0, $a3
move $t1, $a2
move $t2, $a1
move $t3, $a0
move $t4, $t0
li $t5, 1
sw $t0, 108($sp)
sw $t1, 104($sp)
sw $t2, 100($sp)
sw $t3, 96($sp)
sw $t4, 92($sp)
sw $t5, 88($sp)
lw $t0, 92($sp)
lw $t1, 88($sp)
beq $t0, $t1, label0
j label1
label0:
lw $t0, 104($sp)
move $t1, $t0
li $t2, 10000
mul $t3, $t1, $t2
lw $t4, 96($sp)
move $t5, $t4
add $t6, $t3, $t5
move $a0, $t6
addi $sp, $sp, -4
sw $ra, 0($sp)
jal write
lw $ra, 0($sp)
addi $sp, $sp, 4
sw $t1, 84($sp)
sw $t2, 80($sp)
sw $t3, 76($sp)
sw $t5, 72($sp)
sw $t6, 68($sp)
j label2
label1:
lw $t0, 108($sp)
move $t1, $t0
li $t2, 1
sub $t3, $t1, $t2
lw $t4, 104($sp)
move $t5, $t4
lw $t6, 96($sp)
move $t7, $t6
lw $t8, 100($sp)
move $t9, $t8
move $a0, $t9
move $a1, $t7
move $a2, $t5
move $a3, $t3
sw $t1, 64($sp)
sw $t2, 60($sp)
sw $t3, 56($sp)
sw $t5, 52($sp)
sw $t7, 48($sp)
sw $t9, 44($sp)
jal hanoi
move $t0, $v0
lw $t1, 104($sp)
move $t2, $t1
li $t3, 10000
mul $t4, $t2, $t3
lw $t5, 96($sp)
move $t6, $t5
add $t7, $t4, $t6
move $a0, $t7
addi $sp, $sp, -4
sw $ra, 0($sp)
jal write
lw $ra, 0($sp)
addi $sp, $sp, 4
lw $t8, 108($sp)
move $t9, $t8
li $t1, 1
sub $t5, $t9, $t1
lw $t8, 100($sp)
sw $t0, 40($sp)
move $t0, $t8
lw $t8, 104($sp)
sw $t0, 4($sp)
move $t0, $t8
lw $t8, 96($sp)
sw $t0, 0($sp)
move $t0, $t8
move $a0, $t0
lw $t8, 0($sp)
move $a1, $t8
lw $t8, 4($sp)
move $a2, $t8
move $a3, $t5
sw $t0, -4($sp)
sw $t1, 12($sp)
sw $t2, 36($sp)
sw $t3, 32($sp)
sw $t4, 28($sp)
sw $t5, 8($sp)
sw $t6, 24($sp)
sw $t7, 20($sp)
sw $t9, 16($sp)
jal hanoi
move $t0, $v0
label2:
li $t1, 0
move $sp, $fp
lw $ra, 116($sp)
lw $fp, 112($sp)
addi $sp, $sp, 120
move $v0, $t1
jr $ra

main:
addi $sp, $sp, -24
sw $ra, 20($sp)
sw $fp, 16($sp)
move $fp, $sp
li $t2, 3
move $t3, $t2
move $t4, $t3
li $t5, 1
li $t6, 2
li $t7, 3
move $a0, $t7
move $a1, $t6
move $a2, $t5
move $a3, $t4
sw $t0, 16($sp)
sw $t1, 12($sp)
sw $t2, 12($sp)
sw $t3, 8($sp)
sw $t4, 4($sp)
sw $t5, 0($sp)
sw $t6, -4($sp)
sw $t7, -8($sp)
jal hanoi
move $t0, $v0
li $t1, 0
move $sp, $fp
lw $ra, 20($sp)
lw $fp, 16($sp)
addi $sp, $sp, 24
move $v0, $t1
jr $ra
