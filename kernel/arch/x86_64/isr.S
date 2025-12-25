.global isr_default_stub
.type isr_default_stub, @function
isr_default_stub:
    pushq $0
    pushq $0xFF
    jmp isr_common_stub

// Exceptions 0..31 -> common stub.
// For exceptions without an error code: push dummy 0, then vector.
// For exceptions with an error code: CPU already pushed error code, so we push only vector.

.macro ISR_NOERR vec
    .global isr_stub_\vec
    .type isr_stub_\vec, @function
isr_stub_\vec:
    pushq $0
    pushq $\vec
    jmp isr_common_stub
.endm

.macro ISR_ERR vec
    .global isr_stub_\vec
    .type isr_stub_\vec, @function
isr_stub_\vec:
    pushq $\vec
    jmp isr_common_stub
.endm

ISR_NOERR 0
ISR_NOERR 1
ISR_NOERR 2
ISR_NOERR 3
ISR_NOERR 4
ISR_NOERR 5
ISR_NOERR 6
ISR_NOERR 7
ISR_ERR   8
ISR_NOERR 9
ISR_ERR   10
ISR_ERR   11
ISR_ERR   12
ISR_ERR   13
ISR_ERR   14
ISR_NOERR 15
ISR_NOERR 16
ISR_ERR   17
ISR_NOERR 18
ISR_NOERR 19
ISR_NOERR 20
ISR_ERR   21
ISR_NOERR 22
ISR_NOERR 23
ISR_NOERR 24
ISR_NOERR 25
ISR_NOERR 26
ISR_NOERR 27
ISR_NOERR 28
ISR_ERR   29
ISR_ERR   30
ISR_NOERR 31
