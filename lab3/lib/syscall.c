// System call stubs.
//这个文件中的其他函数sys_cputs(),sys_cgetc(),sys_env_destroy(),sys_getenvid()全都是在以不同参数调用syscall().

//inc/syscall.h中以enum 形式定义了SYS_cgetc等符号。

#include <inc/syscall.h>
#include <inc/lib.h>


//内核中的syscall()。
static inline int32_t
syscall(int num, int check, uint32_t a1, uint32_t a2, uint32_t a3, uint32_t a4, uint32_t a5)
{
	int32_t ret;

	// Generic system call: pass system call number in AX,
	// up to five parameters in DX, CX, BX, DI, SI.
	// Interrupt kernel with T_SYSCALL.
	//
	// The "volatile" tells the assembler not to optimize
	// this instruction away just because we don't use the
	// return value.
	//
	// The last clause tells the assembler that this can
	// potentially change the condition codes and arbitrary
	// memory locations.

	//int指令导致处理器中断，int $0x30作为系统调用中断。注意这已经是在JOS内核中了。 
	//这个函数好像一定返回0.
	asm volatile("int %1\n"
		     : "=a" (ret) /*将返回值传递给%eax*/
		     : "i" (T_SYSCALL),
		       "a" (num),/*将系统调用编号传入%eax*/
		       "d" (a1),/*依次将参数传递给%eax %edx %ecx %ebx %edi %esi 也就是lab3文档中描述的那样*/
		       "c" (a2),
		       "b" (a3),
		       "D" (a4),
		       "S" (a5)
		     : "cc", "memory");

	if(check && ret > 0)
		panic("syscall %d returned %d (> 0)", num, ret);

	return ret;
}

void
sys_cputs(const char *s, size_t len)
{
	syscall(SYS_cputs, 0, (uint32_t)s, len, 0, 0, 0);
}

int
sys_cgetc(void)
{
	return syscall(SYS_cgetc, 0, 0, 0, 0, 0, 0);
}

int
sys_env_destroy(envid_t envid)
{
	return syscall(SYS_env_destroy, 1, envid, 0, 0, 0, 0);
}

envid_t
sys_getenvid(void)
{
	 return syscall(SYS_getenvid, 0, 0, 0, 0, 0, 0);
}

