	.file	"mula.c"
	.text
.globl mula64
	.type	mula64, @function
mula64:
.LFB12:
/*
	imulq	%rsi, %rdi
	movq	%rdi, (%rdx)
	movq	$0, 8(%rdx)
*/
	movq    %rdi, %rax          /* rax = a */
	movq    %rdx, %rdi          /* rdi = c */
	mulq   %rsi                /* rdx:rax = rax * b */
	movq    %rax, (%rdi)        /* c[0] = rax */
	movq    %rdx, 8(%rdi)       /* c[1] = rdx */

	ret
.LFE12:
	.size	mula64, .-mula64
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
	.long	.LFB12
	.long	.LFE12-.LFB12
	.uleb128 0x0
	.align 8
.LEFDE1:
	.ident	"GCC: (GNU) 4.1.2 20070115 (prerelease) (SUSE Linux)"
	.section	.note.GNU-stack,"",@progbits
