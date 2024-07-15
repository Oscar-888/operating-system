// System call numbers
#define SYS_fork    1
#define SYS_exit    2
#define SYS_wait    3
#define SYS_pipe    4
#define SYS_read    5
#define SYS_kill    6
#define SYS_exec    7
#define SYS_fstat   8
#define SYS_chdir   9
#define SYS_dup    10
#define SYS_getpid 11
#define SYS_sbrk   12
#define SYS_sleep  13
#define SYS_uptime 14
#define SYS_open   15
#define SYS_write  16
#define SYS_mknod  17
#define SYS_unlink 18
#define SYS_link   19
#define SYS_mkdir  20
#define SYS_close  21

//系统调用实验修改5：为自定义的系统调用分配系统调用号
#define SYS_getProcessNumber 22
//内存管理实验修改9：为测试堆分配函数分配系统调用号
#define SYS_testheapalloc 23

#define SYS_heapinit      24
#define SYS_heapalloc     25
#define SYS_heapfree      26

#define SYS_showalloclist 27
#define SYS_showfreelist  28

#define SYS_getblock      29

#define SYS_writeToHeap   30