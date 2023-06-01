/**
*		    修改:	1.login函数修改
*						1.1 参数由(unsigned short uid, char	*passwd)变为(char* username, char	*passwd)
*						1.2 uid==pwd[i].p_uid 改了 strcmp(passwd, pwd[i].password) == 0
*						1.3 函数逻辑修改重写
*						1.4 注释添加
*						1.5 登陆的用户的权限为USERMODE,root用户则为ROOTMODE
*					2.logout函数修改
*						2.1 函数逻辑修改
*						2.2 注释添加
						2.3 添加注销后跳转回根目录的功能来修正登出后重登的错误
*/
#include <stdio.h>
#include "filesys.h"
/***************************************
登陆函数，实现用户名和密码的匹配和查询
username: 待匹配用户名
passwd: 待匹配密码
返回值： 当前用户的id
****************************************/
int login(char* username, char* passwd)
{
	int i, j;
	int match_flag = 0, ret = 0;
	unsigned short tempuid = 0, tempgid = 0;
	for (i = 0; i < PWDNUM; i++)
	{
		if (strcmp(username, pwd[i].username) == 0) break;//用户名匹配退出循环
	}
	if (i == PWDNUM)
	{//用户名不匹配
		printf(">Incorrect username. please retry\n");
		return -1;
	}
	//用户名匹配
	for (i = 0; i < PWDNUM; i++)
	{
		if (strcmp(passwd, pwd[i].password) == 0)
		{//密码匹配
			match_flag = 1;
			tempuid = pwd[i].p_uid;
			tempgid = pwd[i].p_gid;
			//返回用户在用户表中的位置
			ret = i;
			break;
		}
	}
	if (i == PWDNUM)
	{//密码不匹配,且已经到密码表尾部了
		printf(">Incorrect password. please retry\n");
		return -1;
	}
	//到这里用户名与密码均已经匹配
	for (j = 0; j < USERNUM; j++)
	{
		//搜索空闲用户表项
		if (user[j].u_uid == -1)
		{
			//找到空闲的用户表项，对用户ID，组ID，用户类别进行赋值
			//如果针对是否是root，给予不同的权限
			user[j].u_uid = tempuid;
			user[j].u_gid = tempgid;
			if (i == 0)user[j].u_default_mode = ROOTMODE;
			else user[j].u_default_mode = ROOTMODE;//USERMODE;
			/*else user[j].u_default_mode = USERMODE;*/
			break;
		}
	}
	if (j == USERNUM)
	{
		//已搜索到用户表最后一项仍然没有找到空闲用户表项
		printf(">Too many users. Please wait\n");
		return -1;
	}
	return ret;
}

int logout(uid)
	unsigned short uid;
{
		int i,j,sys_no;
		struct inode *inode;
		for(i=0; i<USERNUM; i++)
		if(uid==user[i].u_uid) break;
	if (i==USERNUM)
	{
		printf("\nno such a file\n");
		return NULL;
	}
	for (j=0;j<NOFILE; j++)
	{
		if (user[i].u_ofile[j]!=SYSOPENFILE+1)
	{
	/* iput the mode free the sys_ofile and clear the user-ofile */
	sys_no=user[i].u_ofile[j];
	inode = sys_ofile[sys_no].f_inode;
	iput(inode);
	sys_ofile[sys_no].f_count--;
	user[i].u_ofile[j] = SYSOPENFILE+ 1;
	}
	}
	return 1;
}