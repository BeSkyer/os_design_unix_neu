#include <stdio.h>
#pragma once
#define _CRT_SECURE_NO_WARNINGS
#pragma warning(disable : 4996)  
// 定义块大小
#define BLOCKSIZ    512
// 系统打开文件数
#define SYSOPENFILE 40
// 目录项数目
#define DIRNUM      128
// 目录项名称长度
#define DIRSIZ      14
// 密码长度
#define PWDSIZ      12
// 密码数目
#define PWDNUM      32
// 进程打开文件数
#define NOFILE      20
// i节点地址数
#define NADDR       10
// i节点哈希表大小
#define NHINO       128
// 用户数目
#define USERNUM     10
// i节点结构体大小
#define DINODESIZ   32
// i节点数据块数目
#define DINODEBLK   32
// 文件数据块大小
#define FILEBLK     512
// 空闲块链表数目
#define NICFREE     50
// 空闲i节点链表数目
#define NICINOD     50
// i节点起始块号
#define DINODESTART 2*BLOCKSIZ
// 数据起始块号
#define DATASTART   (2+DINODEBLK)*BLOCKSIZ
// 空目录项标志
#define DIEMPTY     00000
// 普通文件标志
#define DIFILE      01000
// 目录文件标志
#define DIDIR       02000



// 用户读权限
#define UDIREAD     00001
// 用户写权限
#define UDIWRITE    00002
// 用户执行权限
#define UDIEXICUTE  00004


// 组读权限
#define GDIREAD     00010
// 组写权限
#define GDIWRITE    00020
// 组执行权限
#define GDIEXICUTE  00040


// 其他用户读权限
#define ODIREAD     00100
// 其他用户写权限
#define ODIWRITE    00200
// 其他用户执行权限
#define ODIEXICUTE  00400


// 读操作
#define READ        1
// 写操作
#define WRITE       2
// 执行操作
#define EXICUTE     3


// 默认权限   
#define DEFAULTMODE 00777
//new
#define USERMODE    00077
#define ROOTMODE    00007


// i节点更新标志
#define IUPDATE     00002
// 超级块更新标志
#define SUPDATE     00001


// 读取文件标志
#define FREAD       00001
// 写入文件标志
#define FWRITE      00002
// 追加写入文件标志
#define FAPPEND     00004


// 磁盘已满标志  返回error
#define DISKFULL    65535
// 文件起始位置标志 fseek origin
#define SEEK_SET    0


//新增 ROOT
//Root id为0
//未被使用的用户id为-1
#define ROOT 0


struct inode{
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
	unsigned short di_addr[NADDR];	//文件物理地址/物理块号 这里原来是int，改为了short
};

//磁盘i节点
//未读入内存中
struct dinode {
	unsigned short di_number;		//文件关联计数
	unsigned short di_mode;		//存取权限
	unsigned short di_uid;			//磁盘索引节点用户id
	unsigned short di_gid;			//磁盘索引节点组id
	unsigned long di_size;			//文件长度/大小
	unsigned short di_addr[NADDR];	//文件物理地址/物理块号 short改为int
};


struct hinodei {
	struct inode* i_forw;//哈希表指针
};



struct filsysi {
	unsigned short s_isize;
	unsigned long s_fsize;
	unsigned int s_nfree;
	unsigned short s_pfree;
	unsigned int s_free[NICFREE + 1];
	unsigned int s_ninode;
	unsigned short s_pinode;
	unsigned int s_inode[NICINOD];
	unsigned int s_rinode;
	char s_fmod;
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


extern struct inode* iget();
extern        iput();
extern unsigned int balloc();
extern        bfree();
extern struct inode* ialloc();
extern        ifree();
extern unsigned int namei();
extern unsigned short iname();
extern unsigned int access();
extern        _dir();
extern        mkdir();
extern        chdir();
extern unsigned short aopen();
extern        creat();
extern unsigned int read();
extern unsigned int write();
extern int    login();
extern        logout();
extern        install();
extern        format();
extern        close();
extern        halt();
extern        delete();
extern        deletefd();			//new 文件删除函数 原来叫deletefd  但是没有delete()
extern void   chpath();			//new 改变窗口中输出路径的函数
extern int	  xfa(char*);            //new 文件名与用户打开表位置转换函数
extern   void     cpy();            //new 复制
extern    void    pst();			//new 粘贴