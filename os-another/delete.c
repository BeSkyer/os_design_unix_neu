

/*删除文件函数delete( )*/
#include <stdio.h>
#include "filesys.h"
deletefd(int user_id, char* filename)
{
	unsigned int dinodeid, i;
	struct inode* inode;
	//获取磁盘i节点位置
	dinodeid = namei(filename);
	//读取到内存
	if (dinodeid != NULL)
		inode = iget(dinodeid);
	else
	{
		printf(">deleted file does not exist!\n");
		return;
	}
	//判断权限
	if (!access(user_id, inode, WRITE))    /* access denied */
	{
		printf(">failed to remove file because of no authority!\n");
		return;
	}
	//是目录或者文件
	if ((inode->di_mode & DIDIR) != 0 || (inode->di_mode & DIFILE) != 0)
	{
		char flag;
		//如果是目录
		if ((inode->di_mode & DIDIR) != 0)                    //如果是目录
			printf(">Are you sure to remove the “%s” directory?(y/n):", filename);
		else printf(">Are you sure to remove the “%s” file?(y/n):", filename);
		do
		{
			scanf("%c", &flag);
			_flushall();
			if (flag == 'y' || flag == 'Y')break;
			else if (flag == 'n' || flag == 'N')return;
			else continue;
		} while (1);
		//检查是否非空
		//如果是目录
		if ((inode->di_mode & DIDIR) != 0)
		{
			//磁盘1的位置放了root，0的位置不使用
			if (inode == iget(1)) {
				printf("root dir can't be deleted\n");
				return;
			}
			//跳转到要删除的目录中
			//判断要删除的目录是否为空
			chdir(ROOT, filename);
			//大于2就证明存在子目录，因为2用于存放索引指针
			if (dir.size > 2)
			{
				printf("DIR %s is NOT EMPTY. You can't remove an unEmpty DIR.\n");
				chdir(ROOT, "..");                    //跳回去
				return;
			}
			//如果不存在，先跳回去
			chdir(ROOT, "..");
		}
	}
	//找到当前要删除的目录或文件
	for (i = 0; i < dir.size; i++)
		if (dir.direct[i].d_ino == dinodeid) break;  //找到要删除的文件或者目录
	//将后面的目录和文件向前移动
	for (i++; i < dir.size; i++)                  //删除目录或文件名，后面的往前移
	{
		strcpy(dir.direct[i - 1].d_name, dir.direct[i].d_name);
		dir.direct[i - 1].d_ino = dir.direct[i].d_ino;
	}
	//腾出空间
	dir.direct[i - 1].d_ino = 0;
	dir.size = i - 1;
	cur_path_inode->di_size -= DIRSIZ + 2;
	//该用户不再使用该文件，此文件关联减一
	inode->di_number--;
	//如果关联为0，则进行置空
	if (inode->di_number == 0)inode->di_mode = DIEMPTY;
	//如果di_number为0则删除该数据块
	iput(inode);
}
