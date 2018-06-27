	.file	"diva.c"
	.text
	.p2align 4,,15
/*
unsigned long diva(unsigned long a[2],unsigned long c,unsigned long *rem);
*/
.globl diva64
	.type	diva64, @function
diva64:
.LFB2:
	movq	(%rdi), %rax
	movq	%rdx, %r8
	movq	8(%rdi), %rdx
	divq	%rsi
	movq	%rdx, (%r8)
	ret
.LFE2:
	.size	diva64, .-diva64
	.section	.eh_frame,"a",@progbits
.Lframe1:
	.long	.LECIE1-.LSCIE1
.LSCIE1:
	.long	0x0
	.byte	0x1
	.string	"zR"
	.uleb128 0x1
	.sleb128 -8
	.byte	0x10
	.uleb128 0x1
	.byte	0x3
	.byte	0xc
	.uleb128 0x7
	.uleb128 0x8
	.byte	0x90
	.uleb128 0x1
	.align 8
.LECIE1:
.LSFDE1:
	.long	.LEFDE1-.LASFDE1
.LASFDE1:
	.long	.LASFDE1-.Lframe1
	.long	.LFB2
	.long	.LFE2-.LFB2
	.uleb128 0x0
	.align 8
.LEFDE1:
	.ident	"GCC: (GNU) 4.1.2 20070115 (prerelease) (SUSE Linux)"
	.section	.note.GNU-stack,"",@progbits
