/**

*		    修改:	1. 修改文件是否存在的判断错误
*					2. 添加注释
*					3. 分配用户打开文件表表项时的比较式子为if (user[user_id].u_ofile[j] == SYSOPENFILE + 1) break;
*					4. 修改记录在系统打开文件表中的位置为：user[user_id].u_ofile[j]=i;
*					5. 如果不是在文件尾端添加元素，则释放文件之旧的磁盘块
*					6. 添加文件已经打开的判断  ***
*/
#include <stdio.h>
#include "filesys.h"
/******************************
用于打开或创建文件，在打开或创建文件时可以指定文件的属性及用户的权限等各种参数。
user_id:用户id
filename: 文件名
openmode: 以什么方式打开
FREAD FWRITE FAPPEND
*******************************/
unsigned short aopen(int user_id, char* filename, unsigned short openmode)
{
	unsigned int dinodeid;
	struct inode* inode;
	int i, j;
	dinodeid = namei(filename);//在当前目录中寻找对应的i节点
	if (dinodeid == NULL)    /* nosuchfile */
	{
		//文件在当前目录中不存在
		printf(">file does not existed!\n");
		return NULL;
	}
	//存在则加载进内存
	inode = iget(dinodeid);
	//判读当前用户id在用户表中的位置是否有权限打开文件
	//这里权限不够
	if (!access(user_id, inode, openmode))    /* access denied */
	{
		printf(">Failed to open file due to unqualified authority!\n");
		iput(inode);
		return NULL;
	}
	/**********分配系统打开文件表项*********/
	for (i = 0; i < SYSOPENFILE; i++)
	{
		//找到空表项退出循环
		if (sys_ofile[i].f_count == 0) break;
		//已经打开
		if (sys_ofile[i].f_inode->i_ino == inode->i_ino)
		{
			printf(">File is open already!\n");
			return NULL;
		}
	}

	if (i == SYSOPENFILE)
	{//没找到空表项，表满
		printf(">Too much file open!\n");
		iput(inode);		//释放i节点
		return NULL;
	}
	//找到空表项
	sys_ofile[i].f_inode = inode;				//记录i节点
	sys_ofile[i].f_flag = openmode;			//记录操作
	sys_ofile[i].f_count = 1;					//引用计数+1
	if (openmode & FAPPEND)					//在文件尾端添加元素
		sys_ofile[i].f_off = inode->di_size;	//读或写指针移至文件尾端，对指针进行移动，做好写的准备
	else
		sys_ofile[i].f_off = 0;				//否则读或写指针移至文件头
	/**********分配用户打开文件表项*********/
	for (j = 0; j < NOFILE; j++)									//查找用户文件打开表
		if (user[user_id].u_ofile[j] == SYSOPENFILE + 1) break;		//找到空的表项退出循环
	if (j == NOFILE)
	{//没找到空的表项，表满
		printf(">Too much file opened by the user! \n");
		sys_ofile[i].f_count = 0;									//对应的系统打开表引用计数清零
		iput(inode);											//释放i节点
		return NULL;
	}
	//找到空的表项
	user[user_id].u_ofile[j] = i;									//记录在系统打开表中的位置

	//读写时需要将旧的文件磁盘块释放
	if ((openmode & FWRITE))
	{
		//将文件旧的磁盘块释放，不足一个磁盘的内容也全部释放
		//对文件来说磁盘块是基本单位，不会有两个文件同占用一个磁盘块
		for (i = 0; i < inode->di_size / BLOCKSIZ + (inode->di_size % BLOCKSIZ != 0); i++)
			bfree(inode->di_addr[i]);
		//将该i节点大小清空
		inode->di_size = 0;
		return j;
	}
	if ((openmode & FREAD))
	{
		//将文件旧的磁盘块释放，不足一个磁盘的内容也全部释放
		//对文件来说磁盘块是基本单位，不会有两个文件同占用一个磁盘块
		for (i = 0; i < inode->di_size / BLOCKSIZ + (inode->di_size % BLOCKSIZ != 0); i++)
			bfree(inode->di_addr[i]);
		return j;
	}
	//返回文件在用户打开表中的位置

}