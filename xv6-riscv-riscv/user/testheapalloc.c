//内存管理实验修改13：测试分配功能
#include"../kernel/types.h"
#include"../kernel/stat.h"
#include"user.h"
int main(int argc,char *argv[])
{
    testheapalloc();
    // //测试1：最大内存空间分配：16MB-48B=16777168B
    // printf("test1:\n");
    // heapinit();
    // void *p=heapalloc(16777168);
    // showalloclist();
    // showfreelist();
    // void *q=getblock((uint64)p);
    // printf("数据起始地址:%p\n",q);

    // //测试2：超出最大内存空间分配：>16777168B
    // printf("\n");
    // printf("test2:\n");
    // heapinit();
    // void *r=heapalloc(16777200);
    // showalloclist();
    // showfreelist();
    // void *s=getblock((uint64)r);
    // printf("数据起始地址:%p\n",s);

    // //测试3：正常分配释放测试：
    // //依次分配  1000B，2000B，3000B，4000B
    // //后释放   1000B，3000B
    // //再分配   4000B
    // printf("\n");
    // printf("test3:\n");
    // heapinit();
    // void *a=heapalloc(1000);
    // //void *b=
    // heapalloc(2000);
    // void *c=heapalloc(3000);
    // //void *d=
    // heapalloc(4000);
    // heapfree(a);
    // heapfree(c);
    // showfreelist();
    // //void *e=
    // heapalloc(4000);
    // showfreelist();

    // //测试4：紧凑功能测试1(分配)
    // //依次分配 2MB，3MB，4MB，5MB
    // //后释放   2MB，4MB
    // //在尝试分配 8MB
    // //在尝试分配 7MB
    // printf("\n");
    // printf("test4:\n");
    // heapinit();
    // void *h=heapalloc(2*1024*1024);
    // //void *i=
    // heapalloc(3*1024*1024);
    // void *j=heapalloc(4*1024*1024);
    // //void *k=
    // heapalloc(5*1024*1024);
    // heapfree(h);
    // heapfree(j);
    // heapalloc(8*1024*1024);
    // heapalloc(7*1024*1024);
    // showalloclist();
    // showfreelist();
    // //测试5：紧凑功能测试2(移动数据块重定位)
    // //依次分配 1MB，2MB，3MB，4MB，5MB
    // //释放    1MB，3MB
    // //尝试分配 4.5MB
    // printf("\n");
    // printf("test5:\n");
    // heapinit();
    // void *t1=heapalloc(1024*1024);
    // void *t2=heapalloc(2*1024*1024);
    // void *t3=heapalloc(3*1024*1024);
    // void *t4=heapalloc(4*1024*1024);
    // void *t5=heapalloc(5*1024*1024);
    // showalloclist();
    // printf("t1数据块起始位置为:%p\n",getblock((uint64)t1));
    // printf("t2数据块起始位置为:%p\n",getblock((uint64)t2));
    // printf("t3数据块起始位置为:%p\n",getblock((uint64)t3));
    // printf("t4数据块起始位置为:%p\n",getblock((uint64)t4));
    // printf("t5数据块起始位置为:%p\n",getblock((uint64)t5));
    // heapfree(t1);
    // heapfree(t3);
    // void *t6=heapalloc(4.5*1024*1024);
    // showalloclist();
    // printf("t2数据块起始位置为:%p\n",getblock((uint64)t2));
    // printf("t4数据块起始位置为:%p\n",getblock((uint64)t4));
    // printf("t5数据块起始位置为:%p\n",getblock((uint64)t5));
    // printf("t6数据块起始位置为:%p\n",getblock((uint64)t6));
    
    //测试6：释放时相邻空闲块合并
    //依次分配 1000，2000，3000，4000
    //1.释放1000后2000
    //2.释放2000后1000
    //3.释放1000，3000后2000
    // printf("\n");
    // printf("test6:\n");
    // heapinit();
    // void *s1=heapalloc(1000);
    // void *s2=heapalloc(2000);
    // void *s3=heapalloc(3000);
    // heapalloc(4000);
    // heapfree(s1);
    // heapfree(s2);
    // showfreelist();
    // showalloclist();
    // void *s12=heapalloc(1000);
    // void *s22=heapalloc(2000);
    // heapfree(s22);
    // heapfree(s12);
    // showfreelist();
    // showalloclist();
    // void *s13=heapalloc(1000);
    // void *s23=heapalloc(2000);
    // heapfree(s13);
    // heapfree(s3);
    // heapfree(s23);

    // showfreelist();
    // showalloclist();

    // //测试7：用户对于分配到的内存块的写入
    // //先为用户分配1000B空间
    // //数据存放模式为大端模式
    // //在数据字段0字节处写入65536
    // //在数据字段16字节处写入65535
    // printf("\n");
    // printf("test7:\n");
    // heapinit();
    // void *o=heapalloc(1000);
    // writeToHeap(o,0,65536);
    // writeToHeap(o,16,65535);
    exit(0);
}