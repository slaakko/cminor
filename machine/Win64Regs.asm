.CODE

getrsp PROC
mov rax, rsp
add rax, 8
ret
getrsp ENDP

getrip PROC
mov rax, [rsp]
ret
getrip ENDP

getrbp PROC
mov rax, rbp
ret
getrbp ENDP

END
