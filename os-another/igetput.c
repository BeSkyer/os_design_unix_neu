/**

				文件说明：内存i节点与磁盘i节点的转换与映射
*/
#include <stdio.h>
#include "filesys.h"

/*****************************************
*	函数名：iget()
*	返回值：内存i节点指针
*	功  能：生成一个内存i节点
*	形  参：磁盘i节点id
******************************************/
//将磁盘中的i节点读入内存中
struct inode* iget(unsigned int dinodeid)
{
	int existed = 0, inodeid;
	long addr;
	struct inode* temp = NULL, * newinode;
	//对磁盘中的i节点进行hash，一共512个i节点
	inodeid = dinodeid % NHINO;			//哈希/散列函数索引

	//通过哈希索引（哈希链表）组织内存i节点的逻辑结构，图示见课件
	//这里设定每行的首个，即hinode[inodeid]是不用的
	//如果不为空说明，该内存i节点不是该行的首项
	if (hinode[inodeid].i_forw != NULL)
	{
		temp = hinode[inodeid].i_forw;
		//遍历该行所有节点
		while (temp)
		{
			if (temp->i_ino == dinodeid)//原来有错误  修改：inodeid->dinodeid
			{/* existed */
				existed = 1;
				temp->i_count++;		//引用计数+1
				return temp;
			}
			else/* not existed */
				temp = temp->i_forw;
		}
	}
	//如果哈希链表中没有要找的内存i节点，该行不存在，或者该行没有
	//说明此i节点还未被分配，下面进行分配
	/* 1. calculate the addr of the dinode in the file sys column */
	addr = DINODESTART + dinodeid * DINODESIZ;
	/* 2. malloc the new mode */
	newinode = (struct inode*)malloc(sizeof(struct inode));	//分配一个新的内存i节点
	/* 3. read the dinode to the mode */
	//读取磁盘i节点的对应信息
	fseek(fd, addr, SEEK_SET);
	//写入内存i节点的对应位置
	fread(&(newinode->di_number), DINODESIZ, 1, fd);				//从磁盘中读出对应的磁盘i节点的内容，写到内存i节点相应的部分
	/* 4. put it into hinode[inodeid] queue */

	//该行为空,新分配的i节点是该行首项
	if (hinode[inodeid].i_forw == NULL)
	{
		hinode[inodeid].i_forw = newinode;
		//如果是首项，则前指针为空
		newinode->i_forw = NULL;
		//后指针指向哈希表首地址
		newinode->i_back = &hinode[inodeid];
	}
	//该行不空，插入哈希表末尾
	else
	{
		//获取首项的内存i节点地址
		temp = hinode[inodeid].i_forw;
		//逐步向前寻找，直到末尾
		while (temp->i_forw)
			temp = temp->i_forw;
		//temp移动到行尾
		//行尾链接入新的i节点
		//新i节点成为行尾
		temp->i_forw = newinode;
		newinode->i_back = temp;
		newinode->i_forw = NULL;
	}

	/* 5. initialize the mode */
	newinode->i_count = 1;				//引用计数，当前有程序使用该节点
	newinode->i_flag = 0;				/* flag for not update */
	newinode->i_ino = dinodeid;			//磁盘i节点编号，标识内存i节点

	return newinode;					//返回新分配的i节点
}

/*****************************************
*	函数名：iput()
*	返回值：无
*	功  能：将内存i节点释放，同时考虑是否需要删除内存中的该i节点索引，如果当前进程不再使用该文件
*	形  参：内存i节点指针
******************************************/
iput(struct inode* pinode)
{
	long addr;
	unsigned int block_num;
	//查看当前还有没有用户的其他进程调用该文件
	if (pinode->i_count > 1)//若引用计数>=2
	{
		pinode->i_count--;	//引用计数-1
		return;
	}
	//如果没有则进行释放
	else					//若引用计数=1
	{
		//文件标识符 如果不是删除，就将i节点信息写回磁盘
		if (pinode->di_number != 0)
		{
			/* write back the mode */
			//重要的一步，说明该i节点是否被使用，而不是空闲态
			addr = DINODESTART + pinode->i_ino * DINODESIZ;
			fseek(fd, addr, SEEK_SET);
			fwrite(&(pinode->di_number), DINODESIZ, 1, fd);//将内存i节点对应的磁盘i节点部分的内容写入磁盘
		}
		//如果di_number=0，则证明此时是想要将i节点对应的磁盘内容也删除
		else
		{
			/* rm the mode & the block of the file in the disk */
			//此内存i节点对应的文件所占的磁盘块数
			block_num = pinode->di_size / BLOCKSIZ;
			for (int i = 0; i < block_num; i++)				 //释放磁盘块，一次一块
			{
				bfree(pinode->di_addr[i]);					 //已修改balloc为bfree

			}
			ifree(pinode->i_ino);							 //释放磁盘i节点
		}

		/* free the mode in the memory */
		//操作哈希链表，将此内存i节点从链表移除
		//此i节点是链的最后一个
		if (pinode->i_forw == NULL)
			pinode->i_back->i_forw = NULL;
		//如果不是最后一个
		else
		{
			pinode->i_forw->i_back = pinode->i_back;
			pinode->i_back->i_forw = pinode->i_forw;
		}
		free(pinode);
	}
}