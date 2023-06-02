/**
*		    文件说明：文件权限的判断
*/
#include <stdio.h>
#include "filesys.h"

unsigned int authority(unsigned int user_id, struct inode* inode, unsigned short mode)
{
	//如果是root获取一切权限
	if (user[user_id].u_uid == ROOT)return 1;
	//当前对文件进行何种操作，读、写、可执行
	switch (mode)
	{
	case READ:
		if (inode->di_mode & ODIREAD) return 1;	//i节点对应的权限为对其他人可读
		if ((inode->di_mode & GDIREAD) && (user[user_id].u_gid == inode->di_gid)) return 1;//i节点对应的权限为同组用户可读
		if ((inode->di_mode & UDIREAD) && (user[user_id].u_uid == inode->di_uid)) return 1;//i节点对应的权限为所有者可读
		return 0;

	case WRITE:
		if (inode->di_mode & ODIWRITE) return 1;//i节点对应的权限为对其他人可写
		if ((inode->di_mode & GDIWRITE) && (user[user_id].u_gid == inode->di_gid)) return 1;//i节点对应的权限为同组用户可写
		if ((inode->di_mode & UDIWRITE) && (user[user_id].u_uid == inode->di_uid)) return 1;//i节点对应的权限为所有者可写
		return 0;

	case EXICUTE:
		if (inode->di_mode & ODIEXICUTE) return 1;//i节点对应的权限为对其他人可执行
		if ((inode->di_mode & GDIEXICUTE) && (user[user_id].u_gid == inode->di_gid)) return 1;//i节点对应的权限为同组用户可执行
		if ((inode->di_mode & UDIEXICUTE) && (user[user_id].u_uid == inode->di_uid)) return 1;//i节点对应的权限为所有者可执行
		return 0;

	default: return 0;
	}
}
