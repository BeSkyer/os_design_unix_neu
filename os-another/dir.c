/**

*		    修改:		1. _dir()修改
*							1.1 修正输出目录项块链时的计数错误
*							1.2 修正输出权限的错误
*							1.3 修改输出的格式(文件名后两次制表符)
*						2. mkidr修改
*							2.1 创建目录函数中创建过程的错误
*							2.2 添加参数 user_id 以便实现多级目录
*							2.3 修改了填写空目录项时i节点获取的逻辑错误，改正后为dirid= inode->i_ino;
							2.4 创建新的目录要填写新的dir_buf时添加初始化功能，以免输出目录时产生目录
*						3. chdir修改
*							3.1 修改目录函数中判断权限的错误
*							3.2 修改返回类型
*							3.3 添加了权限参数force控制变量
*							3.4 修改了压缩目录数组时的错误
*							3.5 修改了chdir中赋予新cur_path_inode后没设置新的目录长度的问题
*/
#include <stdio.h>
#include <string.h>
#include "filesys.h"

/*************************
显示当前目录函数
**************************/
_dir()	/* _dir */
{
	unsigned int di_mode;
	int i, one, j, x = 0;
	//创建内存i节点
	struct inode* temp_inode;
	printf("CURRENT DIRECTORY :\n");
	//在当前目录中查找，顺序打印出目录
	for (int i = 0; i < dir.size; i++)
	{
		//当前目录的d_ino不为0,就证明存在目录，其中root的d_ino为1
		//dir指的是当前目录
		if (dir.direct[i].d_ino != DIEMPTY)
		{
			//输出文件名
			printf("%-20s", dir.direct[i].d_name);
			//获取对应的内存i节点
			temp_inode = iget(dir.direct[i].d_ino);
			//获取文件类型和存取权限
			di_mode = temp_inode->di_mode;
			//类型为目录则打印d
			if (di_mode & DIDIR)printf("d");
			//类型为文件则打印f
			else { printf("f"); x = 1; }

			/*****************输出存取权限*******************/
			//输出效果举例d(目录)rwx(创建者权限)r-x(与创建者同组权限)r-x(其他用户权限) 
			for (int j = 0; j < 9; j++)
			{
				int temp = j % 3;
				one = di_mode % 2;					//step1和step2共同完成判断权限的工作
				di_mode = di_mode / 2;
				if (one)
				{
					switch (temp)
					{
					case 0:printf("r"); break;
					case 1:printf("w"); break;
					case 2:printf("x"); break;
					default:printf(">error in print di_mode!\n");
					}
				}
				else
					printf("-");
			}
			printf("\t");
			/***************文件则输出大小与物理块号目录则输出提示***************/
			if (x)//i节点指向的是文件
			{
				//标志清空
				x = 0;
				//输出文件大小
				printf("file size%ld\t", temp_inode->di_size);
				//输出块链
				printf("block chain:");
				int s = (temp_inode->di_size / BLOCKSIZ) + 1;
				for (j = 0; j < s; j++)
				{
					//如果分配时，该节点恰好没有空间分配，则该节点为65535，该磁盘标志位设为65535
					if (temp_inode->di_addr[j] == 65535)
						continue;
					//成功分配到了空间，此时显示占用的空间
					else
						printf("%d ", temp_inode->di_addr[j]);
				}
				printf("\n");
			}
			else
				printf("<dir>\n");								//i节点指向的是目录
			iput(temp_inode);
		}
	}//for
}

