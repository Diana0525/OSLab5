#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "disk.h"
#include "main.h"


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
int Init(){
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
    /************* 询问是否开启磁盘系统 *************/
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
        printf("Formating finished,good luck and have fun!\n");
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
    sp_block->dir_inode_count = 1;// 根目录占据了一个目录inode
    // 初始化数据块占用位图为0，注意是按bit记录的，共记录了128*32=4096个数据
    for(int i = 0; i < 128; i++){
        sp_block->block_map[i] = 0;
        if(i == 0){
            sp_block->block_map[i] = 0xffffffff;// 表示前32个数据块都被占据，其中第一块是超级快，2~32块都是inode块
        }
        if(i == 1){
            sp_block->block_map[i] = 3;// 表示第1、2块即第33和34块被占据，33是inode块，34是存放根目录目录项的数据块
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
    for(int i = 0; i < INODE_NUM; i++){// 给inode数组分配32个数据块
        if(read_data_block(i+1, buf) == ERROR){
            exit(-1);
        }
        buf_pointer = buf;
        for (int j = 0; j < INODE_NUM; j++)// 一个数据块里能放32个inode
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
              inode->block_point[0] = NUMBLOCK(1);// 让第一个block指针指向第一块数据块，此数据块存有根目录项
            } 
            buf_pointer += 32;//指向下一个inode
        }
        if(write_data_block(i+1, buf) == ERROR){
            exit(-1);
        }
    }

    /* 初始化根目录项 */
    if((read_data_block(NUMBLOCK(0), buf)) == ERROR){
        printf("error!\n");
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
        printf("sp_block->block_map[%d]=%x\n",i,sp_block->block_map[i]);
    }
    for(int i = 0; i < 32; i++){
        printf("sp_block->inode_map[%d]=%x\n",i,sp_block->inode_map[i]);
    }

    // 验证inode块是否初始化成功

    if(read_data_block(1, buf) == ERROR){
        exit(-1);
    }
    buf_pointer = buf;
    inode = (struct inode*)buf_pointer;
    printf("inode->size=%d\n",inode->size);
    printf("inode->file_type=%d\n",inode->file_type);
    printf("inode->link=%d\n",inode->link);
    for(int i = 0; i < 6; i++){
        printf("inode->block_point[%d]=%d\n",i,inode->block_point[i]);
    }
}
/* 
* 函数名：findInodeID
* 函数功能：寻找没有被使用的inode_id,从1~1024
*/
uint32_t findInodeID(){
    char *buf_pointer;
    char buf[Buffer_Length];
    uint32_t InodeID;
    buf_pointer = buf;
    sp_block* sp_block;
    uint32_t temp=1;
    // 读取超级块
    if(read_data_block(0, buf) == ERROR){
        exit(-1);
    }
    sp_block = (struct sp_block*) buf_pointer;
    for(uint32_t i = 0; i < 32; i++){
        InodeID = i*32;
        if(sp_block->inode_map[i] != 0xffffffff){// 表示当前有空闲位置
            for(uint32_t j = 1; j <= 32; j++){
                if((sp_block->inode_map[i] & temp) == 0){// 表示当前遍历到了空闲位置
                    return InodeID;
                }
                InodeID++;
                temp = temp<<1;
            }
        }
    }
    //若没找到空闲位置，返回0，表示没找到合适的inode
    return 0;
}
/* 
* 函数名：findBlockID
* 函数功能：寻找没有被使用的数据块,从1到4063
*/
uint32_t findBlockID(){
    char *buf_pointer;
    char buf[Buffer_Length];
    uint32_t BlockID;
    buf_pointer = buf;
    sp_block* sp_block;
    uint32_t temp=1;// 用来做与运算
    // 读取超级块
    if(read_data_block(0, buf) == ERROR){
        exit(-1);
    }
    sp_block = (struct sp_block*) buf_pointer;
    for(uint32_t i = 0;i < 128; i++){
        BlockID = i*128;
        if(sp_block->block_map[i] != 0xffffffff){
            for(uint32_t j = 1; j <= 32; j++){
                BlockID++;
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
* 函数名：findinodepoint
* 函数功能：通过输入的indoe_id,找到指定的inode并返回该指针
*/
struct inode*
findinodepoint(uint32_t inode_id){
    char *buf_pointer;
    char buf[Buffer_Length];
    inode* inode;
    uint32_t block,num;
    block = (inode_id-1)/32;
    num = (inode_id-1)%32;
    if(read_data_block(block+1, buf) == ERROR){
        exit(-1);
    }
    buf_pointer += num*32;
    inode = (struct inode*)buf_pointer;
    return inode;
}
/* 
* 函数名：refreshSuperBlock
* 函数功能：更新超级块中的inode位图
* 入口参数意义：inode_id表示inode的id，从1~1024；type表示是占据该inode还是清除该数据块
*/
void refreshInodemapinSuperBlock(uint32_t inode_id, int type){
    char *buf_pointer;
    char buf[Buffer_Length];
    sp_block* sp_block;
    buf_pointer = buf;
    uint32_t block,num;
    uint32_t temp = 1;// 用于&或|运算
    // 读取超级块
    if(read_data_block(0, buf) == ERROR){
        exit(-1);
    }
    sp_block = (struct sp_block*) buf_pointer;
    block = (inode_id-1)/32;// 1~32映射为0，33~64映射为1，65~128映射为2
    num = (inode_id-1)%32;// 例如20映射为第0块的第20个inode，需要左移19位后&1运算
    temp = temp << num;
    if(type == Occupy){
        sp_block->free_inode_count --;
        sp_block->inode_map[block] |= temp; // 进行或运算，置1
    }else if(type == notOccupy){
        sp_block->free_inode_count ++;
        sp_block->inode_map[block] &= ~temp;// 取反后，进行与运算，即可让相应位置0
    }
    if(write_data_block(0, buf) == ERROR){
        exit(-1);
    }
}
/* 
* 函数名：refreshBlockmapinSuperBlock
* 函数功能：更新超级块中的blockmap数据块位图
* 入口参数意义：BlockID表示数据块编号，从1~4063；type表示该数据块是被占据还是被释放
*/
void refreshBlockmapinSuperBlock(uint32_t BlockID, int type){
    char *buf_pointer;
    char buf[Buffer_Length];
    sp_block* sp_block;
    buf_pointer = buf;
    uint32_t block,num;
    uint32_t temp = 1;// 用于&或|运算
    // 读取超级块
    if(read_data_block(0, buf) == ERROR){
        exit(-1);
    }
    sp_block = (struct sp_block*) buf_pointer;
    block = NUMBLOCK(BlockID)/32;// 1~4063映射到实际数据块的33~4095
    num = NUMBLOCK(BlockID)%32;// 例如第1块，在block[1]的第1块（从0开始编码） 
    temp = temp << num;
    if(type == Occupy){
        sp_block->free_block_count--;
        sp_block->block_map[block] |= temp; // 进行或运算，置1
    }else if(type == notOccupy){
        sp_block->free_block_count++;
        sp_block->block_map[block] &= ~temp;// 取反后，进行与运算，即可让相应位置0
    }
    if(write_data_block(0, buf) == ERROR){
        exit(-1);
    }
}
/* 
* 函数名：refreshInode
* 函数功能：根据inode_id找到指定inode块，并更新相应的信息
*/
void refreshInode(uint32_t inode_id, uint16_t file_type, uint32_t file_size){
    char *buf_pointer;
    char buf[Buffer_Length];
    inode* inode;
    buf_pointer = buf;
    int block,num;
    block = (inode_id-1)/32;// 1~32映射为0，33~64映射为1，65~128映射为2
    num = (inode_id-1)%32;
    if(read_data_block(block+1, buf) == ERROR){// +1因为要跳过超级块
        exit(-1);
    }
    buf_pointer += num*32;
    inode = (struct inode*)buf_pointer;
    inode->size += file_size;
    inode->file_type = file_type;
    if(write_data_block(block+1, buf) == ERROR){
        exit(-1);
    }
}
/**
 * 函数名：create
 * 函数功能：根据指定的文件名创建新目录
 * 入口参数：文件名，inode指针，filetype表示文件或目录
 */
uint32_t
create(char filename[121], uint32_t inodeID, uint8_t file_type, uint32_t file_size){
    char buf[Buffer_Length];
    char *buf_pointer;
    buf_pointer = buf;
    dir_item* dir_item;
    inode* inode;
    uint32_t newInodeID,BlockID;
    int temp_file_size;
    /* 根据输入的inodeID找到相应的inode */
    inode = findinodepoint(inodeID);
    /* 输入的inode是dir类型 */
    if(inode->file_type == IsDir){
        // 在输入的inode块中找到一个空闲目录项数据块
        for(int i = 0; i < BLOCK_POINT_NUM; i++){
            if((inode->block_point[i] != 0   // 若找到一个已经被使用过的数据块
            && inode->size < (i+1)*1024)    // 且小于该值表示该目录项数据块还有空余的空间
            || inode->block_point[i] == 0){ // 或者是找到了一个未被使用过的数据块
                // 读取该目录项数据块
                if(inode->block_point[i] == 0){// 需要选出一块未被使用的数据块，被这个指针所指向
                    BlockID = findBlockID();// 找出一块未被使用的数据块
                    inode->block_point[i] = NUMBLOCK(BlockID); // 使得指针指向此块新的数据块
                    refreshBlockmapinSuperBlock(BlockID, Occupy);/* 更新数据块位图 */
                }
                if((read_data_block(inode->block_point[i], buf)) == ERROR){
                    printf("error!\n");
                    exit(-1);
                }
                dir_item = (struct dir_item*) buf_pointer;
                for(int j = 0; j < DIR_ITEM_NUM; j++){// 遍历此数据块中的8个目录项，找到未使用的一个目录项
                    if(dir_item->inode_id == 0){// 找到了未使用的
                        dir_item->inode_id = findInodeID();
                        newInodeID = dir_item->inode_id;
                        dir_item->valid = 1;
                        dir_item->type = file_type;
                        strcpy(dir_item->name, filename);
                        // dir_item->name = filename;
                        /* 超级块需要更新占用目录项的数目 */
                        break;
                    }
                }
                // 写回该目录项数据块
                if((write_data_block(inode->block_point[i], buf)) == ERROR){
                    printf("write error!\n");
                    exit(-1);
                }
                refreshInodemapinSuperBlock(newInodeID, Occupy);// 新增了一个inode块被使用，更新超级块中的信息
                refreshInode(newInodeID, file_type, file_size);// 更新inode数组中的信息
                return newInodeID;
            }
        }
    }
    else if(inode->file_type == IsFile){// 到了创建文件的一步，是最终的步骤
        temp_file_size = file_size;
        for(int i = 0; i < BLOCK_POINT_NUM; i++){
            if(inode->block_point[i] == 0 && temp_file_size > 0){ // 找到了一个未被使用过的数据块
                BlockID = findBlockID();// 找出一块未被使用的数据块
                inode->block_point[i] = NUMBLOCK(BlockID); // 使得指针指向此块新的数据块
                refreshBlockmapinSuperBlock(BlockID, Occupy);// 更新数据块位图，表示该数据块存放了数据 
                inode->size = temp_file_size;
                temp_file_size -= 1024;
            }
            return 0;// 表示创建文件成功，没必要返回新inode_id
        }
    }
}

/* 
* 函数名：readInodeMessage
* 函数功能：能够读取inode链接的根目录项的信息
 */
int readInodeMessage(char *dirname[]){
    char buf[Buffer_Length];
    char *buf_pointer;
    buf_pointer = buf;
    inode* inode;
    dir_item* dir_item;
    int j=0;
    printf("hello\n");
    inode = findinodepoint(1);// 根据ID找到指定inode的地址
    
    for(int i = 0; i < MAXNUM_DIR; i++){ // 根目录最多可以存储6*8=48个目录
        dirname[i] = &lsName[i][0];
    }
    // 读取根目录数据
    for(int i = 0; i < BLOCK_POINT_NUM; i++){
        if(inode->block_point[i] != 0){ // 有所指向
            if(read_data_block(inode->block_point[i], buf) == ERROR){
                exit(-1);
            }
            dir_item = (struct dir_item*)buf_pointer;
            for(int k = 0; k < DIR_ITEM_NUM; k++){ // 遍历这一块数据块上的8个目录项
                if(dir_item->valid == 1){ // 若是有效的目录
                    strcpy(dirname[j++], dir_item->name); // 读取目录名
                }
                buf_pointer += 128; // 指向下一个目录项
                dir_item = (struct dir_item*)buf_pointer;
            }
        }
    }
    // 返回文件数
    return j;
}

