/**

*            修改:	1. 创建新的文件时文件初始化大小为1个盘块的大小
*					2. 原来初始化时新建的文件会立马加入系统打开文件表和用户文件打开表，这里我们不要
*                   3. 将放目录的过程放在最后  ***
*					4. 修改了对i节点的addr的初始化 ***
*					5. 添加了函数的参数 size ***
					6. 最后加了iput
*/
#include <stdio.h>
#include "filesys.h"

/***********************************
*	函数名：creat()
*	返回值：无
*	功能：创建文件或者目录
*	形参：创建人，要创建的文件名，文件权限
***********************************/
unsigned int creat(int user_id, char* filename, unsigned short mode)
{
	unsigned int di_pos, di_ino;
	//内存i结点
	struct inode* inode;
	unsigned int i, j;
	//当前目录是否存在同名文件或目录
	di_ino = namei(filename);
	//存在则返回
	if (di_ino != NULL)
	{
		printf(">'%s' has already existed as file or subdirectory\n", filename);
		return -1;
	}
	//否则创建
	else
	{
		//找当前目录的i结点在磁盘中的序号
		di_ino = namei(".");
		//将当前目录读入内存
		inode = iget(di_ino);
		//查看当前目录的权限，写和执行，不足则返回
		//控制当前目录有无权限创建
		if (!access(user_id, inode, WRITE) || !access(user_id, inode, EXICUTE))
		{
			printf(">failed to creat file because of no authority!\n");
			iput(inode);
			return -1;
		}
		//查询结束
		iput(inode);
		//生成内存i节点
		inode = ialloc();
		//文件权限和类型 权限为ROOTMODE(创建者rwx) GDIREAD（同组可读r--）
		inode->di_mode = mode;
		//用户id
		inode->di_uid = user[user_id].u_uid;
		//用户组id
		inode->di_gid = user[user_id].u_gid;
		//文件大小
		inode->di_size = 0;
		//文件关联计数为1
		inode->di_number = 1;
		int i;
		//因为没有内容，所以该i节点不指向磁盘块
		for (i = 0; i < NADDR; i++)
		{
			inode->di_addr[i] = -1;
		}
		//当前目录下的空目录项，一共128个目录最多
		//找到当前目录下的空闲目录号
		di_pos = iname(filename);
		//分配失败，返回
		if (di_pos == -1)return;
		//分配成功，目录长度+1，但是不能超过128
		dir.size++;
		//对磁盘i节点逻辑序号进行赋值
		dir.direct[di_pos].d_ino = inode->i_ino;
		//修改当前路径i节点的文件大小，文件名长度+i节点索引指针长度
		cur_path_inode->di_size += (DIRSIZ + 2);
		iput(inode);
	}
}