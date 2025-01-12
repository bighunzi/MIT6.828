
# lab1

lab使用x86架构。

PC's power-on bootstrap procedure：PC的开机引导程序。

JOS 是6.828的kernel名字

## Introduction
`git diff`将显示自上次提交以来对代码的更改。

`git diff origin/lab1`将显示相对于为这个实验室提供的初始代码的更改。

[PC Assembly Language Book](https://pdos.csail.mit.edu/6.828/2018/readings/pcasm-book.pdf)中使用的是NASM汇编器，而项目使用GNU汇编器，二者差别见于[Brennan's Guide to Inline Assembly](http://www.delorie.com/djgpp/doc/brennan/brennan_att_inline_djgpp.html)。

本项目中使用的GNU汇编器使用AT&T/Unix语法:source在左，destination在右！！！

## PC Bootstrap
obj/kern/kernel.img是模拟PC的“虚拟硬盘”的内容，这个硬盘映像包含引导加载程序(obj/boot/boot)和内核(obj/kernel)。

虚拟VGA显示器其实就在qemu窗口上。

基本输入/输出系统(BIOS Basic Input/Output System)。在早期的pc中，BIOS保存在真正的只读存储器(ROM)中，但现在的pc将BIOS存储在可更新的闪存中。
BIOS负责执行基本的系统初始化，如激活显卡和检查内存量。在执行此初始化之后，BIOS从某些适当的位置(如软盘、硬盘、CD-ROM或网络)加载操作系统，并将计算机的控制传递给操作系统。

JOS将只使用PC的物理内存的前256MB。

### Exercise 2(The ROM BIOS)
```language
//当PC机启动时，CPU运行在实模式(real mode)下，而当进入操作系统内核后，将会运行在保护模式下(protected mode)。
//实模式下指令中出现的地址都是采用 (段基址：段内偏移)。
//但是由于8086CPU中寄存器都是16位，而CPU地址总线是20位的，所以把段寄存器中的值左移4位，形成20位段基址，然后和16位段内偏移相加，就得到了真实地址，即0xffff0的由来

[f000:fff0]    0xffff0:	ljmp   $0xf000,$0xe05b 
//可以看到IBM PC在物理地址0x000ffff0开始执行，它位于预留给ROM BIOS的64KB区域的最顶端（结合那个PC physical adress的图看）。
//一条跳转指令，跳转到0xfe05b地址处
//16*0xf000 + 0xfff0=0xffff0 

[f000:e05b]    0xfe05b:	cmpl   $0x0,%cs:0x6ac8
//把0x0这个立即数和$cs:0x6ac8所代表的内存地址处的值比较
//如果相等，会将ZF标志位置1，否则0

[f000:e062]    0xfe062:	jne    0xfd2e1
//jne指令：如果ZF标志位为0的时候跳转，
//即cs:0x6ac8地址处的值不是0x0时跳转。

[f000:e066]    0xfe066:	xor    %dx,%dx
//xor 异或
//0xfe066表明上面的跳转指令并没有跳转。这条指令的功能是把dx寄存器清零。

[f000:e068]    0xfe068:	mov    %dx,%ss
[f000:e06a]    0xfe06a:	mov    $0x7000,%esp
[f000:e070]    0xfe070:	mov    $0xf34c2,%edx
//设置寄存器的值

[f000:e076]    0xfe076:	jmp    0xfd15c
//跳转

[f000:d15c]    0xfd15c:	mov    %eax,%ecx
//寄存器

[f000:d15f]    0xfd15f:	cli
//关闭中断指令。这个比较好理解，启动时的操作是比较关键的，所以肯定是不能被中断的。这个关中断指令用于关闭那些可以屏蔽的中断。比如大部分硬件中断。

[f000:d160]    0xfd160:	cld    
//设置方向标识位为0，表示后续的串操作比如MOVS操作，内存地址的变化方向，如果为0代表从低地址值变为高地址

[f000:d161]    0xfd161:	mov    $0x8f,%eax

//out，in 是用来操作IO端口的（设备控制器当中的寄存器）
[f000:d167]    0xfd167:	out    %al,$0x70
//向 0x70 端口写入al寄存器的值
[f000:d169]    0xfd169:	in     $0x71,%al
//利用al寄存器 读取 0x71端口的值
//0070-0071	NMI（不可屏蔽中断） Enable / Real Time Clock

[f000:d16b]    0xfd16b:	in     $0x92,%al
//0090-009F	System devices
[f000:d16d]    0xfd16d:	or     $0x2,%al
//按位或
[f000:d16f]    0xfd16f:	out    %al,$0x92


[f000:d171]    0xfd171:	lidtw  %cs:0x6ab8
//lidt指令：加载中断向量表寄存器(IDTR)。
[f000:d177]    0xfd177:	lgdtw  %cs:0x6a74
//把从0xf6a74为起始地址处的6个字节的值加载到全局描述符表格寄存器中GDTR中。

[f000:d17d]    0xfd17d:	mov    %cr0,%eax
[f000:d180]    0xfd180:	or     $0x1,%eax
[f000:d184]    0xfd184:	mov    %eax,%cr0
/*计算机中包含CR0~CR3四个控制寄存器，用来控制和确定处理器的操作模式。其中这三个语句的操作明显是要把CR0寄存器的最低位(0bit)置1。
CR0寄存器的0bit是PE位，启动保护位，当该位被置1，代表开启了保护模式。
*/

//后面的应该就不是了
[f000:d187]    0xfd187:	ljmpl  $0x8,$0xfd18f
The target architecture is set to "i386".
=> 0xfd18f:	mov    $0x10,%eax
=> 0xfd194:	mov    %eax,%ds
=> 0xfd196:	mov    %eax,%es
=> 0xfd198:	mov    %eax,%ss
=> 0xfd19a:	mov    %eax,%fs
=> 0xfd19c:	mov    %eax,%gs
=> 0xfd19e:	mov    %ecx,%eax

```


综上，我们可以看到BIOS的操作就是在控制，初始化，检测各种底层的设备，比如时钟，GDTR寄存器。以及设置中断向量表。这都和Lab 1 Part 1.2最后两段说的一样。但是作为PC启动后运行的第一段程序，它最重要的功能是把操作系统从磁盘中导入内存，然后再把控制权转交给操作系统。所以BIOS在运行的最后会去检测可以从当前系统的哪个设备中找到操作系统，通常来说是我们的磁盘。也有可能是U盘等等。当BIOS确定了，操作系统位于磁盘中，那么它就会把这个磁盘的第一个扇区，通常把它叫做启动区（boot sector）先加载到内存中，这个启动区中包括一个非常重要的程序--boot loader，它会负责完成整个操作系统从磁盘导入内存的工作，以及一些其他的非常重要的配置工作。最后操作系统才会开始运行。

可见PC启动后的运行顺序为 BIOS -> boot loader -> 操作系统内核

## The Boot Loader
boot/boot.S源码：
参考 [Lab_1：练习3——分析bootloader进入保护模式的过程](https://www.cnblogs.com/cyx-b/p/11809742.html)  与其他收藏教程。

知识点：
test命令将两个操作数进行逻辑与运算，并根据运算结果设置相关的标志位。

boot/main.c源码：参考博客很详细，我就不复述了。

### Exercise 3
>练习3
看一下lab tools guide，特别是关于GDB命令的部分。其中包括一些难懂的GDB命令，这些命令对操作系统工作很有用。
在地址0x7c00处设置断点，这是装入引导扇区的位置。继续执行直到该断点。跟踪boot/boot.s中的代码，使用源代码和反汇编文件obj/boot/boot.asm追踪你在哪里。还可以使用GDB中的x/i命令来反汇编引导加载程序中的指令序列，并将原始引导加载程序源代码与obj/boot/boot.asm中的反汇编和GDB进行比较。
跟踪到boot/main.c中的bootmain()，然后跟踪到readsect()。确定与readsect()中的每条语句对应的汇编指令。跟踪readsect()的其余部分并返回到bootmain()，并确定从磁盘读取内核剩余扇区的for循环的开始和结束。找出在循环结束时将运行的代码，在那里设置断点，并继续执行到该断点。然后逐步执行引导加载程序的剩余部分。

> 问题1
处理器在什么时候开始执行32位代码?是什么导致了从16位模式切换到32位模式?

boot.s文件中  `ljmp    $PROT_MODE_CSEG, $protcseg`。
这条语句之前的几句开启了保护模式，这条语句跳转到了32位对应代码处。
是修改了CR0 bit0位导致了32位模式的开启

> 问题2
引导加载程序执行的最后一条指令是什么?它刚刚加载的内核的第一条指令是什么?

最后一条指令`call   *0x10018`。
第一条指令`movw   $0x1234,0x472`。

> 问题3
内核的第一条指令在哪里?

0x10000c处 ，即指令`movw $0x1234, 0x472` 位于/kern/entry.S文件中。


一个问题，在boot.asm文件中可以看出最后一行指令是 call *0x10018，那为什么入口是0x10000c?
```language
//答案在这
(gdb) x/10x 0x10018
0x10018:	0x0c	0x00	0x10	0x00	0x34	0x00	0x00	0x00
0x10020:	0x7c	0x52
```

> 问题4
引导加载程序如何决定为了从磁盘获取整个内核必须读取多少扇区?它在哪里找到这些信息?

main.c程序中通过ELFHDR指向的struct中的对象，也就是elf.h头文件中Program Header Table

### Exercise 4
>练习4
运行pointer.c的代码，运行它，并确保您理解所有打印值的来源。

看pointer.c的几行输出：
1,2,4,6行很简单，c语言基础。

3: 3[c]的写法很奇怪，没见过。

5:注意window是小端系统(数据的低位字节序的内容放在低地址处，即人读的顺序应该是从右到左)，所以拼接的时候注意顺序。
修改前a[1]=400=0x00000190。 按byte存放：90 01 00 00
      a[2]=302=0x0000012e  按byte存放：2e 01 00 00

修改后将 指针后移了1个byte 赋值500，即0x000001f4。 按byte存放：f4 01 00 00。
于是a[1]变为 90 f4 01 00(128144)   a[2]变为 00 01 00 00(256)



boot loader程序使用ELF程序头来决定如何加载各节。程序头指定了ELF对象的哪些部分需要加载到内存中，以及各个部分应该占用的目标地址。你可以输入以下命令查看程序头:`objdump -x obj/kern/kernel`。
然后，在objdump输出的“Program Headers”下列出程序头。ELF对象中需要加载到内存中的区域标记为“LOAD”。也给出了每个程序头的其他信息，如虚拟地址(“vaddr”)、物理地址(“paddr”)、加载区域的大小(“memsz”和“filesz”)。

回到boot/main.c，每个程序头的字段都包含段的目标物理地址(在本例中，它实际上是一个物理地址，尽管ELF规范对该字段的实际含义很模糊)。ph->p_pa

BIOS从地址0x7c00开始将引导扇区加载到内存中，因此这是引导扇区的加载地址。这也是启动扇区执行的地方，所以这也是它的链接地址。我们通过向boot/Makefrag中的链接器传递-Ttext 0x7C00来设置链接地址，这样链接器就会在生成的代码中生成正确的内存地址。

### Exercise 5
>练习5
再次跟踪引导加载程序的前几条指令，并确定如果引导加载程序的链接地址错误，将会“中断”或做错误事情的第一条指令。将boot/Makefrag中的链接地址更改为错误的地址，运行make clean，使用make重新编译该实验，并再次跟踪引导加载程序以查看发生了什么。最后别忘了把链接地址改回来，make clean。

指令`objdump -h obj/kern/kernel`检查内核可执行文件中所有节的名称、大小和链接地址的完整列表。

Link Address是指编译器指定代码和数据所需要放置的内存地址，由链接器配置。
Load Address是指程序被实际加载到内存的位置。

段的链接地址是段预期执行的内存地址。链接器以各种方式在二进制文件中编码链接地址，例如当代码需要一个全局变量的地址时，结果是如果从一个没有链接的地址执行，二进制文件通常无法工作。(可以生成位置无关的代码，其中不包含任何绝对地址。它被现代共享库广泛使用，但它有性能和复杂性成本，所以我们不会在6.828中使用它。)
[链接地址和加载地址的区别](https://blog.csdn.net/sgy1993/article/details/89281964) , 这个应该指的是位置无关代码！！！！！

通常，链接地址和加载地址是相同的。

修改下面这行地址，我将其改为0x7D00。
`$(V)$(LD) $(LDFLAGS) -N -e start -Ttext 0x7C00 -o $@.out $^  `


注意：BIOS将 boot loader加载到 0x7c00，所以咱们修改链接地址的结果是导致了boot.s以及后续的main.c的变化。


开始实验：
修改后重新编译，发生变化处：0x7c1e:	lgdtw  0x7d64

![0x7d64与0x7c64处信息对比](./MIT6828_img/lab1_exercise5_1.png)

上面这条指令是把指令后面的值所指定内存地址处后6个字节的值输入全局描述符表寄存器GDTR，但是当前这条指令读取的内存地址是0x7d64，我们在图中也展示了一下这个地址处与0x7c64处的差别。这是不对的，正确的应该是在0x7c64处存放的值，即图中最下面一样的值。可见，问题出在这里，GDTR表的值读取不正确，这是实现从实模式到保护模式转换的非常重要的一步。
进一步执行，到后面这条语句发现：程序由于跳转地址的错误，已经无法执行了。(本人猜测，可能因为并没有0x7d32地址对应的指令，所以无法跳转。从boot.asm文件中可以看出，7d30和7d33有指令，7d32并没有）

![程序已经无法执行](./MIT6828_img/lab1_exercise5_2.png)


回顾内核的加载地址和链接地址。与引导加载程序不同的是，这两个地址并不相同:内核告诉引导加载程序以低地址(1兆字节)将其加载到内存中，但它期望从高地址执行。我们将在下一节深入探讨如何实现这一功能。除了节信息，ELF头中还有一个字段对我们很重要，名为e_entry。这个字段保存了程序入口点的链接地址:程序文本部分中应该开始执行的内存地址。你可以看到入口点:`objdump -f obj/kern/kernel`现在你应该能够理解boot/main.c中的最小ELF加载器。它将内核的每个部分从磁盘读取到内存的加载地址，然后跳转到内核的入口点。
### Exercise 6
>练习6
在BIOS进入引导加载程序时检查0x00100000处8个字的内存，然后在引导加载程序进入内核时检查一次。为什么它们不同?第二个断点是什么?。


```
//at the point the BIOS enters the boot loader：
0x100000:	0x00000000	0x00000000	0x00000000	0x00000000
0x100010:	0x00000000	0x00000000	0x00000000	0x00000000

//at the point the boot loader enters the kernel：
0x100000:	0x1badb002	0x00000000	0xe4524ffe	0x7205c766
0x100010:	0x34000004	0x1000b812	0x220f0011	0xc0200fd8
```

他们为什么不同？
bootmain中最后一句加载内核的程序是((void (*)(void)) (ELFHDR->e_entry))()， 其中e_entry字段的含义是这个可执行文件的第一条指令的虚拟地址。所以这句话的含义就是把控制权转移给操作系统内核。
在这之前bootmain已经把kernel程序装载到0x10000处了，从main.c就可以看出来，这几处有了改变。


在第二个断点处的东西是什么？
是kernel程序。

## The Kernel
现在，我们只映射前4MB的物理内存，这足以让我们启动和运行。我们使用kern/entrypgdir.c中手写的、静态初始化的页目录和页表来实现这一点。


JOS只使用前256MB的物理内存，因为我们把从物理地址0x00000000到0x0fffffff，分别映射到虚拟地址0xf0000000到0xffffffff。

### Exercise 7
注：从Exercise 3可以看出来 kernel 的入口地址是0x10000c。

>练习7
使用QEMU和GDB跟踪到JOS内核，并停止在`movl %eax， %cr0`。检查0x00100000和0xf0100000的内存。现在，使用stepi GDB命令对该指令进行单步执行。再次检查0x00100000和0xf0100000的内存。确保你明白刚刚发生了什么。建立新映射后，如果映射不到位就不能正常工作的第一条指令是什么?注释掉kern/entry.s中的`movl %eax， %cr0`，追踪到它，看看你是否正确。

在`movl %eax, %cr0`停止。
执行完该句的前一句指令，si提示下一句为movl %eax, %cr0时：
```language
at 0x00100000:
(gdb) x/10x 0x00100000 
0x100000:	0x1badb002	0x00000000	0xe4524ffe	0x7205c766
0x100010:	0x34000004	0x1000b812	0x220f0011	0xc0200fd8
0x100020:	0x0100010d	0xc0220f80

at 0xf0100000:
(gdb) x/10x 0xf0100000
0xf0100000 <_start-268435468>:	0x00000000	0x00000000	0x00000000	0x00000000
0xf0100010 <entry+4>:	0x00000000	0x00000000	0x00000000	0x00000000
0xf0100020 <entry+20>:	0x00000000	0x00000000
```

使用stepi (si命令显示的下一行将要执行的指令！) GDB命令执行完该指令.
```language
at 0x00100000:
(gdb) x/10x 0x00100000 
0x100000:	0x1badb002	0x00000000	0xe4524ffe	0x7205c766
0x100010:	0x34000004	0x1000b812	0x220f0011	0xc0200fd8
0x100020:	0x0100010d	0xc0220f80

at 0xf0100000:
(gdb) x/10x 0xf0100000
0xf0100000 <_start-268435468>:	0x1badb002	0x00000000	0xe4524ffe	0x7205c766
0xf0100010 <entry+4>:	0x34000004	0x1000b812	0x220f0011	0xc0200fd8
0xf0100020 <entry+20>:	0x0100010d	0xc0220f80
```
可见原本存放在0xf0100000处地址空间，已经被映射到0x00100000处了，这样我们才能看到它所存储的内容与0x00100000处相同了。

> 问题
建立新映射后，如果映射不到位就不能正常工作的第一条指令是什么?

将entry.S文件中的%movl %eax, %cr0这句话注释掉，进行尝试：

![尝试结果](.\MIT6828_img/lab1_exercise7_1.png)

其中在0x10002a处的jmp指令，要跳转的位置是0xf010002C，由于没有进行映射，此时不会进行虚拟地址到物理地址的转化。所以报出错误。

### Exercise 8
> Exercise 8
程序中省略了一小段代码——使用“%o”形式的模式打印八进制数所必需的代码。找到并填写此代码片段。

分析一下三个文件：
printf.c文件中有三个函数: 
* putch()调用consol.c中的cputchar(), 
* vcprintf()调用 printfmt.c中的 vprintfmt(), 
* cprintf()调用vcprintf().

先看短的：
* 先分析console.c：
这个文件中定义了如何把一个字符显示到console上，即我们的显示屏之上，里面包括很多对IO端口的操作。
* 最重要的cputchar函数：其调用cons_putc,而根据注释,后者的功能是输出一个字符到控制台(计算机的屏幕)。（注意：putch主体是调用cputchar）

* 再看printfmt.c:
文件注释：精简的原语printf风格的格式化例程，通常由printf、sprintf、fprintf等使用。内核程序和用户程序也使用此代码。
其中根据注释，printfmt()函数是格式化和打印字符串的主要函数，而其调用vprintfmt函数，其有4个参数，如下：
(1)void (*putch)(int, void*)：
这个参数是一个函数指针，这类函数包含两个输入参数int, void*，int参数代表一个要输出的字符的值。void* 则代表要把这个字符输出的位置的地址
(2)void *putdat
这个参数就是输入的字符要存放在的内存地址的指针，就是和上面putch函数的第二个输入参数是一个含义。
(3)const char *fmt
这个参数代表你在编写类似于printf这种格式化输出程序时，你指定格式的字符串，即printf函数的第一个输入参数，比如printf("This is %d test", n)，这个子程序中，fmt就是"This is %d test"。
(4)va_list ap
这个参数代表的是多个输入参数，即printf子程序中从第二个参数开始之后的参数，比如("These are %d test and %d test", n, m)，那么ap指的就是n，m

答案：省略的部分在 printfmt.c文件208行处，修改为：
```c
case 'o':
	// Replace this with your code.
	//imitate (unsigned) hexadecimal and unsigned decimal part
	num = getuint(&ap, lflag);
	base=8;
	goto number;
	//no break,because "break" is in number:
```

并回答下列问题：
> 问题1
解释printf.c和console.c之间的接口。具体来说，console.c提供了什么函数?printf.c如何使用这个函数?

printf.c中putch()调用consol.c中的cputchar()
用来向显示屏上显示字符。

> 问题2
2.解释下列console.c中的代码：

crt_buf:这是一个字符数组缓冲区，里面存放着要显示到屏幕上的字符
crt_pos:这个表示当前最后一个字符显示在屏幕上的位置。
当crt_pos >= CRT_SIZE，其中CRT_SIZE = 80*25，由于我们知道crt_pos取值范围是0 ~ (80 * 25 - 1)，那么这个条件如果成立则说明现在在屏幕上输出的内容已经超过了一页。所以此时要把页面向上滚动一行，即把原来的1 ~ 79号行放到现在的0 ~ 78行上，然后把79号行换成一行空格（当然并非完全都是空格，0号字符上要显示你输入的字符int c）。所以memcpy操作就是把crt_buf字符数组中1 ~ 79号行的内容复制到0 ~ 78号行的位置上。而紧接着的for循环则是把最后一行，79号行都变成空格。最后还要修改一下crt_pos的值。

> 问题3
3.在调用cprintf()时，fmt指向什么?ap指向什么?

在kern/moniter.c  mon_backtrace()函数中添加了这3，4，5题的代码并调试。

从boj/kernel.asm中可以看到mon_backtrace()入口在 0xf0100877（1055行），而后续练习的代码自0xf0100895开始，所以将断点设置在此，然后开始调试。
```language
(gdb) p fmt
$1 = 0xf0101d8e "x %d, y %x, z %d\n"

//ap的值和类型
(gdb)  p ap
$2 = (va_list) 0xf010eef4 "\001"

//ap类型为char *，所以后续要转换类型
(gdb) ptype ap
type = char *

(gdb) p ((int*)ap)[0]
$3 = 1

(gdb) p ((int*)ap)[1]
$15 = 3

(gdb) p ((int*)ap)[2]
$16 = 4
```

可以看出fmt 指向的是"x %d, y %x, z %d\n" 字符串，ap会指向所有输入参数的集合，即（1,3,4）。


> 问题3
列出(按执行顺序)对con_putc、va_arg和vcprintf的每个调用。对于con_putc，也列出它的参数。对于va_arg，列出ap在调用前后所指向的内容。对于vcprintf，列出其两个参数的值。

注：根据c文件判断 cprintf()函数执行的调用顺序应该是(只列出与题干有关的函数):
* vcprintf(fmt, ap)->vprintfmt->putch()->cputchar()->cons_putc(c)执行完成后
* vprintfmt下一部分->getint/getuint()->va_arg(*ap,long long)

```
//cons_putc and its arguments
(gdb) si
=> 0xf0100364 <cons_putc>:	push   %ebp
cons_putc (c=120) at kern/console.c:434
434	{
//ascii码表中 120代表字符x

//va_arg
//从kernel.asm发现断点在0xf01011c1
//ap before va_arg
=> 0xf01011c1 <vprintfmt+785>:	mov    0x14(%ebp),%eax
Breakpoint 1, vprintfmt (putch=0xf0100a52 <putch>, putdat=0xf010ef8c, fmt=0xf0101aa0 "entering test_backtrace %d\n", ap=0xf010efc4 "\005") at lib/printfmt.c:75
75			return va_arg(*ap, int);

(gdb) p ((int*)ap)[0]
$2 = 5
//不明白！

//ap after va_arg
=> 0xf01011d6 <vprintfmt+806>:	jmp    0xf010118c <vprintfmt+732>
0xf01011d6	75			return va_arg(*ap, int);
(gdb) p ((int*)ap)[0]
$7 = 0


//vcprintf and its two arguments
(gdb) si
=> 0xf0100a80 <vcprintf+12>:	add    $0xf888,%ebx
0xf0100a80 in vcprintf (fmt=0xf0101dae "x %d, y %x, z %d\n", ap=0xf010eee4 "\001") at kern/printf.c:18
18	{
//与上个小问结果相同。
```


> 问题4
输出是什么?解释如何按照前面练习的步骤一步一步地得到这个输出。

输出：He110 World。
%x按16位格式输出，57616 16进制为e110。
%s按格式输出字符串，但是c语言中没有字符串，只有字符数组，那么该题中显然将int*指针所指的对象作为字符数组输出了。
但是注意小端系统 int 0x 00 64 6c 72（按ASCII分别对应：空字符,d,l,r.）按byte存放的顺序是反着的，所以现在按照地址递增打印字符的结果就是rld.

> 问题4
这个输出取决于x86是小端方式的这一事实。如果x86是大端，为了产生相同的输出，你会将i设置为什么?是否需要将57616更改为不同的值?

lrd会反过来。
但是e110不会，因为读取的结果就是这个数本身，不会按byte拆分。

> 问题5
在下面的代码中，'y='后将打印什么?(注意:答案不是一个具体的值。)为什么会发生这种情况?

输出： x=3 y=-267325684
由于y并没有参数被指定，所以会输出一个不确定的值。

> 问题6
假设GCC改变了它的调用约定，将参数按声明顺序推入堆栈，这样最后一个参数就被推到最后。您必须如何更改cprintf或它的接口，以便仍然可以向它传递可变数量的参数?

在原接口的最后增加一个int型参数，用来记录所有参数的总长度，这样我们可以根据栈顶元素找到格式化字符串的位置。这种方法需要计算所有参数的总长度，比较麻烦。。。

### Exercise 9

> 问题
确定内核初始化堆栈的位置，以及堆栈在内存中的确切位置。内核如何为堆栈预留空间?初始的栈指针指向这个保留区域的哪个“端” ?

从kernel.asm可以看出来 kernel中的entry.s 从此处初始化栈，esp位于0xf010f000。（可能因为我之前在exercise8中改过monitor.c的文件，所以和别人的结果不一样。 esp指向的是整个堆栈中正在被使用的部分的最低地址。在这个地址之下的更低的地址空间都是还没有被利用的堆栈空间。）

继续往下看，找到bootstack标签，其中.space KSTKSIZE语句申请了大小为KSTKSIZE = 8 * PGSIZE = 8 * 4096 字节、初始值全为0的栈空间。再往后定义了bootstacktop标签，可见栈顶位置处于栈的最高地址上，而栈指针指向栈顶，亦即指向栈的最高地址，这也说明栈是由上到下（高地址向低地址）生长的
```asm
movl	$0x0,%ebp			# nuke frame pointer
f010002f:	bd 00 00 00 00       	mov    $0x0,%ebp
	# Set the stack pointer
	movl	$(bootstacktop),%esp
f0100034:	bc 00 f0 10 f0       	mov    $0xf010f000,%esp


.data
###################################################################
# boot stack
###################################################################
	.p2align	PGSHIFT		# force page alignment
	.globl		bootstack
bootstack:
	.space		KSTKSIZE
	.globl		bootstacktop   
bootstacktop:
```
lab中也提到了 esp指向的是栈的低地址端


### Exercise 10

> 问题
要熟悉x86上的C调用约定，可以在obj/kern/kernel.asm中找到test_backtrace函数的地址，在那里设置一个断点，并检查每次内核启动后调用它时会发生什么。test_backtrace的每个递归嵌套层在堆栈上推了多少个32位单词，这些单词是什么?

test_backtrace函数开始于 f0100040
其c程序是这样的：
```c
void
test_backtrace(int x)
{
	cprintf("entering test_backtrace %d\n", x);
	if (x > 0)
		test_backtrace(x-1);
	else
		mon_backtrace(0, 0, 0);
	cprintf("leaving test_backtrace %d\n", x);
}
//可以看出这个函数就是一个递归调用，一直调用自己直到x=0，调用monitor.c中的mon_backtrace(0, 0, 0)，也就是我们之前修改过的文件。
```

其实后续利用si步进执行的结果和asm文件中没什么区别，所以直接看asm文件就可以了
```language
f0100040:	55                   	push   %ebp
f0100041:	89 e5                	mov    %esp,%ebp
f0100053:	83 ec 08             	sub    $0x8,%esp
//与栈相关的指令只有上述三条，过程与lab前面的描述也一致，即将ebp压栈，将esp赋给ebp，再减小esp。
//第三行指令减小esp是为了调用cprintf()

//而后开始调用cprintf()
//而在f0100063 即if(x>0)开始判断，之后就如递归调用
//与递归调用有关的指令：
f010006a:	83 ec 0c             	sub    $0xc,%esp

//与上面调用cprinf（）的差别只不过在于esp移动的距离不同了，
```

### Exercise 11
后面要利用monitor.c中的mon_backtrace函数了，所以注释掉之前修改的代码，然后重新编译

> 问题
实现上面指定的回溯函数。使用与示例中相同的格式，否则评分脚本将被混淆。当您认为它可以正常工作时，运行make grade来查看它的输出是否符合我们的评分脚本的要求，如果不符合则修复它。在您提交Lab 1代码后，欢迎您以任何您喜欢的方式更改回溯函数的输出格式。

ebp值表示进入该函数之前使用的堆栈的基指针，eip值是函数的返回指令指针。
至于为什么ebp,eip，args分布在栈上这些位置，看下图：

![栈帧结构](.\MIT6828_img/lab1_exercise11_栈帧结构.png)

一定要记住ebp寄存器中保存的是指向栈的指针！另外， 代码中的类型转换也要注意！  循环的终止条件是前面练习中得出的结论，ebp寄存器中初始值是0x00,代码如下：
```c
	uint32_t * ebp;
	ebp=(uint32_t *)read_ebp();
	cprintf("Stack backtrace:\n");
	while((uint32_t)ebp != 0x0){//the first ebp value is 0x0
		//Exercise 11
		cprintf(" ebp %08x",(uint32_t) ebp);
		cprintf(" eip %08x",*(ebp+1));
		cprintf(" args");
		// 函数压栈顺序为从右至左！！！
		cprintf(" %08x",*(ebp+2));
		cprintf(" %08x",*(ebp+3));
		cprintf(" %08x",*(ebp+4));
		cprintf(" %08x",*(ebp+5));
		cprintf(" %08x\n",*(ebp+6));
		//
		ebp=(uint32_t *)(*ebp);
	}
```

### Exercise 12
> 问题
Modify your stack backtrace function to display, for each eip, the function name, source file name, and line number corresponding to that eip.

利用kern/kdebug.c.中的debuginfo_eip()函数即可，其注释清晰的表明了函数功能。同时debuginfo_eip()函数还需要我们调用stab_binsearch 来完成对line number 的查询。

先回答一些问题：
> 问题
在debuginfo_eip中，__STAB_*来自哪里?这个问题的答案很长;为了帮助你找到答案，这里有一些你可能需要做的事情:
- look in the file kern/kernel.ld for __STAB_*
- run objdump -h obj/kern/kernel
- run objdump -G obj/kern/kernel
- run gcc -pipe -nostdinc -O2 -fno-builtin -I. -MD -Wall -Wno-format -DJOS_KERNEL -gstabs -c -S kern/init.c, and look at init.s.
- see if the bootloader loads the symbol table in memory as part of loading the kernel binary

1. 从kernel.ld文件中可以看到：
```
	.stab : {
		PROVIDE(__STAB_BEGIN__ = .);
		*(.stab);//*符号代表任意输入文件，该句指令的意思应该是任何文件的stab段都放在这个部分
		PROVIDE(__STAB_END__ = .);
		BYTE(0)		/* Force the linker to allocate space for this section */
	}
	.stabstr : {
		PROVIDE(__STABSTR_BEGIN__ = .);
		*(.stabstr);
		PROVIDE(__STABSTR_END__ = .);
		BYTE(0)		/* Force the linker to allocate space for this section */
	}
//注：ld文件是链接脚本文件后缀，其中：
//PROVIDE(symbol = expression)  用于：在某些情况下，链接器脚本只需要定义一个被引用的符号，并且该符号不是由链接中包含的任何对象定义的。

//调试信息的传统格式被称为 STAB（符号表）。STAB 信息保存在 ELF 文件的 .stab 和 .stabstr 部分。
//.stab节：符号表部分，这一部分的功能是程序报错时可以提供错误信息
//.stabstr节：符号表字符串部分。
//stab(符号表字符串)是一种调试数据格式，用于存储有关计算机程序的信息，供符号和源代码级调试器使用。
//汇编程序创建两个自定义部分，一个名为.stab的部分包含一个固定长度的结构数组，每个stab有一个结构，另一个名为.stabstr的部分包含由.stab部分中的stab引用的所有可变长度字符串。
```

2.`objdump -h obj/kern/kernel`运行结果（显示文件的整体头部摘要信息）：

![运行结果](.\MIT6828_img/lab1_exercise12_1.png)

可以看到.stab段加载地址是 0x0010227c，size是0x0000327c
.stabstr段加载地址是0x00105bad  size是00006bad

  
3.`objdump -G obj/kern/kernel`的运行结果（-G, --stabs Display (in raw form) any STABS info in the file 即可以看到kernel的.stab节的内容）：（太长了，只截取一部分）
```language
bighunzi@bighunzi-VirtualBox:~/MIT6.828/course_rep/lab$ objdump -G obj/kern/kernel
obj/kern/kernel：     文件格式 elf32-i386
.stab 节的内容：
Symnum n_type n_othr n_desc n_value  n_strx String
-1     HdrSym  0      1206   00001530  1     
0      SO      0      0      f0100000  1     {standard input}
1      SOL     0      0      f010000c  18    kern/entry.S
2      SLINE   0      44     f010000c  0      
3      SLINE   0      57     f0100015  0      
4      SLINE   0      58     f010001a  0      
5      SLINE   0      60     f010001d  0      
6      SLINE   0      61     f0100020  0    
//据博客说：
//Symnum是符号索引，换句话说，整个符号表看作一个数组，Symnum是当前符号在数组中的下标
//n_type是符号类型，FUN指函数名，SLINE指在text段中的行号
//n_othr目前没被使用，其值固定为0
//n_desc表示在文件中的行号
//n_value表示地址。特别要注意的是，这里只有FUN类型的符号的地址是绝对地址，SLINE符号的地址是偏移量（这一点会在kdebug.c中的158行对addr做差的指令有关），其实际地址为函数入口地址加上偏移量。
```

4.`gcc -pipe -nostdinc -O2 -fno-builtin -I. -MD -Wall -Wno-format -DJOS_KERNEL -gstabs -c -S kern/init.c`生成的init.s文件：
```
//太长了，在这里不记录了
```

5.确认boot loader在加载内核时是否把符号表（stab）也加载到内存中。
问题2中我们已经知道了stab的加载地址0x0010227c，stabstr的加载地址是0x00105bad，所以我们只要看那块地址的信息就可以了
```
//还是同之前一样在kernel的入口0x10000c处设置断点，然后利用命令查看对应地址
(gdb) x/8s 0x0010227c
0x10227c:	"\001"
0x10227e:	""
0x10227f:	""
0x102280:	""
0x102281:	""
0x102282:	"\303\004(\026"
0x102287:	""
0x102288:	"\001"

(gdb) x/8s 0x00105bad
0x105bad:	""
0x105bae:	"{standard input}"
0x105bbf:	"kern/entry.S"
0x105bcc:	"kern/entrypgdir.c"
0x105bde:	"gcc2_compiled."
0x105bed:	"entry_pgdir:G(0,1)=ar(0,2)=r(0,2);0;4294967295;;0;1023;(0,3)=(0,4)=(0,5)=r(0,5);0;4294967295;"
0x105c4b:	"pde_t:t(0,3)"
0x105c58:	"uint32_t:t(0,4)"
//我运行的结果是这样的，其他博客上记录的都是第二个结果，我现在也不太明白stab段 和stabstr的区别。。。。。。。。
```
根据结果，显然是加载进来了。。。


debuginfo_eip()的修改部分：
struct Eipdebuginfo数据结构在kdebug.h中定义。
理解stabs每行记录的含义后，调用stab_binsearch便能找到某个地址对应的行号了，stab_binsearch函数的注释也表明了stab_binsearch返回的就是行号。
由于前面的代码已经找到地址在哪个函数里面以及函数入口地址，将原地址减去函数入口地址即可得到偏移量，再根据偏移量在符号表中的指定区间查找对应的记录即可。代码如下所示:
```c
	stab_binsearch(stabs, &lline, &rline, N_SLINE, addr);
	if (lline <= rline) {
    		info->eip_line = stabs[lline].n_desc;//n_desc表示在文件中的行号
	} else {
    		return -1;
	}	
```

mon_backtrace()的修改部分，其实上个部分写出来了这个就简单了：
```c
	// Your code here.
	uint32_t * ebp;
	struct Eipdebuginfo info;
	ebp=(uint32_t *)read_ebp();
	cprintf("Stack backtrace:\n");

	while((uint32_t)ebp != 0x0){//the first ebp value is 0x0
		//Exercise 11
		cprintf(" ebp %08x",(uint32_t) ebp);
		cprintf(" eip %08x",*(ebp+1));
		cprintf(" args");
		// the order of arguments is reverse in the stack. For example, func(a,b), in the stack is b,a.
		cprintf(" %08x",*(ebp+2));
		cprintf(" %08x",*(ebp+3));
		cprintf(" %08x",*(ebp+4));
		cprintf(" %08x",*(ebp+5));
		cprintf(" %08x\n",*(ebp+6));

		//Exercise 12
		debuginfo_eip(*(ebp+1) , &info);
		cprintf("\t%s:",info.eip_file);
		cprintf("%d: ",info.eip_line);
		cprintf("%.*s+%d\n", info.eip_fn_namelen , info.eip_fn_name , *(ebp+1) - info.eip_fn_addr );

		//
		ebp=(uint32_t *)(*ebp);
	}

```


注：感觉文件从.c最终到二进制可执行文件的过程没怎么讲明白。例如elf头，stab,stabstr段到底是干嘛的？ 以后有空回来补一下。
