#include "disk.h"
#include "main.h"
#include <stdio.h>
#include <stdlib.h>
//https://blog.csdn.net/liiiii_22/article/details/91398203?utm_medium=distribute.pc_relevant.none-task-blog-baidulandingword-11&spm=1001.2101.3001.4242
#define Buffer_Length 1024   // 缓冲区大小,是物理磁盘块大小的两倍
#define notFileorDir 0
#define IsFile 1            // 是文件
#define IsDir 2             // 是目录
#define SUCCESS 1           // 返回成功
#define ERROR 0             // 返回失败
#define INODE_NUM 32        // 给inode分配的数据块个数
#define DATA_BLOCK_NUM 4096-1-32 // 分配给数据存储的数据块
#define BLOCK_POINT_NUM 6   // 一个inode里有6个数据块指针

/**
 * 函数名：read_data_block
 * 函数功能：从磁盘块中读取第block_num块数据块，从0开始编号
 */
int read_data_block(unsigned int block_num, char* buf){
    if(disk_read_block(block_num*2, buf) == -1 || disk_read_block(block_num*2+1, buf+512) == -1){
        printf("fail to read block!\n");
        return ERROR;
    } 
    return SUCCESS;
}
/**
 * 函数名：write_data_block
 * 函数功能：向磁盘块中写入第block_num块数据块，从0开始编号
 */
int write_data_block(unsigned int block_num, char* buf){
    if(disk_write_block(block_num*2, buf) == -1 || disk_write_block(block_num*2+1, buf+512) == -1){
        printf("fail to write block!\n");
        return ERROR;
    } 
    return SUCCESS;
}
/**
 * 函数名：Init
 * 函数功能：初始化文件系统
 */
void Init(){
    /* 初始化磁盘 */
    int open;
    char *buf_pointer;
    char buf[Buffer_Length];
    buf_pointer = buf;
    sp_block* sp_block;
    inode* inode;
    open = open_disk();
    if(open == -1){
        printf("disk is already open!\n");
    }
    else if(open == 0){
        printf("open disk successfully!\n");
    }
    printf("1:%d\n",sizeof(sp_block));
    printf("1:%d\n",sizeof(buf_pointer));

    /* 初始化超级块 */
    if(read_data_block(0, buf) == ERROR){
        exit(-1);
    } 
    printf("hello\n");
    sp_block = (struct sp_block*) buf_pointer;
    sp_block->magic_num = 0xdec0de;  // 给幻数赋值
    // 4M空间，1个数据块1K，共有4096个数据块
    sp_block->free_block_count = 4096;
    sp_block->free_inode_count = 1024;
    // dir_inode_count 指的是被占用的目录inode数
    sp_block->dir_inode_count = 0;
    // 初始化数据块占用位图为0，注意是按bit记录的，共记录了128*32=4096个数据
    for(int i = 0; i < 128; i++){
        sp_block->block_map[i] = 0;
    }
    for(int i = 0; i < 32; i++){
        sp_block->inode_map[i] = 0;
    }
    write_data_block(0, buf);//将初始化的超级块写入磁盘块
    
    /* 初始化根目录存储在block2中 */
    // 根目录有固定的起点，Ext2文件系统的根目录索引号为2 
    
    /* 初始化inode，给inode数组分配数据块 */
    for(int i = 0; i < INODE_NUM; i++){//给inode数组分配32个数据块
        if(read_data_block(i+1, buf) == ERROR){
            exit(-1);
        }
        buf_pointer = buf;
        for (int j = 0; j < INODE_NUM; j++)//一个数据块里能放32个inode
        {
            inode = (struct inode*) buf_pointer;
            
            inode->size = 0;
            inode->file_type =  notFileorDir;
            inode->link = 0;
            for (int k = 0; k < BLOCK_POINT_NUM; k++)
            {
                inode->block_point[k] = 0;
            }
            buf_pointer += 32;//指向下一个inode
        }
        if(write_data_block(i+1, buf) == ERROR){
            exit(-1);
        }
    }
    
}

/**
 * 函数名：create
 * 函数功能：根据指定的文件名创建新文件
 */
/* int create(char *filename){

} */

int main(){
    Init();
    printf("hello4\n");
    return 0;
}