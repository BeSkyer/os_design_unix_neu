/**

*			文件说明：关闭目录或者文件
*/
#include <stdio.h>
#include "filesys.h"

/*****************************************
*	函数名：close
*	返回值：无
*	功能：修改用户打开文件表，修改系统打开文件表
*	形参：用户id，在用户打开文件表中的位置
******************************************/
//关闭文件或者目录函数的调用
close(unsigned int user_id, short cfd)
{
	//此时文件已读入内存，加载内存i节点
	struct inode* inode;
	//user[user_id]找到该用户
	//u_ofile[cfd]找到用户打开文件表的表项中指向系统文件表项的内容，也就是指针
	//sys_ofile[user[user_id].u_ofile[cfd]]获取系统文件表内容
	//sys_ofile[user[user_id].u_ofile[cfd]].f_inode找到内存i节点
	inode = sys_ofile[user[user_id].u_ofile[cfd]].f_inode;
	//释放该用户的内存i节点
	if (inode != NULL)
		iput(inode);
	//在系统文件表中，让该文件引用次数减一
	sys_ofile[user[user_id].u_ofile[cfd]].f_count--;
	//用户能打开的最大表项+1
	user[user_id].u_ofile[cfd] = SYSOPENFILE + 1;
}
