kernel_start:
	lidt idt
	lgdt gdt

	iret main


syscall1:
	print one
	iret

syscall2:
	print two
	iret

syscall3:
	print three
	iret

idt:
	db 3
	dd syscall1, syscall2, syscall3


gdt:
	db 2
	dd kernel_start, kernel_end, 0
	dd userspace_start, userspace_end, 1

one:
	db "syscall one!", 10, 0

two:
	db "syscall two!", 10, 0

three:
	db "syscall three!", 10, 0

main_str:
	db "doing stuff in main", 10, 0
kernel_end:

# userspace begins

userspace_start:

main:
	print main_str
	int 1
	int 2
	int 3
	hlt


userspace_end:


