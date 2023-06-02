/**

*/
#include <stdio.h>
#include "filesys.h"
halt()
{
	//内存i节点
	struct inode* inode;
	int i, j;
	/**************step1: 清空所有用户的相关记录*************************/
	for (i = 0; i < USERNUM; i++)   //关闭所有打开文件
	{
		if (user[i].u_uid != 0)//该用户表存在用户
		{
			for (j = 0; j < NOFILE; j++)
			{
				if (user[i].u_ofile[j] != SYSOPENFILE + 1)//默认41是关闭了用户当前的文件
				{
					//清空对应的系统打开表表项
					close(user[i].u_uid, user[i].u_ofile[j]);
					//关闭用户打开表中的文件
					user[i].u_ofile[j] = SYSOPENFILE + 1;
				}
			}
		}
	}
	/**************step2: 写回磁盘块并关闭文件系统************************/
	//将超级块的内容写回磁盘，保持内外存超级块内容一致
	fseek(fd, BLOCKSIZ, SEEK_SET);
	fwrite(&filsys, sizeof(struct filsysi), 1, fd);  //写回磁盘块
	fclose(fd);
	printf(">Good Bye！See You Next Time.\n");
	exit(0);
}