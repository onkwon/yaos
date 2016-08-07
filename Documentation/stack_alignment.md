## Eight-byte stack alignment & Narrow arguments

### Summary

ARM 아키텍처의 외부 인터페이스에 대한 스택 정렬과 매개변수 규약에 대해 정리한다. 마땅히 ABI 에 대한 내용이므로 스택 정렬과 매개변수 규약에 국한하지 않고, 관련 내용을 접할 때마다 아래 내용을 업데이트할 것이다.

ARM 아키텍처 ABI(AAPCS) 은 스택을 8 바이트 단위로 정렬할 것을 권고한다. ABI 는 외부 인터페이스를 위한 프로토콜이므로 다른 함수를 호출하지 않는 말단 함수의 경우 해당 규약으로부터 자유롭다고 생각할 수도 있겠으나, 말단 함수에서 정렬이 깨진 상태로 인터럽트나 예외가 발생할 경우 문제가 생길 수가 있음을 주지해야 한다.

기본형보다 작은 데이터 타입을 매개변수로 넘기는 경우 해당 데이터 타입은 기본형으로 확장된다. 그외 기본형보다 큰 공간을 차지하는 long long, float, double 등의 데이터 타입은 해당 크기만큼 공간을 차지하게 된다. 주의할 점은 넘겨지는 매개변수의 개수가 특정되지 않은 가변인자 리스트이거나 pre-ANSI C 함수의 알려지지 않은 타입인 경우 float 데이터 타입은 double 형으로 확장된다.

### 테스트 코드

#### 정수형 인자 5개

정수형 인자 5 개는 규약에 따라 레지스터 r0-r3 와 스택으로(마지막 인수) 넘겨진다. 스택에 넘겨진 홀수개의 인자로 인해 caller 뿐만 아니라 callee 스택의 8 바이트 정렬이 깨지므로 이를 보상하기 위해 컴파일러는 padding 을 삽입할 것이다.

```c
int subsub(int a, int b, int c, int d, int e)
{
        int f = 6;

        return e;
}

int sub(int a, int b, int c, int d, int e)
{
        int f = 6;
        int g = 7;
        int h = 8;
        int i = 9;

        subsub(a, b, c, d, e); 

        return e;
}

int main()
{
        int a;

        sub(1, 2, 3, 4, 5); 
        return 0;
}
```

