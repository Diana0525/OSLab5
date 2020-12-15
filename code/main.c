#include "disk.h"
#include "main.h"
#include <stdio.h>
#include <stdlib.h>

#define Buffer_Length 1024   // 缓冲区大小,是物理磁盘块大小的两倍
#define notFileorDir 0
#define IsFile 1            // 是文件
#define IsDir 2             // 是目录
#define SUCCESS 1           // 返回成功
#define ERROR -1             // 返回失败
#define INODE_NUM 32        // 给inode分配的数据块个数
#define DATA_BLOCK_NUM 4096-1-32 // 分配给数据存储的数据块
#define BLOCK_POINT_NUM 6   // 一个inode里有6个数据块指针
#define DIR_ITEM_NUM 8      // 一个数据块里有8个dir_item
#define NUMBLOCK(x) x+33    //将第0-2014块号的数据块映射到实际的数据块编号上
#define Occupy 1            // 表示有一块数据块或inode块被占据
#define notOccupy 0         // 表示有一块数据块或inode块被释放
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
    char answer;
    buf_pointer = buf;
    sp_block* sp_block;
    inode* inode;
    dir_item* dir_item;
    printf("nn's file system found\n");
    while(1){
        printf("Format disk and rebuild a sucking file system?(y/n)");
        scanf("%c",&answer);
        getchar();
        if(answer == 'n'){
            return 0;
        }
        else if(answer == 'y'){
            open = open_disk();
            break;
        }
        else{
            continue;
        }
    }
    if(open == -1){
        printf("disk is already open!\n");
    }
    else if(open == 0){
        printf("open disk successfully!\n");
    }

    /* 初始化超级块 */
    if(read_data_block(0, buf) == ERROR){
        exit(-1);
    } 
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
        if(i == 0){
            sp_block->block_map[i] = 1;// 第一个数据块被用于存放根目录的目录项,从右到左，从1到32
        }
    }
    // 32*32bit 来表示1024个inode块
    for(int i = 0; i < 32; i++){
        sp_block->inode_map[i] = 0;
        if(i == 0){
            sp_block->inode_map[i] = 1;// 第一个inode块用于存放指向根目录以及其他目录的数据块指针,从右到左，从1到32
        }
    }
    if((write_data_block(0, buf)) == ERROR){//将初始化的超级块写入磁盘块
        printf("write data error!\n");
        exit(-1);
    }
    
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
            if(j == 0){
              inode->file_type = IsDir;// 将第一个inode设置为指向目录项  
              inode->block_point[0] = NUMBLOCK(0);// 让第一个block指针指向第一块数据块，此数据块存有根目录项
            } 
            buf_pointer += 32;//指向下一个inode
        }
        if(write_data_block(i+1, buf) == ERROR){
            exit(-1);
        }
    }

    /* 初始化根目录项 */
    if((read_data_block(NUMBLOCK(0), buf)) == ERROR){
        printf("write data error!\n");
        exit(-1);
    }
    buf_pointer = buf;
    for(int i = 0; i < DIR_ITEM_NUM; i++){
        dir_item = (struct dir_item*) buf_pointer;
        dir_item->inode_id = 0;// 无效的inode_id定为0，因此1024个inode的编号从1~1024
        dir_item->valid = 0;
        dir_item->type = notFileorDir;
        buf_pointer += 128;     //指向下一个dir_item
    }
    if(write_data_block(NUMBLOCK(0), buf) == ERROR){
        printf("write data error!\n");
        exit(-1);
    }
}
/**
 * 函数名：TestInit
 * 函数功能:测试Init是否成功初始化磁盘块中的超级块和inode块
 */
