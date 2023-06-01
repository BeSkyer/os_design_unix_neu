
#include <stdio.h>
#define _CRT_SECURE_NO_WARNINGS
#pragma warning(disable : 4996)  

//磁盘空间示意图
/**********************/

//|超级块;块数2|i节点;块数32;512个i节点（0节点不用，1节点是root，2节点是etc，3节点是password）|数据区;块数512（0节点存放root下的fcb，1节点存放etc下的fcb，2节点存放18个用户信息）|

/**********************/
#define BLOCKSIZ    512						//每个磁盘块大小，0.5kb
#define SYSOPENFILE 40						//系统打开文件表最大项数40
#define DIRNUM      128						//每个目录所包含的最大目录项数
#define DIRSIZ      14						//每个目录项名字部分所占字节数（文件名长度）
#define PWDSIZ      12						//口令字（登录密码和用户名长度）
#define PWDNUM      32						//最多可设32个口令登录，最多设置512/32 = 16名用户
#define NOFILE      20						//用户打开文件表最大项数20
#define NADDR       10						//每个i节点最多指向10块，addr[0]~addr[9]，文件最大为5kb
#define NHINO       128						//共128个Hash链表，提供索引i节点（必须为2的幂），磁盘到内存的映射
#define USERNUM     10						//最多允许10个用户登录
#define DINODESIZ   32						//每个磁盘i节点所占字节（文件的属性，文件的10个数据块地址空间）
#define DINODEBLK   32						//所有磁盘i节点共占32个物理块
#define FILEBLK     512						//共有512个目录文件物理块
#define NICFREE     50						//超级块中空闲块数组的最大块数
#define NICINOD     50						//超级块中空闲i节点的个数
#define DINODESTART 2 * BLOCKSIZ				//i节点起始地址。因为超级块既要存储空闲块信息，还要存储空闲节点信息
#define DATASTART   (2 + DINODEBLK) * BLOCKSIZ  //目录、文件区（数据区）起始地址

#define DIEMPTY     00000//类型为空
#define DIFILE      01000//类型为文件
#define DIDIR       02000//类型为目录

//user
#define UDIREAD     00001//创建者可读
#define UDIWRITE    00002//创建者可写
#define UDIEXICUTE  00004//创建者可运行

//group
#define GDIREAD     00010//创建者同组可读
#define GDIWRITE    00020//创建者同组可写
#define GDIEXICUTE  00040//创建者同组可运行

//other
#define ODIREAD     00100//所有人可读
#define ODIWRITE    00200//所有人可写
#define ODIEXICUTE  00400//所有人可运行

//用户访问权限
#define READ        1
#define WRITE       2
#define EXICUTE     3

#define DEFAULTMODE 00777
#define USERMODE    00077
#define ROOTMODE    00007

//i_flag
#define IUPDATE     00002

//s_fmod
#define SUPDATE     00001 //超级块修改标志

//f_flag
#define FREAD       00001
#define FWRITE      00002
#define FAPPEND     00004

//磁盘已满 返回error
#define DISKFULL    65535

//fseek origin
#define SEEK_SET    0

//Root id为0
//未被使用的用户id为-1
#define ROOT 0

//内存i节点-文件系统数据结构
//已读入内存中
struct inode {
	struct inode* i_forw;			//链接指针
	struct inode* i_back;			//链接指针
	char i_flag;					//状态，指示i节点是否上锁或被修改
	unsigned int i_ino;			//磁盘索引节点编号，标识内存索引节点，该内存节点对应的磁盘i节点的编号
	unsigned int i_count;			//访问/引用计数

	unsigned short di_number;		//文件关联计数
	unsigned short di_mode;		//存取权限及类型
	unsigned short di_uid;			//磁盘索引节点用户id
	unsigned short di_gid;			//磁盘索引节点组id
	unsigned long di_size;			//文件长度/大小
	unsigned short di_addr[NADDR];	//文件物理地址/物理块号
};

//磁盘i节点
//未读入内存中
struct dinode {
	unsigned short di_number;		//文件关联计数
	unsigned short di_mode;		//存取权限
	unsigned short di_uid;			//磁盘索引节点用户id
	unsigned short di_gid;			//磁盘索引节点组id
	unsigned long di_size;			//文件长度/大小
	unsigned short di_addr[NADDR];	//文件物理地址/物理块号
};