```
$ arm-none-eabi-gcc -mtune=cortex-a7 -O0 test_stack_alignment.c -lgcc
$ arm-none-eabi-objdump -D a.out

[...]

00008220 <subsub>:
    8220:       e52db004        push    {fp}            ; (str fp, [sp, #-4]!)
    8224:       e28db000        add     fp, sp, #0
    8228:       e24dd01c        sub     sp, sp, #28
    822c:       e50b0010        str     r0, [fp, #-16]
    8230:       e50b1014        str     r1, [fp, #-20]  ; 0xffffffec
    8234:       e50b2018        str     r2, [fp, #-24]  ; 0xffffffe8
    8238:       e50b301c        str     r3, [fp, #-28]  ; 0xffffffe4
    823c:       e3a03006        mov     r3, #6
    8240:       e50b3008        str     r3, [fp, #-8]
    8244:       e59b3004        ldr     r3, [fp, #4]
    8248:       e1a00003        mov     r0, r3
    824c:       e24bd000        sub     sp, fp, #0
    8250:       e49db004        pop     {fp}            ; (ldr fp, [sp], #4)
    8254:       e12fff1e        bx      lr

00008258 <sub>:
    8258:       e92d4800        push    {fp, lr}
    825c:       e28db004        add     fp, sp, #4
    8260:       e24dd028        sub     sp, sp, #40     ; 0x28
    8264:       e50b0018        str     r0, [fp, #-24]  ; 0xffffffe8
    8268:       e50b101c        str     r1, [fp, #-28]  ; 0xffffffe4
    826c:       e50b2020        str     r2, [fp, #-32]  ; 0xffffffe0
    8270:       e50b3024        str     r3, [fp, #-36]  ; 0xffffffdc
    8274:       e3a03006        mov     r3, #6
    8278:       e50b3008        str     r3, [fp, #-8]
    827c:       e3a03007        mov     r3, #7
    8280:       e50b300c        str     r3, [fp, #-12]
    8284:       e3a03008        mov     r3, #8
    8288:       e50b3010        str     r3, [fp, #-16]
    828c:       e3a03009        mov     r3, #9
    8290:       e50b3014        str     r3, [fp, #-20]  ; 0xffffffec
    8294:       e59b3004        ldr     r3, [fp, #4] 
    8298:       e58d3000        str     r3, [sp]
    829c:       e51b3024        ldr     r3, [fp, #-36]  ; 0xffffffdc
    82a0:       e51b2020        ldr     r2, [fp, #-32]  ; 0xffffffe0
    82a4:       e51b101c        ldr     r1, [fp, #-28]  ; 0xffffffe4
    82a8:       e51b0018        ldr     r0, [fp, #-24]  ; 0xffffffe8
    82ac:       ebffffdb        bl      8220 <subsub>
    82b0:       e59b3004        ldr     r3, [fp, #4] 
    82b4:       e1a00003        mov     r0, r3
    82b8:       e24bd004        sub     sp, fp, #4
    82bc:       e8bd4800        pop     {fp, lr} 
    82c0:       e12fff1e        bx      lr  

000082c4 <main>:
    82c4:       e92d4800        push    {fp, lr} 
    82c8:       e28db004        add     fp, sp, #4
    82cc:       e24dd008        sub     sp, sp, #8
    82d0:       e3a03005        mov     r3, #5
    82d4:       e58d3000        str     r3, [sp]
    82d8:       e3a03004        mov     r3, #4
    82dc:       e3a02003        mov     r2, #3
    82e0:       e3a01002        mov     r1, #2
    82e4:       e3a00001        mov     r0, #1
    82e8:       ebffffda        bl      8258 <sub>
    82ec:       e3a03000        mov     r3, #0
    82f0:       e1a00003        mov     r0, r3
    82f4:       e24bd004        sub     sp, fp, #4
    82f8:       e8bd4800        pop     {fp, lr}
    82fc:       e12fff1e        bx      lr
```

빗금친 공간이 정렬로 인해 생긴 빈 공간이다.

```
<main>          |-----------|
                | lr        |
                |-----------| -> fp1(main)
                | fp        |
                ^-----------| -> sp1(main)
                | ///////// |
         sp1-4  |-----------|
                | 5(r3)     |
<sub>    sp1-8  ^-----------| -> sp2(main)
                | lr        |
                |-----------| -> fp2(sub)
                | fp        |
         fp2-4  ^-----------| -> sp3(sub)
                | 6         |
         fp2-8  |-----------|
                | 7         |
         fp2-12 ^-----------|
                | 8         |
         fp2-16 |-----------|
                | 9         |
         fp2-20 ^-----------|
                | 1(r0)     |
         fp2-24 |-----------|
                | 2(r1)     |
         fp2-28 ^-----------|
                | 3(r2)     |
         fp2-32 |-----------|
                | 4(r3)     |
         fp2-36 ^-----------|
                | ///////// |
         fp2-40 |-----------|
                | 5(copied) |
<subsub> sp3-40 ^-----------| -> sp4(sub)
                | fp        |
                |-----------| -> sp5(subsub), fp3(subsub)
                | ///////// |
             -4 ^-----------|
                | 6         |
             -8 |-----------|
                | ///////// |
            -12 ^-----------|
                | 1(r0)     |
            -16 |-----------|
                | 2(r1)     |
            -20 ^-----------|
                | 3(r2)     |
            -24 |-----------|
                | 4(r3)     |
            -28 |-----------| ->sp6(subsub)
```

