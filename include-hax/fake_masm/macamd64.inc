push_reg macro Reg
	push Reg
	.pushreg Reg
	endm

alloc_stack macro Size
	sub rsp, Size
	.allocstack Size
	endm
