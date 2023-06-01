/**

*			修改： 1.判断逻辑
*/

#include <stdio.h>
#include "filesys.h"

/********************************
*	函数名：xfa()
*	返回值：-1或者某一行
*	功能：判断是否在用户打开文件表中，存在则返回是哪一行
*	形参：文件名
**********************************/
int xfa(char* filename)
{
	int i, j, ino;
	int flag = 1;
	//在当前目录中查找
	ino = namei(filename);
	//是否存在该文件
	if (ino == NULL)
		flag = 0;
	//查找用户打开文件表
	for (i = 0; i < NOFILE; i++)
	{
		//user[user_id].u_ofile[cfd] = SYSOPENFILE + 1;			//标识空闲
		//当前表项空闲，不做处理
		if (user[user_id].u_ofile[i] == SYSOPENFILE + 1);
		//如果不空闲，并且等于该文件的i节点磁盘号，则退出
		else
		{
			int t = user[user_id].u_ofile[i];
			//t为当前用户的用户打开文件表中的某一项，值为在系统打开文件表中的位置
			//sys_ofile[t].f_inode->i_ino为系统打开表某一表项对应的i节点的i节点号
			//二者相等，说明该文件存在当前目录并且有被打开
			if (sys_ofile[t].f_inode->i_ino == ino)break;
		}

	}
	if (i == NOFILE)
		flag = 0;
	if (flag)//要关闭的文件已经被打开，可以进一步处理
		return i;//返回在用户文件打开表中的位置
	return -1;
}