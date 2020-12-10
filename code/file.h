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
struct inode {
    uint32_t size;              // 文件大小
    uint16_t file_type;         // 文件类型（文件/文件夹）,uint_16 由typedef short int定义
    uint16_t link;              // 连接数
    uint32_t block_point[6];    // 数据块指针
}

// 目录项结构体
// 一个目录项占据了 4+2+1+121 = 128B
// 一个数据块1024B 是两个物理磁盘块 可以存放8个目录项
struct dir_item {
    uint32_t inode_id;          // 目录项一个更常见的叫法是 dirent(directory entry)
    uint16_t valid;             // 当前目录项表示的文件/目录的对应inode
    uint8_t type;               //当前目录项类型(文件/目录)
    char name[121];             // 目录项表示的文件/目录的文件名/目录名
}