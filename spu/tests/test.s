start:
	in
	in
	sub

	# cmp 10, 10

	dump

	print resis
	out

	jz printe
	jg printg
	jl printl
	hlt


printl:
	print l
	hlt


printe:
	print e
	hlt
printg:
	print g
	hlt


l:
	db "less!", 10, 0

e:
	db "eq!!", 10, 0

g:
	db "Greater!", 10, 0

resis:
	db "result is: ", 0