#### 정수형 인자 4개

위 코드(인자 5 개)와 달리 인자 4 개는 8 바이트 정렬을 유지하므로 padding 이 일어나지 않을 것이다.

```c
int subsub(int a, int b, int c, int d)
{
        int f = 6;

        return d;
}

int sub(int a, int b, int c, int d)
{
        int f = 6;
        int g = 7;
        int h = 8;
        int i = 9;

        subsub(a, b, c, d); 

        return d;
}

int main()
{
        int a;

        sub(1, 2, 3, 4); 
        return 0;
}
```

```
$ arm-none-eabi-gcc -mtune=cortex-a7 -O0 test_stack_alignment.c -lgcc
$ arm-none-eabi-objdump -D a.out

[...]

```
00008220 <subsub>:
    8220:	e52db004 	push	{fp}		; (str fp, [sp, #-4]!)
    8224:	e28db000 	add	fp, sp, #0
    8228:	e24dd01c 	sub	sp, sp, #28
    822c:	e50b0010 	str	r0, [fp, #-16]
    8230:	e50b1014 	str	r1, [fp, #-20]	; 0xffffffec
    8234:	e50b2018 	str	r2, [fp, #-24]	; 0xffffffe8
    8238:	e50b301c 	str	r3, [fp, #-28]	; 0xffffffe4
    823c:	e3a03006 	mov	r3, #6
    8240:	e50b3008 	str	r3, [fp, #-8]
    8244:	e51b301c 	ldr	r3, [fp, #-28]	; 0xffffffe4
    8248:	e1a00003 	mov	r0, r3
    824c:	e24bd000 	sub	sp, fp, #0
    8250:	e49db004 	pop	{fp}		; (ldr fp, [sp], #4)
    8254:	e12fff1e 	bx	lr

00008258 <sub>:
    8258:	e92d4800 	push	{fp, lr}
    825c:	e28db004 	add	fp, sp, #4
    8260:	e24dd020 	sub	sp, sp, #32
    8264:	e50b0018 	str	r0, [fp, #-24]	; 0xffffffe8
    8268:	e50b101c 	str	r1, [fp, #-28]	; 0xffffffe4
    826c:	e50b2020 	str	r2, [fp, #-32]	; 0xffffffe0
    8270:	e50b3024 	str	r3, [fp, #-36]	; 0xffffffdc
    8274:	e3a03006 	mov	r3, #6
    8278:	e50b3008 	str	r3, [fp, #-8]
    827c:	e3a03007 	mov	r3, #7
    8280:	e50b300c 	str	r3, [fp, #-12]
    8284:	e3a03008 	mov	r3, #8
    8288:	e50b3010 	str	r3, [fp, #-16]
    828c:	e3a03009 	mov	r3, #9
    8290:	e50b3014 	str	r3, [fp, #-20]	; 0xffffffec
    8294:	e51b3024 	ldr	r3, [fp, #-36]	; 0xffffffdc
    8298:	e51b2020 	ldr	r2, [fp, #-32]	; 0xffffffe0
    829c:	e51b101c 	ldr	r1, [fp, #-28]	; 0xffffffe4
    82a0:	e51b0018 	ldr	r0, [fp, #-24]	; 0xffffffe8
    82a4:	ebffffdd 	bl	8220 <subsub>
    82a8:	e51b3024 	ldr	r3, [fp, #-36]	; 0xffffffdc
    82ac:	e1a00003 	mov	r0, r3
    82b0:	e24bd004 	sub	sp, fp, #4
    82b4:	e8bd4800 	pop	{fp, lr}
    82b8:	e12fff1e 	bx	lr

000082bc <main>:
    82bc:	e92d4800 	push	{fp, lr}
    82c0:	e28db004 	add	fp, sp, #4
    82c4:	e3a03004 	mov	r3, #4
    82c8:	e3a02003 	mov	r2, #3
    82cc:	e3a01002 	mov	r1, #2
    82d0:	e3a00001 	mov	r0, #1
    82d4:	ebffffdf 	bl	8258 <sub>
    82d8:	e3a03000 	mov	r3, #0
    82dc:	e1a00003 	mov	r0, r3
    82e0:	e24bd004 	sub	sp, fp, #4
    82e4:	e8bd4800 	pop	{fp, lr}
    82e8:	e12fff1e 	bx	lr
```

```
<main>          |-----------|
                | lr        |
                |-----------| -> fp1(main)
                | fp        |
<sub>           ^-----------| -> sp1(main)
                | lr        |
                |-----------| -> fp2(sub)
                | fp        |
         fp2-4  ^-----------| -> sp3(sub)
                | 6         |
         fp2-8  |-----------|
                | 7         |
         fp2-12 ^-----------|
                | 8         |
         fp2-16 |-----------|
                | 9         |
         fp2-20 ^-----------|
                | 1(r0)     |
         fp2-24 |-----------|
                | 2(r1)     |
         fp2-28 ^-----------|
                | 3(r2)     |
         fp2-32 |-----------|
                | 4(r3)     |
<subsub> fp2-36 ^-----------| -> sp4(sub)
                | fp        |
                |-----------| -> sp5(subsub), fp3(subsub)
                | ///////// | --> callee saved register 가 짝수개라면 align 된 이 공간이 생기지 않을 것
             -4 ^-----------|
                | 6         |
             -8 |-----------|
                | ///////// | --> 내부 변수가 짝수개라면 align 된 이 공간이 생기지 않을 것
            -12 ^-----------|
                | 1(r0)     |
            -16 |-----------|
                | 2(r1)     |
            -20 ^-----------|
                | 3(r2)     |
            -24 |-----------|
                | 4(r3)     |
            -28 |-----------| ->sp6(subsub)
```

### References

> The Application Binary Interface (ABI) for the ARM architecture requires that the stack must be eight-byte aligned on all external interfaces, such as calls between functions in different source files. However, code does not need to maintain eight-byte stack alignment internally, for example in leaf functions.

ARM 아키텍처 ABI에 따르면 (다른 소스파일 간의 함수 호출등과 같은)모든 외부 인터페이스에 대해 스택은 8바이트 단위로 정렬되어야 한다. 하지만, leaf 함수와 같은 경우, 내부적으로 스택을 8바이트 정렬할 필요는 없다.

> This means that when an interrupt or exception occurs the stack might not be correctly eight-byte aligned. Revision 1 and later of the Cortex-M3 silicon can automatically align the stack pointer when an exception occurs. This behavior must be enabled by setting STKALIGN (bit 9) in the Configuration Control Register at address 0xE000ED14.

이는 인터럽트나 예외가 발생한 경우 스택은 8바이트로 정렬되어 있지 않을 수 있다는 것이다. 리비전 1과 이후의 M3 버전은 예외가 발생한 경우 자동적으로 스택 포인터를 정렬할 수 있다. 이 기능은 `0xE000ED14` 레지스터의 STKALIGN 비트를 셋팅해야 동작한다.

> If you are using revision 0 of the Cortex-M3, this adjustment cannot be performed in hardware. The compiler can generate code in your IRQ handlers that correctly aligns the stack. To do this you must prefix your IRQ handlers with `__irq` and use the `--cpu=Cortex-M3-rev0` compiler switch, not `--cpu=Cortex-M3`.

만약 리비전 0을 사용하고 있다면 하드웨어적으로 정렬은 지원되지 않는다. 따라서 제대로 정렬을 수행하기 위해서는 컴파일러 지원을 받아야 한다. IRQ 핸들러에 `__irq` 프리픽스를 사용하고 컴파일 옵션에 `--cpu=Cortex-M3-rev0` 를 적용하라.

> The --cpu=Cortex-M3-rev0 compiler switch is only supported in RVCT 3.0 SP1 (build 586) and later versions.

`--cpu=Cortex-M3-rev0` 컴파일러 옵션은 RVCT 3.0 SP1 또는 이후 버전에서만 지원된다.

#### Narrow arguments

> A narrow integer argument (type char, short, enum, and so on) is widened to fill a 32-bit word by zero-extending it or sign-extending it as appropriate to its type. - [The ARM-THUMB Procedure Call Standard - 7.1 ANSI C and C++ argument passing conventions](http://www.cs.cornell.edu/courses/cs414/2001fa/armcallconvention.pdf)

기본형보다 작은 데이터 타입은 기본형으로 확장된다.

> In C++, and in ANSI-C in the presence of a function prototype, a calling function does not convert a float argument value to double if the called function’s parameter type is float.

함수의 인자형이 float 으로 명시된 경우 float 형은 double 형으로 확장되지 않는다.

> If a float argument value matches an ellipsis (‘...’) in the called function’s parameter list, or is being passed to a pre-ANSI C function of unknown parameter type, the calling function must convert the value to double. - 7.2 Narrow arguments

하지만, 가변인자 리스트 함수이거나 pre-ANSI C 함수의 알려지지 않은 타입인 경우 float 타입은 double 형으로 확장된다.

### Links

> Eight-byte stack alignment is of particular benefit to processors supporting LDRD and STRD instructions, for example, processors based on ARM architecture v5TE and later. If the stack is not eight-byte aligned the use of LDRD and STRD might cause an alignment fault, depending on the target and configuration used. - [ARM Information Center](http://infocenter.arm.com/help/index.jsp?topic=/com.arm.doc.faqs/14269.html)

v5TE 또는 그 이후의 ARM 아키텍처, 즉 LDRD와 STRD 명령어를 지원하는 프로세서에서 8 바이트로 정렬된 스택은 성능상 이점을 준다. 만약 8 바이트로 정렬되지 않은 주소에 LDRD와 STRD로 액세스를 시도한다면 alignment fault가 발생할 것이다.

> In ARMv7-A, the alignment requirement for LDRD/STRD is 4-byte.  There is potentially a performance benefit if the address is 8-byte aligned.  In ARMv5TE they were required to 8-byte aligned. - [8-byte stack alignment for ARM Cortex-A9](https://community.arm.com/thread/6910)

8 바이트로 정렬되어있다면 성능상 이점이 있겠지만, ARMv7-A 아키텍처에서 LDRD/STRD 명령어는 4 바이트 정렬을 지원한다(alignment fault를 발생시키지 않는다). - [ABI for the ARM Architecture Advisory Note - SP must be 8-byte aligned on entry to AAPCS-conforming functions](http://infocenter.arm.com/help/topic/com.arm.doc.ihi0046b/IHI0046B_ABI_Advisory_1.pdf)

* [What is an external interface?](http://infocenter.arm.com/help/index.jsp?topic=/com.arm.doc.faqs/ka13786.html)
  - external interface 란 분리되어 컴파일되었거나 어셈블된 루틴을 말한다.
  - leaf function 이 아닌 경우, 스택은 반드시 8 바이트로 정렬되어야 한다. 따라서 컴파일러는 짝수 개의 레지스터를 push 하거나(32-bit 시스템) 스택 포인터를 8바이트 단위로 조정한다.
* [What is a leaf function?](http://infocenter.arm.com/help/index.jsp?topic=/com.arm.doc.faqs/ka13785.html)
* [Procedure Call Standard for the ARM Architecture](http://infocenter.arm.com/help/topic/com.arm.doc.ihi0042f/IHI0042F_aapcs.pdf)
* [ELF for ARM Architecture](http://infocenter.arm.com/help/topic/com.arm.doc.ihi0044f/IHI0044F_aaelf.pdf)
