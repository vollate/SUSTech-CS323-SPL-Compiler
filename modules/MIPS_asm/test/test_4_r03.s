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
addi $sp, $sp, -40
sw $ra, 36($sp)
sw $fp, 32($sp)
move $fp, $sp
li $t0, 110
li $t1, 97
li $t2, 3
sub $t3, $t0, $t1
li $t4, 2
mul $t5, $t2, $t4
add $t6, $t3, $t5
move $t2, $t6
move $t7, $t2
move $a0, $t7
addi $sp, $sp, -4
sw $ra, 0($sp)
jal write
lw $ra, 0($sp)
addi $sp, $sp, 4
li $t8, 0
move $sp, $fp
lw $ra, 36($sp)
lw $fp, 32($sp)
addi $sp, $sp, 40
move $v0, $t8
jr $ra
