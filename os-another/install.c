/**

*		修改：	1. fd = fopen("filesystem", "w+r+b"); 改为 r+b
				2. 添加初始化装载password的功能
				3. 注释标明
*/
#include <stdio.h>
#include <string.h>
#include "filesys.h"

install()
{
	int i, j;
	/**********装载file column **********/
	fd = fopen("filesystem", "r+b");//以二进制可读写方式打开
	if (fd == NULL)
	{
		printf("\nfilesys can not be loaded\n");
		exit(0);
	}
	/*************装载password*****************/
	//0号物理块存放了root的fcb
	//1号物理块存放了etc的fcb
	//2号物理块存放了password的18个用户口令信息
	//0号i节点不使用
	//1号i节点对应root信息
	//2号i节点对应etc信息
	//3号i节点对应password信息
	//一共18个用户，这里只显示4个
	{
		fseek(fd, DATASTART + 2 * BLOCKSIZ, SEEK_SET);
		fread(pwd, BLOCKSIZ, 1, fd);
		for (int i = 0; i < PWDNUM; i++) {
			if (strcmp(" ", pwd[i].username) != 0) {
				printf("%s", pwd[i].username);
				printf("   %s\n", pwd[i].password);
			}
			else break;
		}
	}
	/**********从超级块中读入文件系统 **********/
	//超级块占用两个磁盘块，包含磁盘空间管理和i节点空间管理
	fseek(fd, BLOCKSIZ, SEEK_SET);
	fread(&filsys, sizeof(struct filsysi), 1, fd);

	/**********初始化内存i节点的哈希表**********/
	for (i = 0; i < NHINO; i++)//NHINO=128
	{
		//先将每一桶都置为空
		hinode[i].i_forw = NULL;
	}

	/**********初始化系统打开文件表**********/
	for (i = 0; i < SYSOPENFILE; i++)//SYSOPENFILE=40
	{
		sys_ofile[i].f_count = 0;					//文件引用计数清零
		sys_ofile[i].f_inode = NULL;				//内存i节点指针清零
	}

	/**********初始化用户表 **************/
	for (i = 0; i < USERNUM; i++)//USERNUM=10
	{
		user[i].u_uid = -1;						//用户ID清空
		user[i].u_gid = -1;						//用户组别ID清空
		for (j = 0; j < NOFILE; j++) // FILE = 20        //用户打开文件表清空
		{
			//用户打开文件表表项初始化为(SYSOPENFILE+ 1)=41，41默认该表项没有文件打开
			user[i].u_ofile[j] = SYSOPENFILE + 1;
		}
	}

	/**********将根目录读入当前目录中 **********/
	//读入root目录节点
	cur_path_inode = iget(1);
	//当前目录初始化size，还差i节点的fcb没有读入dir中
	dir.size = cur_path_inode->di_size / (DIRSIZ + 2);	//目录项的个数，即当前目录大小
	//初始化fcb中的name以及i节点，0是不用的i节点
	for (i = 0; i < DIRNUM; i++)
	{
		strcpy(dir.direct[i].d_name, "              ");
		dir.direct[i].d_ino = 0;						//0号i节点是不使用的
	}

	/********** 5.1 将磁盘中所有的目录项存入目录表 **********/
	/// BLOCKSIZ / (DIRSIZ + 2)					每块磁盘块能存下的目录项个数32项
	/// dir.size / (BLOCKSIZ / (DIRSIZ + 2))	所有的目录项占用的磁盘块数 最多是4块 = 128 /32
	int x = dir.size / (BLOCKSIZ / (DIRSIZ + 2));
	//根目录下的fcb有没有用超过一个磁盘块存储，也就是除了0磁盘块，有没有用别的磁盘块存储
	//保证目录占满了整个磁盘
	for (i = 0; i < x; i++)
	{
		/// cur_path_inode->di_addr[i]			目录所在的磁盘块块号 i多可能取值为0 1 2 3 因为所有目录最多能占用4块磁盘块
		/// BLOCKSIZ / (DIRSIZ + 2)				作为跳数，从磁盘读一个磁盘块存入目录表
		//读取root的inode中的磁盘空间数组，一共di_addr[10]，可以存储320个目录fcb，但是规定只有128个
		fseek(fd, DATASTART + BLOCKSIZ * cur_path_inode->di_addr[i], SEEK_SET);
		//每一次都读32个目录项，保证全部读入
		fread(&dir.direct[(BLOCKSIZ / (DIRSIZ + 2)) * i], BLOCKSIZ, 1, fd);
	}
	/********** 5.2 存入不足一个磁盘块大小的目录项 **********/
	/// cur_path_inode->di_size % BLOCKSIZ		剩余的目录项占的字节数，即占用磁盘块的有效字节数
	fseek(fd, DATASTART + BLOCKSIZ * cur_path_inode->di_addr[i], SEEK_SET);
	fread(&dir.direct[BLOCKSIZ / (DIRSIZ + 2) * i], cur_path_inode->di_size % BLOCKSIZ, 1, fd);
}
