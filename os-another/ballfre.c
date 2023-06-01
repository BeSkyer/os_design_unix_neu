/**

*				文件说明：超级块的使用
*/
#include <stdio.h>
#include "filesys.h"

//用于接收组长块的内容
static unsigned int block_buf[BLOCKSIZ];
/*****************************************
*	函数名：balloc()
*	返回值：分配的磁盘块块号
*	功  能：分配一个空闲磁盘块
*	形  参：无
******************************************/

unsigned int balloc()
{
	unsigned int free_block;

	if (filsys.s_nfree == 0)//空闲块总块数为0，此时位于第二组的组长块位置1处
	{
		printf("\nDisk Full!!! \n");
		return DISKFULL;
	}
	//从超级块空闲块堆栈中取出一个空闲块指针指向的空闲块
	//这里的free_block依然是逻辑块号
	free_block = filsys.s_free[filsys.s_pfree];

	//只剩最后一个组长块，不需要在赋值 
	if (filsys.s_nfree == 1)
	{
		filsys.s_pfree--;
		filsys.s_nfree--;
		filsys.s_fmod = SUPDATE;
		return free_block;
	}

	//空闲块栈中的最后一个块--组长块被分配
	//默认数组下标1的位置放了组长块
	//将组长块中记录的上一组空闲块信息存入超级块中
	if (filsys.s_pfree == 1)
	{
		//设置文件指针fd的位置(文件指针,偏移量,基准)
		fseek(fd, DATASTART + free_block * BLOCKSIZ, SEEK_SET);
		//用于接收数据的起始地址,每个数据项的字节数,数据项数,输入流
		//将文件中对应的内容写入block_buf中，再用block_buf赋值给超级块
		fread(block_buf, BLOCKSIZ, 1, fd);

		//更新超级块
		for (int i = 0; i <= NICFREE; i++)
		{
			filsys.s_free[i] = block_buf[i];
		}
		//这里一定是50，因为每次分配空闲块都是从超级块中分配
		filsys.s_pfree = NICFREE;
	}
	//如果不是组长块，超级块长度减一
	else
		filsys.s_pfree--;	//修改空闲块指针-1

	filsys.s_nfree--;		//空闲块总块数-1
	filsys.s_fmod = SUPDATE;//超级块修改标志

	return free_block;
}
/*****************************************
*	函数名：bfree()
*	返回值：无
*	功  能：回收一个磁盘块
*	形  参：要回收的磁盘块块号（块号从0开始）
******************************************/
bfree(unsigned int block_num)
{
	filsys.s_nfree++;				//空闲块总数+1
	filsys.s_fmod = SUPDATE;		//超级块修改标志

	if (filsys.s_pfree == NICFREE)	/* if s-free full */
	{
		for (int i = 0; i <= NICFREE; i++)
		{
			//接收超级块中的空闲盘块信息，赋值给新生成的组长块
			block_buf[i] = filsys.s_free[i];
		}
		fseek(fd, DATASTART + BLOCKSIZ * block_num, SEEK_SET);
		//写入新生成的组长块
		fwrite(block_buf, BLOCKSIZ, 1, fd);
		filsys.s_pfree = 1;					//修改空闲块指针	
		filsys.s_free[0] = 1;				//修改空闲块栈中空闲块数
		filsys.s_free[1] = block_num;		//收回的空闲块号写入空闲块栈
	}
	else
	{
		filsys.s_free[0]++;								//空闲块栈中块数+1
		filsys.s_free[++(filsys.s_pfree)] = block_num;	//先将指针后移,再收回的空闲块号写入空闲块栈			
	}
}