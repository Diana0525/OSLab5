#include "disk.h"
#include "file.h"
#include <stdio.h>
//https://blog.csdn.net/liiiii_22/article/details/91398203?utm_medium=distribute.pc_relevant.none-task-blog-baidulandingword-11&spm=1001.2101.3001.4242
#define Buffer_Length 1024   // 缓冲区大小,是物理磁盘块大小的两倍
#define IsFile 1            // 是文件
#define IsDir 2             // 是目录
#define SUCCESS 1           // 返回成功
#define ERROR 0             // 返回失败
char buf[Buffer_Length];
/**
 * 函数名：Init
 * 函数功能：初始化文件系统
 */
void Init(){
    int i, j;
    /* 初始化磁盘 */
    int open;
    open = open_disk();
    if(open == -1){
        printf("disk is already open!\n");
    }
    else if(open == 0){
        printf("open disk successfully!\n");
    }

    /* 初始化目录项 */
    // 根目录有固定的起点，Ext2文件系统的根目录索引号为2 

    /* 初始化超级块 */
    if(disk_read_block(1, buf) == -1 || disk_read_block(2, buf+512) == -1){
        printf("fail to read block %d !\n",)
    } 
    sp_block = (struct spuer_block*) buf;
    sp_block.magic_num = 0xdec0de;  // 给幻数赋值
    // 4M空间，1个数据块1K，共有4096个数据块
    sp_block.free_block_count = 4096;
    sp_block.free_inode_count = 1024;
    // dir_inode_count 指的是被占用的目录inode数
    sp_block.dir_inode_count = 0；
    // 初始化数据块占用位图为0，注意是按bit记录的，共记录了128*32=4096个数据
    for(int i = 0; i < 128; i++){
        block_map[i] = 0;
    }
    for(int i = 0; i < 32; i++){
        inode_map[i] = 0;
    }
    buf = (char*) sp_block;
}

/**
 * 函数名：create
 * 函数功能：根据指定的文件名创建新文件
 */
int create(char *filename){

}
