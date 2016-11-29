.text
.code64

.extern callThreadMain

.global asm_call_thread_main
asm_call_thread_main:
    movq %r15, %rdi
	movq %r14, %rsi
	
	cld
	callq callThreadMain


.global asm_switch_threads
asm_switch_threads:
    pushq %rbp
    pushq %rbx
    pushq %r12
    pushq %r13
    pushq %r14
    pushq %r15
    pushfq

    movq %rsp, (%rdi)
    movq %rsi, %rsp

    popfq
    popq %r15
    popq %r14
    popq %r13
    popq %r12
    popq %rbx
    popq %rbp

    ret
