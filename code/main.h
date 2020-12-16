#include <stdint.h>

#define Buffer_Length 1024   // 缓冲区大小,是物理磁盘块大小的两倍
#define notFileorDir 0
#define IsFile 1            // 是文件
#define IsDir 2             // 是目录
#define SUCCESS 1           // 返回成功
#define ERROR -1            // 返回失败
#define INODE_NUM 32        // 给inode分配的数据块个数
#define DATA_BLOCK_NUM 4096-1-32 // 分配给数据存储的数据块
#define BLOCK_POINT_NUM 6   // 一个inode里有6个数据块指针
#define DIR_ITEM_NUM 8      // 一个数据块里有8个dir_item
#define NUMBLOCK(x) x+32    //将第1-4063块号的数据块映射到实际的数据块编号上（read从0开始）
                            //超级块+inode块共占据了第0~32，因此1映射到33，4063映射到4095
#define Occupy 1            // 表示有一块数据块或inode块被占据
#define notOccupy 0         // 表示有一块数据块或inode块被释放
#define MAXNUM_DIR 48
#define NAME_LENGTH 121
char lsName[MAXNUM_DIR][NAME_LENGTH];

// 超级块的表示
// 一个超级块占据了 4*4+4*128+4*32=656B
// 一个数据块 即两个物理磁盘块分配给超级块
typedef struct spuer_block {
    int32_t magic_num;          // 幻数,int32_t 由typedef signed int定义
    int32_t free_block_count;   // 空闲数据块数
    int32_t free_inode_count;   // 空闲inode数
    int32_t dir_inode_count;    // 目录inode数
    uint32_t block_map[128];    // 数据块占用位图
    uint32_t inode_map[32];     // inode占用位图，表示一共使用32个数据块存放inode
} sp_block;

// 文件的表示
// 一个inode占据了 4+2+2+4*6 = 32B
// 一个数据块1024B 是两块物理磁盘块 可以存放32个inode
typedef struct inode {
    uint32_t size;              // 文件大小
    uint16_t file_type;         // 文件类型（文件/文件夹）,uint_16 由typedef short int定义
    uint16_t link;              // 连接数
    uint32_t block_point[6];    // 数据块指针
} inode;

// 目录项结构体
// 一个目录项占据了 4+2+1+121 = 128B
// 一个数据块1024B 是两个物理磁盘块 可以存放8个目录项
typedef struct dir_item {
    uint32_t inode_id;          // 目录项一个更常见的叫法是 dirent(directory entry)
    uint16_t valid;             // 当前目录项表示的文件/目录的对应inode
    uint8_t type;               // 当前目录项类型(文件/目录)
    char name[121];             // 目录项表示的文件/目录的文件名/目录名
} dir_item;

/* main.c */
int read_data_block(unsigned int block_num, char* buf);
int write_data_block(unsigned int block_num, char* buf);
int Init();
void TestInit();
uint32_t findInodeID();
uint32_t findBlockID();
void refreshInodemapinSuperBlock(uint32_t inode_id, int type);
void refreshBlockmapinSuperBlock(uint32_t BlockID, int type);
void refreshInode(uint32_t inode_id, uint16_t file_type, uint32_t file_size);
uint32_t
create(char filename[121], uint32_t inodeID, uint8_t file_type, uint32_t file_size);
int readInodeMessage(char *dirname[]);
