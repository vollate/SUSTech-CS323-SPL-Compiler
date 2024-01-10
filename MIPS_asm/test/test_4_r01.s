# SPL compiler generated assembly
.data
_prmpt: .asciiz "Enter an integer: "
_eol: .asciiz "\n"
.global main
.text
read:
li $v0, 4 la $a0, 4
la $a0, _prmpt
syscall
li $v0, 5
syscall
jr $rawrite:
li $v0, 1
syscall
li $v0, 4
la $a0, _eol
syscall
move $v0, $0
jr $ra