//查找内存索引节点的哈希表
struct hinodei {
	struct inode* i_forw;//哈希表指针
};

//超级块-系统文件表
struct filsysi {
	unsigned short s_isize;			//i节点块块数，FCB总数
	unsigned long s_fsize;			//数据块总块数，数据空间管理
	unsigned int s_nfree;			//空闲块总块数，空闲空间管理
	unsigned short s_pfree;			//空闲块指针，指向当前超级块的栈顶，下标从10开始递减
	unsigned int s_free[NICFREE + 1];	//空闲磁盘块堆栈，超级块中的空闲盘块，s_free[0]存放块内的空闲块个数
	unsigned int s_ninode;			//空闲i节点数，可用作新文件索引的i节点
	unsigned short s_pinode;		//空闲i节点指针，下标从0开始递增
	unsigned int s_inode[NICINOD];	//空闲i节点数组
	unsigned int s_rinode;			//铭记i节点
	/*铭记ｉ节点是保存在ｉ节点数组中的
	最后一个ｉ节点，如果系统分配到铭记ｉ节点
	时，则启动I / O设备，从铭记ｉ节点开始，重新搜索
	磁盘上的空闲ｉ节点，然后写进i结点数组。
	这可以确保系统不浪费时间去读那些已不含空闲ｉ节点的磁盘块*/
	char s_fmod;					//超级块修改标志
};

//用户口令字
struct pwdi {
	short p_uid;	//用户id
	short p_gid;	//用户组别id
	char username[PWDSIZ];	//用户名
	char password[PWDSIZ];	//密码
};

//目录项结构，FCB的结构
struct direct {
	char d_name[DIRSIZ];	//文件名
	unsigned short d_ino;	//i节点号，磁盘中的i节点号
};

//目录
struct diri {
	struct direct direct[DIRNUM];	//目录表，每个目录中最多包含128个目录项
	int size;						//当前目录下的目录项个数
};

//系统打开表表项
struct file {
	char f_flag;			//文件操作标志
	unsigned int f_count;	//文件引用计数
	struct inode* f_inode;	//指向内存i节点
	unsigned long f_off;	//读或写指针
};

//用户打开表表项
struct useri {
	unsigned short u_default_mode;	//用户类别
	short u_uid;			//用户ID
	short u_gid;			//用户组ID
	unsigned short u_ofile[NOFILE];	//用户打开文件表
};

extern struct hinodei hinode[NHINO];			//内存i节点哈希表
extern struct diri dir;							//当前目录=内存中的目录
extern struct file sys_ofile[SYSOPENFILE];		//系统打开文件表
extern struct filsysi filsys;					//内存中的超级块
extern struct pwdi pwd[PWDNUM];					//密码表
extern struct useri user[USERNUM];				//用户表
extern        FILE* fd;							//the file system column of all the system
extern struct inode* cur_path_inode;			//当前目录i节点
extern int    user_id;							//user_id
extern char* hehe;                   //new 复制缓冲区

//主要函数
extern struct inode* iget(int);	//索引节点内容获取函数
extern        iput();			//索引节点内容释放函数
extern unsigned int balloc();	//磁盘块分配函数
extern        bfree();			//磁盘块释放函数
extern struct inode* ialloc();	//分配索引节点区函数
extern        ifree();			//释放索引节点区函数
extern unsigned int namei();	//目录搜索函数
extern unsigned short iname();	//搜索当前目录下文件的函数
extern unsigned int access();	//访问控制函数
extern        _dir();			//显示目录和文件用函数
extern        mkdir();			//目录创建函数
extern		  mkdir_user();		//new 用户
extern  int   chdir();			//改变当前文件用函数
extern unsigned short aopen();	//打开文件函数
extern        creat();			//创建文件函数
extern unsigned int read();		//读文件用函数
extern unsigned int write();	//写文件用函数
extern int    login();			//用户登陆函数
extern        logout();			//用户退出函数
extern        install();		//进入文件系统函数
extern        format();			//文件格式化函数
extern        close();			//关闭文件函数
extern        halt();		//退出文件系统函数
extern        deletefd();			//new 文件删除函数 原来叫deletefd  但是没有delete()
extern void   chpath();			//new 改变窗口中输出路径的函数
extern int	  xfa(char*);            //new 文件名与用户打开表位置转换函数
extern   void     cpy();            //new 复制
extern    void    pst();			//new 粘贴
//extern       singup();

extern void clear_input_buffer();