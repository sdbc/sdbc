/**************************************************
 * from /usr/include/sys/ucontext.h
 * depend on the system linux
 **************************************************/
#include <ucontext.h>

#ifndef REG_ERR

#if __WORDSIZE == 64
enum
{
  REG_R8 = 0,
# define REG_R8         REG_R8
  REG_R9,
# define REG_R9         REG_R9
  REG_R10,
# define REG_R10        REG_R10
  REG_R11,
# define REG_R11        REG_R11
  REG_R12,
# define REG_R12        REG_R12
  REG_R13,
# define REG_R13        REG_R13
  REG_R14,
# define REG_R14        REG_R14
  REG_R15,
# define REG_R15        REG_R15
  REG_RDI,
# define REG_RDI        REG_RDI
  REG_RSI,
# define REG_RSI        REG_RSI
  REG_RBP,
# define REG_RBP        REG_RBP
  REG_RBX,
# define REG_RBX        REG_RBX
  REG_RDX,
# define REG_RDX        REG_RDX
  REG_RAX,
# define REG_RAX        REG_RAX
  REG_RCX,
# define REG_RCX        REG_RCX
  REG_RSP,
# define REG_RSP        REG_RSP
  REG_RIP,
# define REG_RIP        REG_RIP
  REG_EFL,
# define REG_EFL        REG_EFL
  REG_CSGSFS,           // Actually short cs, gs, fs, __pad0.  
# define REG_CSGSFS     REG_CSGSFS
  REG_ERR,
# define REG_ERR        REG_ERR
  REG_TRAPNO,
# define REG_TRAPNO     REG_TRAPNO
  REG_OLDMASK,
# define REG_OLDMASK    REG_OLDMASK
  REG_CR2
# define REG_CR2        REG_CR2
};

#else

enum
{
  REG_GS = 0,
# define REG_GS         REG_GS
  REG_FS,
# define REG_FS         REG_FS
  REG_ES,
# define REG_ES         REG_ES
  REG_DS,
# define REG_DS         REG_DS
  REG_EDI,
# define REG_EDI        REG_EDI
  REG_ESI,
# define REG_ESI        REG_ESI
  REG_EBP,
# define REG_EBP        REG_EBP
  REG_ESP,
# define REG_ESP        REG_ESP
  REG_EBX,
# define REG_EBX        REG_EBX
  REG_EDX,
# define REG_EDX        REG_EDX
  REG_ECX,
# define REG_ECX        REG_ECX
  REG_EAX,
# define REG_EAX        REG_EAX
  REG_TRAPNO,
# define REG_TRAPNO     REG_TRAPNO
  REG_ERR,
# define REG_ERR        REG_ERR
  REG_EIP,
# define REG_EIP        REG_EIP
  REG_CS,
# define REG_CS         REG_CS
  REG_EFL,
# define REG_EFL        REG_EFL
  REG_UESP,
# define REG_UESP       REG_UESP
  REG_SS
# define REG_SS REG_SS
};

#endif
#endif