/**************************
创建目录函数
**************************/
mkdir(int user_id, char* dirname)
{
	int dirid, dirpos;
	struct inode* inode;
	struct direct buf[BLOCKSIZ / (DIRSIZ + 2)];
	unsigned int block;

	dirid = namei(dirname);//先在当前目录查找是否存在同名目录项
	if (dirid != NULL)
	{//存在冲突，不允许创建
		inode = iget(dirid);
		if (inode->di_mode & DIDIR)	//存在同名目录项
			printf("\n%s directory already existed! ! \n");
		else						//与文件名冲突
			printf("\n%s is a file name, &can't create a dir the same name", dirname);
		iput(inode);
		return;
	}
	else
	{//允许创建
		dirpos = iname(dirname);//在当前目录中查找空目录项
		if (dirpos == -1)
		{//当前目录已满
			printf(">dir is full\n");
			return;
		}
		else
		{//还有空目录项
			inode = ialloc();							//分配新的磁盘i节点，并读入内存
			dirid = inode->i_ino;						//获取新分配的i节点号
			dir.direct[dirpos].d_ino = dirid;			//填写磁盘i节点号
			dir.size++;									//当前目录长度+1
			/*	fill the new dir buf */
			for (int x = 0; x < BLOCKSIZ / (DIRSIZ + 2); x++)
			{//初始化要作为目录的数据块以免输出子目录时输出错误
				strcpy(buf[x].d_name, "");				//名字清空				
				buf[x].d_ino = 0;						//i节点号清空
			}
			strcpy(buf[0].d_name, ".");					//.是当前目录的意思
			buf[0].d_ino = dirid;						//当前目录就是新创建的目录
			strcpy(buf[1].d_name, "..");				//..是当前目录的上一级目录
			buf[1].d_ino = cur_path_inode->i_ino;		//上一级目录数据块对应的i节点在cur_path_inode中
			block = balloc();										//分配新的磁盘块*
			//将文件i节点内容写入磁盘
			fseek(fd, DATASTART + block * BLOCKSIZ, SEEK_SET);		//定位分配的磁盘
			fwrite(buf, BLOCKSIZ, 1, fd);							//写入磁盘
			inode->di_size = 2 * (DIRSIZ + 2);						//文件长度/大小
			inode->di_number = 1;									//文件关联计数
			inode->di_mode = user[user_id].u_default_mode | DIDIR;	//存取权限与类型
			inode->di_uid = user[user_id].u_uid;					//用户ID
			inode->di_gid = user[user_id].u_gid;					//用户组ID
			inode->di_addr[0] = block;								//物理块号
			iput(inode);
			return;
		}
	}
}

mkdir_user(int user_id, char* dirname)
{
	int dirid, dirpos;
	struct inode* inode;
	struct direct buf[BLOCKSIZ / (DIRSIZ + 2)];
	unsigned int block;

	dirid = namei(dirname);//先在当前目录查找是否存在同名目录项
	if (dirid != NULL)
	{//存在冲突，不允许创建
		inode = iget(dirid);
		if (inode->di_mode & DIDIR)	//存在同名目录项
			printf("\n%s directory already existed! ! \n");
		else						//与文件名冲突
			printf("\n%s is a file name, &can't create a dir the same name", dirname);
		iput(inode);
		return;
	}
	else
	{//允许创建
		dirpos = iname(dirname);//在当前目录中查找空目录项
		if (dirpos == -1)
		{//当前目录已满
			printf(">dir is full\n");
			return;
		}
		else
		{//还有空目录项
			inode = ialloc();							//分配新的磁盘i节点，并读入内存
			dirid = inode->i_ino;						//获取新分配的i节点号
			dir.direct[dirpos].d_ino = dirid;			//填写磁盘i节点号
			dir.size++;									//当前目录长度+1
			/*	fill the new dir buf */
			for (int x = 0; x < BLOCKSIZ / (DIRSIZ + 2); x++)
			{//初始化要作为目录的数据块以免输出子目录时输出错误
				strcpy(buf[x].d_name, "");				//名字清空				
				buf[x].d_ino = 0;						//i节点号清空
			}
			strcpy(buf[0].d_name, ".");					//.是当前目录的意思
			buf[0].d_ino = dirid;						//当前目录就是新创建的目录
			strcpy(buf[1].d_name, "..");				//..是当前目录的上一级目录
			buf[1].d_ino = cur_path_inode->i_ino;		//上一级目录数据块对应的i节点在cur_path_inode中
			block = balloc();										//分配新的磁盘块*
			//将文件i节点内容写入磁盘
			fseek(fd, DATASTART + block * BLOCKSIZ, SEEK_SET);		//定位分配的磁盘
			fwrite(buf, BLOCKSIZ, 1, fd);							//写入磁盘
			inode->di_size = 2 * (DIRSIZ + 2);						//文件长度/大小
			inode->di_number = 1;									//文件关联计数
			inode->di_mode = ODIWRITE | ODIEXICUTE | ODIREAD | DIDIR;	//存取权限与类型
			inode->di_uid = user[user_id].u_uid;					//用户ID
			inode->di_gid = user[user_id].u_gid;					//用户组ID
			inode->di_addr[0] = block;								//物理块号
			iput(inode);
			return;
		}
	}
}

