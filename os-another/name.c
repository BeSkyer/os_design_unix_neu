/* name.c*/
/**

*		修改：	1. line 21 : return i -> retrun dir.direct[1].d_ino
*				2. line 54 : strcpy(name,dir.direct[i].d_name) -> strcpy(dir.direct[i].d_name, name)
*				3. 修改了namei中的判断以及将i写成1的错误
*/
#include <string.h>
#include <stdio.h>
#include "filesys.h"
/***********************************
*	函数名：namei
*	返回值：磁盘节点号
*	功能：搜寻当前目录下文件的磁盘索引编号
*	形参：文件名
*************************************/
//查找当前目录下有无该文件
unsigned int namei(char* name)
{
	int i, notfound = 1;
	for (i = 0; ((i < dir.size) && (notfound)); i++)
	{
		//对比当前目录中的名字与要查找的文件名是有相同且对应的i节点号不为0
		//d_ino==0说明是空的目录项
		if ((strcmp(dir.direct[i].d_name, name) == 0) && (dir.direct[i].d_ino != 0))
			//返回该文件对应i节点在磁盘中的序号
			return dir.direct[i].d_ino;
	}
	/* notfind */
	return NULL;
};

	/**************************************
	*	函数名：iname
	*	返回值：空目录在目录表中的位置
	*	功能：搜寻当前目录下的空目录项
	*	形参：文件名
	****************************************/
	unsigned short iname(char* name)
	{
		int i, notfound = 1;
		//内存中一次能放128个目录项
		for (i = 0; ((i < DIRNUM) && (notfound)); i++)
		{	//存在空的目录项
			if (dir.direct[i].d_ino == 0)
			{
				notfound = 0;
				break;
			}
		}
		//找不到空的目录项数组
		if (notfound)
		{
			printf(">The current directory is full!\n");
			return -1;
		}
		//找到空的目录项数组
		else
		{	/*将要创建的目录(路径名)放到找到的空闲目录项数组中*/
			strcpy(dir.direct[i].d_name, name);
			//返回空闲目录号
			return i;
		}
	}