void TestInit(){
    char *buf_pointer;
    char buf[Buffer_Length];
    buf_pointer = buf;
    sp_block* sp_block;
    inode* inode;
    // 验证超级块是否初始化成功
    if(read_data_block(0, buf) == ERROR){
        exit(-1);
    } 
    sp_block = (struct sp_block*) buf_pointer;
    printf("sp_block->magic_num=%x\n",sp_block->magic_num);
    printf("sp_block->free_block_count=%d\n",sp_block->free_block_count);
    printf("sp_block->free_inode_count=%d\n",sp_block->free_inode_count);
    printf("sp_block->dir_inode_count=%d\n",sp_block->dir_inode_count);
    // 初始化数据块占用位图为0，注意是按bit记录的，共记录了128*32=4096个数据
    for(int i = 0; i < 128; i++){
        printf("sp_block->block_map[%d]=%d\n",i,sp_block->block_map[i]);
    }
    for(int i = 0; i < 32; i++){
        printf("sp_block->inode_map[%d]=%d\n",i,sp_block->inode_map[i]);
    }

    /* // 验证inode块是否初始化成功
    for(int i = 0; i < INODE_NUM; i++){//给inode数组分配32个数据块
        if(read_data_block(i+1, buf) == ERROR){
            exit(-1);
        }
        buf_pointer = buf;
        for (int j = 0; j < INODE_NUM; j++)//一个数据块里能放32个inode
        {
            inode = (struct inode*) buf_pointer;
            printf("%d: ",j);
            printf(" inode->size=%d",inode->size);
            printf(" inode->file_type=%d",inode->file_type);
            printf(" inode->link=%d",inode->link);
            for (int k = 0; k < BLOCK_POINT_NUM; k++)
            {
                printf("inode->block_point[%d]=%d\n",k,inode->block_point[k]);
            }
            buf_pointer += 32;//指向下一个inode
        }
    } */
}
/* 
* 函数名：findInodeID
* 函数功能：寻找没有被使用的inode_id,从1~1024
*/
uin32_t findInodeID(){
    char *buf_pointer;
    char buf[Buffer_Length];
    uin32_t InodeID;
    buf_pointer = buf;
    sp_block* sp_block;
    uin32_t temp=1;
    // 读取超级块
    if(read_data_block(0, buf) == ERROR){
        exit(-1);
    }
    sp_block = (struct sp_block*) buf_pointer;
    for(int i = 0; i < 32; i++){
        if(sp_block->inode_map[i] != 0xffffffff){// 表示当前有空闲位置
            for(InodeID = 1; InodeID <= 32; InodeID++){
                if((sp_block->inode_map[i] & temp) == 0){// 表示当前遍历到了空闲位置
                    return InodeID;
                }
                temp = temp<<1;
            }
        }
    }
    //若没找到空闲位置，返回0，表示没找到合适的inode
    return 0;
}
/* 
* 函数名：findBlockID
* 函数功能：寻找没有被使用的inode_id,从1~1024
*/
void findBlockID(){
    char *buf_pointer;
    char buf[Buffer_Length];
    uin32_t BlockID;
    buf_pointer = buf;
    sp_block* sp_block;
    uin32_t temp=1;// 用来做与运算
    // 读取超级块
    if(read_data_block(0, buf) == ERROR){
        exit(-1);
    }
    sp_block = (struct sp_block*) buf_pointer;
    for(int i = 0;i < 128; i++){
        if(sp_block->block_map[i] != 0xffffffff){
            for(BlockID = 1; BlockID <= 32; BlockID++){
                if((sp_block->block_map[i] & temp) == 0){
                    return BlockID;
                }
                temp = temp << 1;
            }
        }
    }
    //若没找到空闲位置，返回0，表示没找到合适的block
    return 0;
}
/* 
* 函数名：refreshSuperBlock
* 函数功能：更新超级块中的一些信息
*/
void refreshSuperBlock(uin32_t inode_id, int type){
    char *buf_pointer;
    char buf[Buffer_Length];
    sp_block* sp_block;
    buf_pointer = buf;
    int block,num;
    uin32_t temp = 1;// 用于&运算
    // 读取超级块
    if(read_data_block(0, buf) == ERROR){
        exit(-1);
    }
    sp_block = (struct sp_block*) buf_pointer;
    block = (inode_id-1)/32;// 1~32映射为0，33~64映射为1，65~128映射为2
    num = (inode_id-1)%32;// 例如32映射为第0块的第20块，需要左移19位后&1运算
    temp = temp << num;
    if(type == Occupy){
        sp_block->inode_map[block] &= temp;
    }else if(type == notOccupy){
        sp_block->inode_map[block] &= ~temp;// 取反后，进行与运算，即可让相应位置0
    }
    if(write_data_block(0, buf) == ERROR){
        exit(-1);
    }
}
/* 
* 函数名：refreshInode
* 函数功能：根据inode_id找到指定inode块，并更新相应的信息
*/
void refreshInode(uin32_t inode_id, int file_type){
    char *buf_pointer;
    char buf[Buffer_Length];
    inode* inode;
    buf_pointer = buf;
    int block,num;
    block = (inode_id-1)/32;// 1~32映射为0，33~64映射为1，65~128映射为2
    num = (inode_id-1)%32;
    if((read_data_block(block+1, buf)) == ERROR){
        exit(-1);
    }
    buf_pointer += num*32;
    inode = (struct inode*)buf_pointer;
    inode->file_type = file_type;
    if((write_data_block(block+1), buf) == ERROR){
        exit(-1);
    }
}
/**
 * 函数名：create
 * 函数功能：根据指定的文件名创建新目录
 * 入口参数：文件名，inode指针，filetype表示文件或目录
 */
int create(char *filename, struct inode* inode, int filetype){
    char buf[Buffer_Length];
    char *buf_pointer;
    buf_pointer = buf;
    dir_item* dir_item;
    uin32_t newInodeID;
    /* 确保输入的inode是dir类型 */
    if(inode->file_type == IsDir){
        // 在输入的inode块中找到一个空闲目录项数据块
        for(int i = 0; i < BLOCK_POINT_NUM; i++){
            if((inode->block_point[i] != 0   // 若找到一个已经被使用过的数据块
            && inode->size < (i+1)*1024)    // 且小于该值表示该目录项数据块还有空余的空间
            || inode->block_point[i] == 0){ // 或者是找到了一个未被使用过的数据块
                // 读取该目录项数据块
                if(inode->block_point[i] == 0){// 需要选出一块未被使用的数据块，被这个指针所指向
                    
                }else{
                    if((read_data_block(block_point[i], buf)) == ERROR){
                        printf("error!\n");
                        exit(-1);
                    }
                }
                dir_item = (struct dir_item*) buf_pointer;
                for(int j = 0; j < DIR_ITEM_NUM; j++){// 遍历此数据块中的8个目录项，找到未使用的一个目录项
                    if(dir_item->inode == 0){// 找到了未使用的
                        dir_item->inode = findInodeID();
                        newInodeID = dir_item->inode;
                        dir_item->valid = 1;
                        dir_item->type = filetype;
                        dir_item->name = filename;
                        break;
                    }
                }
                // 写回该目录项数据块
                if((write_data_block(block_point[i], buf)) == ERROR){
                    printf("write error!\n");
                    exit(-1);
                }
                refreshSuperBlock(newInodeID, Occupy);// 新增了一个inode块被使用，更新超级块中的信息
                refreshInode(newInodeID, filetype);
                break;
            }
        }
    }
    else if(inode->file_type == IsFile){// 到了创建文件的一步，是最终的步骤

    }
    return ERROR;
}

int main(){
    Init();
    /*TestInit();*/
    close_disk();
    return 0;
}