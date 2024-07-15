//内存管理实验修改5：实现堆中的动态内存分配
#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "riscv.h"
#include "defs.h"
#include "stddef.h"
#define HEAP_SIZE (16*1024*1024)
#define MAX_HANDLES 1024
//static char heap[HEAP_SIZE];
static void *heap_start=(void*)HEAPSTART;
static void *heap_end=(void*)HEAPSTART+HEAP_SIZE;
//占48字节，64位系统8字节对齐
struct block {
  uint size; // 内存块的大小()                8B（原4B，对齐后8B）
  struct block *next; // 指向下一个内存块的指针 8B
  int free; // 是否空闲                       8B(原4B，对齐后8B)
  struct spinlock lock;                  //24B
};
struct handle_entry
{
    int in_use;
    struct block *block;
};
static struct handle_entry handles[MAX_HANDLES];
static struct spinlock handle_lock;
static int next_handle=1;
static struct block *free_list; // 初始化空闲链表
void init_block(struct block *b, uint size, int free) {
    b->size = size;
    b->free = free;
    initlock(&b->lock, "block");
    //memset(b,1,size);
}
void heap_init()
{
    free_list=(struct block*)heap_start;
    init_block(free_list, HEAP_SIZE - sizeof(struct block), 1);                                          
    free_list->next = NULL; // 空闲链表只有一个块
    //句柄数据结构初始化：
    initlock(&handle_lock, "handle_lock");
    for (int i = 0; i < MAX_HANDLES; i++) {
        handles[i].in_use = 0;
        handles[i].block = NULL;
    }
    //memset(free_list,1,HEAP_SIZE - sizeof(struct block));
}
//为块分配句柄
int alloc_handle(struct block *b) {
    acquire(&handle_lock);
    for (int i = 0; i < MAX_HANDLES; i++) {
        int h = (next_handle + i) % MAX_HANDLES;
        if (!handles[h].in_use) {
            handles[h].in_use = 1;
            handles[h].block = b;
            next_handle = (h + 1) % MAX_HANDLES;
            release(&handle_lock);
            return h;
        }
    }
    release(&handle_lock);
    return -1;
}
//通过句柄来找到对应的块指针（数据起始位置）
void* get_block_by_handle(int handle) {
    if (handle < 0 || handle >= MAX_HANDLES || !handles[handle].in_use) {
        return NULL;
    }
    return (void*)((char*)handles[handle].block + sizeof(struct block));
}
void* heap_alloc(uint size)
{
    if (size == 0 || size > HEAP_SIZE - sizeof(struct block)) 
    {
        printf("请求的大小无效：%d\n", size);
        return NULL;
    }
    struct block *current=free_list;
    struct block *prev = NULL;
    while(current!=NULL)
    {
        acquire(&current->lock); // 锁定当前块
        if(current->free && current->size >= size)
        {
            if(current->size > size + sizeof(struct block))
            { // 如果块足够大，则分裂
                struct block *new_block = (struct block*)((char*)current + sizeof(struct block) + size);
                if ((void*)new_block < heap_start || (void*)new_block >= heap_end) 
                {
                    printf("新块超出堆范围\n");
                    release(&current->lock);
                    return NULL;
                }
                init_block(new_block, current->size - size - sizeof(struct block), 1);
                new_block->next = current->next; // 将新块插入链表
                current->size = size; // 更新当前块的大小
                current->next = new_block; // 更新链表指针
            }
            current->free = 0; // 标记当前块为已使用
            release(&current->lock);
            if (prev) 
            {
                acquire(&prev->lock); // 锁定前一个块
                prev->next = current->next; // 更新前一个块的指针
                release(&prev->lock); // 解锁前一个块
            } 
            else 
            {
                free_list = current->next; // 更新空闲链表头指针
            }
            //分配句柄
            int handle = alloc_handle(current);
            if (handle < 0) {
                current->free = 1;
                return NULL;
            }

            acquire(&current->lock); // 锁定当前块
            void *data_ptr = (void*)((char*)current + sizeof(struct block));
            memset(data_ptr, 5, size); // 分配时填充为数字 5
            release(&current->lock);
            printf("已在%p处分配了%dB大小的空间\n",data_ptr,size);
            return (void*)(uint64)handle;
            //return data_ptr; // 返回分配的内存地址
        }
        release(&current->lock); // 解锁当前块
        prev = current;
        current = current->next;
    }
    printf("当前内存中空闲块如下：\n");
    print_free_list();
    printf("没有大小适合的内存块，需要内存紧凑\n");
    printf("内存紧凑中...\n");
    compact();
    printf("内存紧凑完成，尝试重新分配中...\n");
    current=free_list;
    prev=NULL;
    while(current!=NULL)
    {
        acquire(&current->lock); // 锁定当前块
        if(current->free && current->size >= size)
        {
            if(current->size > size + sizeof(struct block))
            { // 如果块足够大，则分裂
                struct block *new_block = (struct block*)((char*)current + sizeof(struct block) + size);
                if ((void*)new_block < heap_start || (void*)new_block >= heap_end) 
                {
                    printf("新块超出堆范围\n");
                    release(&current->lock);
                    return NULL;
                }
                init_block(new_block, current->size - size - sizeof(struct block), 1);
                new_block->next = current->next; // 将新块插入链表
                current->size = size; // 更新当前块的大小
                current->next = new_block; // 更新链表指针
            }
            current->free = 0; // 标记当前块为已使用
            release(&current->lock);
            if (prev) 
            {
                acquire(&prev->lock); // 锁定前一个块
                prev->next = current->next; // 更新前一个块的指针
                release(&prev->lock); // 解锁前一个块
            } 
            else 
            {
                free_list = current->next; // 更新空闲链表头指针
            }
            //分配句柄
            int handle = alloc_handle(current);
            if (handle < 0) {
                current->free = 1;
                return NULL;
            }


            acquire(&current->lock); // 锁定当前块
            void *data_ptr = (void*)((char*)current + sizeof(struct block));
            memset(data_ptr, 5, size); // 分配时填充为数字 5
            release(&current->lock);
            return (void*)(uint64)handle;
            //return data_ptr; // 返回分配的内存地址
        }
        release(&current->lock); // 解锁当前块
        prev = current;
        current = current->next;
    }
    printf("超出最大可分配容量！不分配\n");
    return NULL;
}
void heap_free(void *ptr)
{
    if (ptr == NULL) return;
    int handle = (int)(uint64)ptr;
    if (handle < 0 || handle >= MAX_HANDLES || !handles[handle].in_use) {
        printf("无效的句柄：%d\n", handle);
        return;
    }

    struct block *block_ptr = handles[handle].block;
    if ((void*)block_ptr < heap_start || (void*)block_ptr >= heap_end) {
        printf("块头指针超出堆范围：%p\n", block_ptr);
        return;
    }

    acquire(&block_ptr->lock);
    block_ptr->free = 1;
    printf("已在%p处释放了%dB大小的空间\n",(void*)((char*)block_ptr + sizeof(struct block)),block_ptr->size);
    memset((void*)((char*)block_ptr + sizeof(struct block)), 1, block_ptr->size);
    release(&block_ptr->lock);

    acquire(&handle_lock);
    handles[handle].in_use = 0;
    handles[handle].block = NULL;
    release(&handle_lock);

    // struct block *current = free_list;
    // struct block *prev = NULL;

    // while (current != NULL && (struct block*)((char*)current + current->size + sizeof(struct block) - 1) < block_ptr) {
    //     prev = current;
    //     current = current->next;
    // }

    // block_ptr->next = current;
    // if (prev) {
    //     prev->next = block_ptr;
    // } else {
    //     free_list = block_ptr;
    // }

    //合并操作
    // 遍历整个堆，查找相邻的块
    struct block *current = (struct block*)heap_start;
    struct block *prev = NULL;

    while ((void*)current < heap_end) {
        if (current == block_ptr) {
            break;
        }
        prev = current;
        current = (struct block*)((char*)current + current->size + sizeof(struct block));
    }

    struct block *next = (struct block*)((char*)block_ptr + block_ptr->size + sizeof(struct block));

    // 如果前后都有空闲块
    if (prev && prev->free && next && next->free && (void*)next < heap_end) {
        acquire(&prev->lock);
        acquire(&block_ptr->lock);
        acquire(&next->lock);
        prev->size += block_ptr->size + next->size + 2 * sizeof(struct block);
        prev->next = next->next;
        release(&next->lock);
        release(&block_ptr->lock);
        release(&prev->lock);
    }
    // 如果前面有空闲块
    else if (prev && prev->free) {
        acquire(&prev->lock);
        acquire(&block_ptr->lock);
        prev->size += block_ptr->size + sizeof(struct block);
        //prev->next = block_ptr->next;
        release(&block_ptr->lock);
        release(&prev->lock);
    }
    // 如果后面有空闲块
    else if ((void*)next < heap_end && next->free) {
        acquire(&block_ptr->lock);
        acquire(&next->lock);
        block_ptr->size += next->size + sizeof(struct block);
        block_ptr->next = next->next;
        if(next==free_list)free_list=block_ptr;
        release(&next->lock);
        release(&block_ptr->lock);
    }
    // 否则直接插入空闲链表
    else {
    current = free_list;
    prev = NULL;

    while (current != NULL && (struct block*)((char*)current + current->size + sizeof(struct block) - 1) < block_ptr) {
        prev = current;
        current = current->next;
    }

    block_ptr->next = current;
    if (prev) {
        prev->next = block_ptr;
    } else {
        free_list = block_ptr;
    }
    }

}
void heap_free_ptr(void *ptr) 
{
  if(ptr == NULL) return; // 如果指针为空，直接返回
  if ((void*)ptr < heap_start || (void*)ptr >= heap_end) 
  {
        printf("释放的地址超出堆范围：%p\n", ptr);
        return;
  }
  struct block *block_ptr = (struct block*)((char*)ptr - sizeof(struct block)); // 获取块头指针
  if ((void*)block_ptr < heap_start || (void*)block_ptr >= heap_end) 
  {
        printf("块头指针超出堆范围：%p\n", block_ptr);
        return;
  }
  acquire(&block_ptr->lock);
  block_ptr->free = 1; // 标记为空闲
  printf("已在%p处释放了%dB大小的空间\n",(struct block *)((char *)ptr),block_ptr->size);
  memset(ptr, 1, block_ptr->size); // 释放时填充为数字 1
  release(&block_ptr->lock);
  struct block *current = free_list;
  struct block *prev = NULL;

  // 将释放的块插入空闲链表
  while (current != NULL && (struct block*)((char*)current+current->size+sizeof(struct block)-1) < block_ptr) 
  {
    prev = current;
    current = current->next;
  }

  block_ptr->next = current;
  if (prev) 
  {
    prev->next = block_ptr;
  } 
  else 
  {
    free_list = block_ptr;
  }
}
void compact()
{
    struct block *current = free_list;
     struct block *next_free = free_list->next;
    void *new_alloc_start = heap_start;
    uint total_free_size = 0;
    struct block *prev_allocated = NULL;
    // 1. 统计所有空闲内存块的总大小
    while (current!=NULL) 
    {
        //printf("%p\n",current);
        acquire(&current->lock);
        if (current->free) 
        {
            total_free_size += current->size + sizeof(struct block);
        }
        release(&current->lock);
        current = current->next;
    }
    printf("空闲内存块的总大小为：%dB\n",total_free_size);
    // 2. 移动所有已分配的内存块到低地址
    current = free_list;
    //printf("%p\n",current);
    //struct block *prev_allocated = NULL;
    while (current != NULL) 
    {
        //printf("Current block: %p\n", current);
        //printf("Next free block: %p\n", next_free);
        // 找到两个空闲块之间的已分配块
        acquire(&current->lock);
        struct block *allocated_block = (struct block *)((char *)current + sizeof(struct block) + current->size);
        release(&current->lock);
        while ((void *)allocated_block < (void *)next_free) 
        {
            //printf("Allocated block: %p\n", allocated_block);
            //printf("allocated_size:%d\n",allocated_block->size);
            // 检查块是否已分配
            acquire(&allocated_block->lock);
            if (!allocated_block->free) 
            {
                if ((void *)allocated_block != new_alloc_start) 
                {
                    // 需要移动已分配块，先更新 current 和 allocated_block,
                    // 因内层循环的判断条件需要使用next_free故此处不能更新，需要等待需要移动的块都移动完成后才能更新
                    // 即退出内层循环后才更新
                    //printf("Allocated block size before move: %d\n", allocated_block->size);
                    current = next_free;
                    if (next_free != NULL) 
                    {
                        //next_free = next_free->next;
                    }
                    // 在移动前，备份 allocated_block 的大小
                    uint allocated_size_backup = allocated_block->size;
                    void *allocated_block_backup = (void *)allocated_block;
                    release(&allocated_block->lock); // 解锁旧的 allocated_block
                    allocated_block = (struct block *)((char *)allocated_block + sizeof(struct block) + allocated_block->size);
                    //printf("Moving block from %p to %p\n", allocated_block, new_alloc_start);
                    //printf("old_current: %p, old_current size: %d\n", old_current, old_current->size);
                    //printf("current: %p, next_free: %p\n", current, next_free);
                    //printf("new_alloc_start: %p\n", new_alloc_start);
                    memmove(new_alloc_start, allocated_block_backup, sizeof(struct block) + allocated_size_backup);
                    struct block *moved_block = (struct block *)new_alloc_start;
                    // 检查移动后的块大小是否正确
                    if (moved_block->size != allocated_size_backup) 
                    {
                        printf("Error: Block size mismatch after move! Expected: %d, Got: %d\n", allocated_size_backup, moved_block->size);
                        return;
                    }
                    acquire(&moved_block->lock); // 锁定移动后的块
                    if (prev_allocated) 
                    {
                        prev_allocated->next = moved_block;
                    }
                    prev_allocated = moved_block;
                    release(&moved_block->lock); // 解锁移动后的块
                    new_alloc_start = (void *)((char *)new_alloc_start + sizeof(struct block) + moved_block->size);
                    //printf("Moved block: %p, Size after move: %d\n", moved_block, moved_block->size);
                    
                    // 更新句柄的块指针
                    acquire(&handle_lock);
                    for (int i = 0; i < MAX_HANDLES; i++) {
                        if (handles[i].in_use && handles[i].block == (struct block*)allocated_block_backup) {
                            handles[i].block = moved_block;
                        }
                    }
                    release(&handle_lock);
                } 
                else 
                {
                    if (prev_allocated) 
                    {
                        prev_allocated->next = allocated_block;
                    }
                    prev_allocated = allocated_block;
                    new_alloc_start = (void *)((char *)new_alloc_start + sizeof(struct block) + allocated_block->size);
                    release(&allocated_block->lock); // 解锁 allocated_block
                }
            }
            else
            {
            // 更新 allocated_block
            //release(&current->lock);
            release(&allocated_block->lock); // 解锁未分配块
            allocated_block = (struct block *)((char *)allocated_block + sizeof(struct block) + allocated_block->size);
            }
        }
        // 如果不需要移动已分配的块，那么更新 current 和 next_free
        next_free=next_free->next;
        if ((void *)allocated_block >= (void *)next_free) 
        {
            current = next_free;
            if (next_free != NULL)
            {
                next_free = next_free->next;
            }
        }
    }

    // 3. 重建空闲列表
    free_list = (struct block *)new_alloc_start;
    init_block(free_list, total_free_size - sizeof(struct block), 1);
    free_list->next = NULL;
    if (prev_allocated) {
        acquire(&prev_allocated->lock);
        prev_allocated->next = free_list;
        release(&prev_allocated->lock);
    }
    printf("内存紧凑已完成，空闲块和分配块如下：\n");
    print_alloc_list();
    print_free_list();
}
void print_free_list() {
    if(free_list==NULL)
    {
        printf("空闲链表:\n");
        printf("    空闲链表为空!");
    }
    else
    {
        acquire(&free_list->lock);
        struct block *current = free_list;
        printf("空闲链表:\n");
        int i=0;
        while (current != NULL) 
        {
           uint bsize=current->size;
           if(bsize>=1024*1024)
           {
               printf("  空闲块 %d: 起始地址:%p, 尾地址:%p, 大小=%d.%dMB, ", i, current,(struct block*)((char*)current+current->size+sizeof(struct block)-1), (current->size/1048576),(current->size%1048576));
           }
           else
           {
               printf("  空闲块 %d: 起始地址:%p, 尾地址:%p, 大小=%dB, ", i, current,(struct block*)((char*)current+current->size+sizeof(struct block)-1), current->size);
           }
           if(current->next==NULL)
           {
               printf(" 已经是最后一个空闲块");
           }
           else
           {
              printf("  下一空闲块地址%p\n",current->next);
           }
           current = current->next;
           i++;
        }
        release(&free_list->lock);
    }
    printf("\n");
}
void print_alloc_list() {
    struct block *current = (struct block *)heap_start;
    int i = 0;
    int flag=0;
    printf("已分配内存块列表:\n");
    while (current < (struct block *)(PHYSTOP) ){
        acquire(&current->lock);
        if (!current->free) {
            flag=1;
            void *start_address = (void *)((char *)current);
            void *end_address = (void *)((char *)start_address + sizeof(struct block)+ current->size - 1);
            uint bsize=current->size;
            if(bsize>=1024*1024)
            {
                printf("  已分配块 %d: 起始地址: %p, 大小: %d.%dMB, 尾地址: %p\n", i, start_address, (current->size/1048576),(current->size%1048576), end_address);
            }
            else
            {
                printf("  已分配块 %d: 起始地址: %p, 大小: %dB, 尾地址: %p\n", i, start_address, current->size, end_address);
            }
            i++;
        }
        release(&current->lock);
        current = (struct block *)((char *)current + sizeof(struct block) + current->size);
    }
    if(flag==0)
    {
        printf("    已分配表为空！");
        printf("\n");
    }
    
}
void byte_to_hex(unsigned char byte, char *hex_buffer) {
    const char hex_chars[] = "0123456789abcdef";
    hex_buffer[0] = hex_chars[(byte >> 4) & 0x0F]; // 高四位
    hex_buffer[1] = hex_chars[byte & 0x0F];        // 低四位
}
void print_block_content(struct block *block) {
    if (block == NULL || block->free) {
        printf("Error: Invalid or free block\n");
        return;
    }

    void *start_address = (void *)((char *)block + sizeof(struct block));
    uint size = block->size;
    unsigned char *data = (unsigned char *)start_address;

    printf("内存块内容 (起始地址: %p, 大小: %dB):\n", start_address, size);
    char hex_buffer[3]; // 两个字符表示一个字节的十六进制值，外加一个空字符
    for (uint i = 0; i < size; i++) {
        byte_to_hex(data[i], hex_buffer);
        hex_buffer[2] = '\0'; // 空字符
        printf("%s ", hex_buffer);
        if ((i + 1) % 16 == 0) {
            printf("\n");
        }
    }
    if (size % 16 != 0) {
        printf("\n");
    }
}
void writeToHeap(void *block, int sit, int data) {
    struct block *p = handles[(uint64)block].block; // 通过句柄获取块头指针
    if (p == NULL || p->free) {
        printf("无效或空闲的块指针\n");
        return;
    }

    void *data_ptr = (void*)((char*)p + sizeof(struct block)); // 数据部分的起始地址
    if (sit < 0 || sit >= p->size) {
        printf("写入位置无效：%d\n", sit);
        return;
    }

    int *target_ptr = (int*)((char*)data_ptr + sit); // 计算目标位置
    *target_ptr = data; // 写入数据
    printf("已在%p处写入数据%d到位置%d\n", target_ptr, data, sit);
    print_block_content(p);
}
void test_heap_alloc(void) 
{
//   printf("句柄数组的起始位置为：%p\n",(void *)handles);
//   //printf("%d\n",sizeof(struct block*));
//   //printf("块头大小为%dB\n",sizeof(struct block));
//   //1.最大可分配空间测试
//   printf("最大可分配空间测试:\n");
//   heap_init();
//   void*a=heap_alloc(16777168);
//   printf("在%p处分配了16777168B空间\n",get_block_by_handle((uint64)a));
//   print_free_list();
//   printf("\n");

//   //2.超出最大空间测试
//   printf("超出最大空间测试:\n");
//   heap_init();
//   void*b=heap_alloc(16777170);
//   if(get_block_by_handle((uint64)b)!=NULL)printf("在%p处分配了16777170B空间\n",get_block_by_handle((uint64)b));
//   print_free_list();
//   printf("\n");

//   //3.常规测试
//   printf("常规测试:\n");
//   heap_init();
//   void* c=heap_alloc(1000);
//   printf("在%p处分配了1000B空间\n",get_block_by_handle((uint64)c));
//   //print_block_content((struct block *)((char *)get_block_by_handle((uint64)c)-sizeof(struct block)));
//   print_free_list();

//   void* d=heap_alloc(3000);
//   printf("在%p处分配了3000B空间\n",get_block_by_handle((uint64)d));
//   print_free_list();

//   void* e=heap_alloc(2000);
//   printf("在%p处分配了2000B空间\n",get_block_by_handle((uint64)e));
//   print_free_list();

//   heap_free(c);
//   printf("在%p处释放了1000B空间\n",get_block_by_handle((uint64)c));
//   print_free_list();

//   heap_free(d);
//   printf("在%p处释放了3000B空间\n",get_block_by_handle((uint64)d));
//   print_free_list();

//   void* f=heap_alloc(4000);
//   printf("在%p处分配了4000B空间\n",get_block_by_handle((uint64)f));
//   print_free_list();
//   printf("\n");

//   //4.紧凑测试
//   printf("紧凑测试:\n");
//   heap_init();
//   void* m=heap_alloc(2*1024*1024);
//   printf("在%p处分配了2MB空间\n",get_block_by_handle((uint64)m));
//   void* n=heap_alloc(3*1024*1024);
//   printf("在%p处分配了3MB空间\n",get_block_by_handle((uint64)n));
//   void* p=heap_alloc(4*1024*1024);
//   printf("在%p处分配了4MB空间\n",get_block_by_handle((uint64)p));
//   void* q=heap_alloc(5*1024*1024);
//   printf("在%p处分配了5MB空间\n",get_block_by_handle((uint64)q));
//   print_alloc_list();
//   heap_free(m);
//   printf("在%p处释放了2MB空间\n",get_block_by_handle((uint64)m));
//   heap_free(p);
//   printf("在%p处释放了4MB空间\n",get_block_by_handle((uint64)p));
//   print_free_list();
//   print_alloc_list();

//   void* r=heap_alloc(9*1024*1024);
//   if(get_block_by_handle((uint64)r)!=NULL)printf("在%p处分配了9MB空间\n",get_block_by_handle((uint64)r));
//   print_free_list();

//   void* s=heap_alloc(7*1024*1024);
//   if(get_block_by_handle((uint64)s)!=NULL)printf("在%p处分配了7MB空间\n",get_block_by_handle((uint64)s));
//   print_free_list();

  //测试6：释放时相邻空闲块合并
    //依次分配 1000，2000，3000，4000
    //1.释放1000后2000
    //2.释放2000后1000
    //3.释放1000，3000后2000
    printf("\n");
    printf("test6:\n");
    heap_init();
    void *s1=heap_alloc(1000);
    void *s2=heap_alloc(2000);
    void *s3=heap_alloc(3000);
    heap_alloc(4000);
    heap_free(s1);
    heap_free(s2);
    print_free_list();
    print_alloc_list();
    void *s12=heap_alloc(1000);
    void *s22=heap_alloc(2000);
    heap_free(s22);
    heap_free(s12);
    print_free_list();
    print_alloc_list();
    void *s13=heap_alloc(1000);
    void *s23=heap_alloc(2000);
    heap_free(s13);
    heap_free(s3);
    heap_free(s23);

    print_free_list();
    print_alloc_list();
}