/**********************************
修改目录函数(目录跳转)
force: 外部控制信号
		1:无视用户级别强制允许
		0:需要正常判断
dirname: 目标目录名
**********************************/
int chdir(int force, char* dirname)
{
	unsigned int dirid;
	struct inode* inode;
	unsigned short block;
	int i, j, low = 0, high = 0;
	dirid = namei(dirname);//在当前目录中寻找

	if (dirid == NULL)
	{//不在当前目录中
		printf(">%s does not existed\n", dirname);
		return -1;
	}
	else
	{//存在当前目录中，并找到了相应i节点号
		inode = iget(dirid);//获取内存i节点
		if (inode->di_mode & DIFILE)
		{//找到的是文件
			printf(">cannot use the command on file!\n");
			iput(inode);
			return -1;
		}
		if ((!force) && (!access(user_id, inode, EXICUTE)))
		{	//用户无执行命令的权限
			printf(">has not access to the directory %s\n", dirname);
			iput(inode);
			return -1;
		}
		//每次跳转之前都要对在本目录下进行的操作进行整合、更新
		//最重要的是进行聚合，方便以后工作的开展
		//dir是当前所在的目录，包含了当前目录下的文件数size，以及存储了FCB[128]数组
		//dir是存储在内存中的
		//每个FCB都对应着一个d_ino，也就是i节点磁盘号，root是1
		//先将目录中的fcb信息整合在一起，方便下面写入一整块磁盘中保存
		/********* pack the current directory **********/
		for (i = 0; i < dir.size; i++)
		{
			//如果当前目录项没有使用
			if (dir.direct[i].d_ino == 0)
			{
				//如果当中有被使用的目录项，则将目录项聚合在一起
				for (j = i + 1; j < DIRNUM; j++)
					if (dir.direct[j].d_ino != 0)
						break;
				memcpy(&dir.direct[i], &dir.direct[j], DIRSIZ + 2);
				dir.direct[j].d_ino = 0;
			}
		}
		//更新当前目录i节点所占用的磁盘块
		//cur_path_inode指向了当前目录i节点，注意和当前目录FCB的磁盘号指向的位置相同
		//也就是dir是描述当前目录下级信息
		//cur_path_inode描述了自身的信息
		/*	write back the current directory *//************************************************/

		//首先获得当前目录大小占用的磁盘块数
		//释放目录所占用的磁盘块
		//释放的信息仍然可以通过dir.direct[j]获取
		//也就是释放了磁盘上面的FCB信息，但是当前目录dir可以得到FCB信息
		for (i = 0; i < cur_path_inode->di_size / BLOCKSIZ + 1; i++)
		{
			bfree(cur_path_inode->di_addr[i]);
		}
		//为当前目录的i节点重新分配磁盘块，分配时根据现有目录多少，一块盘可以容纳32个目录
		//最多有128个目录项，每个磁盘容纳32个目录项
		//将目录dir的fcb信息写进一整块磁盘中保存，方便下次跳转时读取到dir
		for (i = 0, j = 0; i < dir.size; i += BLOCKSIZ / (DIRSIZ + 2), j++)
		{
			//分配一个磁盘块，容纳32个目录
			block = balloc();
			//将目录分配到这个磁盘块上存储
			cur_path_inode->di_addr[j] = block;
			//定位到该空闲磁盘处
			fseek(fd, DATASTART + block * BLOCKSIZ, SEEK_SET);
			//将dir.direct[j]的内容写入到该空闲磁盘处
			//将目录中的足够多的fcb信息重新写入磁盘
			//这里fwrite可以尽量写多
			fwrite(&dir.direct[j], BLOCKSIZ, 1, fd);
		}
		//更新cur_path_inode的大小/长度(字节)
		cur_path_inode->di_size = dir.size * (DIRSIZ + 2);
		//将当前目录释放，此时已经完成了，1.该目录i节点记录的信息更新（自身长度，指向文件的物理地址）
		//2.文件指向的物理地址保证是尽量紧凑的放在磁盘上
		iput(cur_path_inode);
		//将新目录的i节点赋值给cur_path_inode
		cur_path_inode = inode;
		//下一个目录的dir.size的获取
		dir.size = cur_path_inode->di_size / (DIRSIZ + 2);
		/*	read the change dir from disk */
		//从磁盘中读取连续的fcb到dir的fcb存储区中
		for (i = 0, j = 0; i < inode->di_size / BLOCKSIZ + (inode->di_size % BLOCKSIZ != 0); i++, j += BLOCKSIZ / (DIRSIZ + 2))
		{
			//读取新的目录数据块
			fseek(fd, DATASTART + inode->di_addr[i] * BLOCKSIZ, SEEK_SET);
			//读取fcb到dir
			fread(&dir.direct[j], BLOCKSIZ, 1, fd);
		}
		return;
	}
}