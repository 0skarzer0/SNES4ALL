
.macro UPDATE_APUTIMER_WITH_SHUTODOWN_i
#ifdef SPC700_SHUTDOWN
	mov	#1,r0
	mov.l	APUWaitCounter, r4
	add	r0,r4
	mov.l	r4, APUWaitCounter
#endif
.endm

.macro UPDATE_APUTIMER_WITH_SHUTODOWN_f
#ifdef SPC700_SHUTDOWN
	mov.b   r0, APUExecuting
#endif
.endm

.macro UPDATE_APUTIMER_WITH_SHUTODOWN
	UPDATE_APUTIMER_WITH_SHUTODOWN_i
	UPDATE_APUTIMER_WITH_SHUTODOWN_f
.endm


.macro UPDATE_APUTIMER K
// r1 = IAPU
// r2 = APUTimer_ptr
// r3 = Cycles * 10000
	mov.l	IAPU, r1
	mov.l	.S9xUpdateAPUTimer_APUTimer_ptr, r2
	mov.l	Cycles, r3
	mov.l	.S9xUpdateAPUTimer_CONST00002710, r0
	mul.l	r0, r3
	sts	macl, r3
.S9xUpdateAPUTimer_bucle\K:
	mov.l	NextAPUTimerPos, r4
	cmp/ge	r4, r3
	bf	.S9xUpdateAPUTimer_salida\K

#ifdef USE_ASM_R5
	mov.l	.S9xUpdateAPUTimer_CONST003334bb, r5
	add	r5, r4
#else
	mov.l	.S9xUpdateAPUTimer_CONST003334bb, r0
	add	r0, r4
#endif
	mov.l	r4, NextAPUTimerPos

	mov.b	@(APUTimerEnabled - APUTimer + 2, r2), r0
	tst	r0, r0
	bt	.S9xUpdateAPUTimer_1salto\K

#ifdef USE_ASM_R5
	mov.w	@(4, r2), r0
	add     #1,r0
	mov.w	r0, @(4, r2)
	mov	r0, r5
	mov.w	@(APUTimerTarget - APUTimer + 4, r2), r0
	extu.w  r0,r4
	extu.w	r5,r0
#else
	mov.w	@(APUTimerTarget - APUTimer + 4, r2), r0
	extu.w  r0,r4
	mov.w	@(4, r2), r0
	add     #1,r0
	mov.w	r0, @(4, r2)
	extu.w	r0,r0
#endif
	cmp/hs  r4,r0
	bf	.S9xUpdateAPUTimer_bucle\K

	mov     #0,r0
	mov.w	r0, @(4, r2)

	mov.l	.S9xUpdateAPUTimer_CONST000000ff, r4
#ifdef USE_ASM_R5
	mov.l	APURAM, r5
	add	r4,r5
	mov.b	@r5,r0
	add	#1,r0
	and	#15,r0
	mov.b	r0,@r5
#else
	mov.l   APURAM,r0
	add	r0,r4
	mov.b	@r4,r0
	add	#1,r0
	and	#15,r0
	mov.b	r0,@r4
#endif

	UPDATE_APUTIMER_WITH_SHUTODOWN

.S9xUpdateAPUTimer_1salto\K:
	mov.l	APUTimerCounter, r0
	add	#1, r0
	mov.l	r0, APUTimerCounter
	cmp/eq	#8, r0
	bf	.S9xUpdateAPUTimer_bucle\K

	mov	#0, r0
	mov.l	r0, APUTimerCounter

	mov.b   @(APUTimerEnabled - APUTimer, r2), r0
	tst	r0, r0
	bt	.S9xUpdateAPUTimer_2salto\K

#ifdef USE_ASM_R5
	mov.w	@r2, r5
	add	#1, r5
	mov.w	r5, @r2

	extu.w	r5, r4
	mov.w 	@(APUTimerTarget - APUTimer, r2), r0
	extu.w	r0, r5
	cmp/hs  r5, r4
#else
	mov.w	@r2, r4
	add	#1, r4
	mov.w	r4, @r2

	extu.w	r4, r4
	mov.w 	@(APUTimerTarget - APUTimer, r2), r0
	extu.w	r0, r0
	cmp/hs  r0, r4
#endif
	bf	.S9xUpdateAPUTimer_bucle\K

	mov	#0, r0
	mov.w	r0, @r2
#ifdef USE_ASM_R5
	mov.l	APURAM, r5
	mov.l	.S9xUpdateAPUTimer_CONST000000fd, r4
	add	r4, r5
	mov.b	@r5,r0
	add	#1,r0
	and	#15,r0
	mov.b	r0,@r5
#else
	mov.l	.S9xUpdateAPUTimer_CONST000000fd, r4
	mov.l   APURAM, r0
	add	r0, r4
	mov.b	@r4,r0
	add	#1,r0
	and	#15,r0
	mov.b	r0,@r4
#endif

	UPDATE_APUTIMER_WITH_SHUTODOWN

.S9xUpdateAPUTimer_2salto\K:
	mov.b   @(APUTimerEnabled - APUTimer + 1, r2), r0
	tst	r0, r0
	bt	.S9xUpdateAPUTimer_bucle\K

	mov.w	@(2, r2), r0
	add	#1, r0
	mov.w	r0, @(2, r2)

	extu.w	r0, r4
	mov.w 	@(APUTimerTarget - APUTimer + 2, r2), r0
#ifdef USE_ASM_R5
	extu.w	r0, r5
	cmp/hs  r5, r4
#else
	extu.w	r0, r0
	cmp/hs  r0, r4
#endif
	bf.s	.S9xUpdateAPUTimer_bucle\K

	mov	#0, r0
	mov.w	r0, @(2, r2)
#ifdef USE_ASM_R5
	mov.l	APURAM, r5
	mov.l	.S9xUpdateAPUTimer_CONST000000fe, r4
	add	r4, r5
	mov.b	@r5,r0
	add	#1,r0
	and	#15,r0
	mov.b	r0,@r5
#else
	mov.l	.S9xUpdateAPUTimer_CONST000000fe, r4
	mov.l	APURAM, r0
	add	r0, r4
	mov.b	@r4,r0
	add	#1,r0
	and	#15,r0
	mov.b	r0,@r4
#endif

	UPDATE_APUTIMER_WITH_SHUTODOWN_i
	bra .S9xUpdateAPUTimer_bucle\K
	UPDATE_APUTIMER_WITH_SHUTODOWN_f

.S9xUpdateAPUTimer_salida\K:
.endm

