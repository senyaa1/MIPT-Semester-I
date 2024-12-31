start:
	mov [num], 10
	mov dx, [num]
	dump
	hlt

num:
	db 0
