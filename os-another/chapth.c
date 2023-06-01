/**

*			文件说明：当前路径的显示
*/
#include <stdio.h>
#include "filesys.h"

/******************************************************************
*	函数名：chpath
*	返回值：无
*	功能：改变屏幕上precord的输出
*	形参：当前precord的输出，要改变的目录cname，当前路径i节点的逻辑序号
*******************************************************************/

void chpath(char* precord, char* cname, int lastInodeId)
{
	int i = 0;
	//直接返回
	if (strcmp(cname, ".") == 0)
		return;
	//返回上层
	else if (strcmp(cname, "..") == 0)
	{
		//指针移到末尾
		while (precord[i++] != '\0');
		i--;
		//判断当前是否是root层，是则直接返回
		if (precord[i - 1] == 't' && precord[i - 2] == 'o' && precord[i - 3] == 'o' && precord[i - 4] == 'r' && lastInodeId == 1)
			return;
		//否则对该层字符串进行截断
		for (int j = i; j >= 0; j--)
		{
			if (precord[j] == '/')
			{
				precord[j] = '\0';
				return;
			}
		}
	}
	//下一层直接跳转
	else
	{
		strcat(precord, "/");
		strcat(precord, cname);
	}
}