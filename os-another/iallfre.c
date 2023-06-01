/**

*				1. 修改ialloc函数
					1.1 增加：报告磁盘i节点分配完
					1.2 修改：while ((count < NICINOD) && (count <= filsys.s_ninode))
					1.3 修改：找到一个空闲i节点后，对缓冲区block_buf的访问标号i加1，cur_di总是和i同步加1
					1.4 增加：while ((count <= NICINOD) && (count <= filsys.s_ninode)) 增加对铭记i节点的最终更新 判断if (count != NICINOD)
					4.5 增加：对能否读入一个磁盘块大小的空间的判断，以及读入的方法，即if (cur_di <= (BLOCKSIZ * ( DINODEBLK - 1) / DINODESIZ))部分
*				2. 修改ifree函数
					2.1 修改：判断空闲i节点栈未满的条件
					2.2 修改：为filsys.s_pinode--;
					2.3 删除：filsys.s_inode[NICINOD] =dinodeid;
					2.4 增加：回收的i节点标志为空闲：block_buf[0].di_mode = DIEMPTY，将修改信息写入改磁盘i节点
*/
#include <stdio.h>
#include "filesys.h"

//每个磁盘可分配16个i节点
static struct dinode block_buf[BLOCKSIZ / DINODESIZ];

/*****************************************
*	函数名：ialloc()
*	返回值：分配的磁盘i节点对应的内存i节点指针
*	功  能：分配一个空闲的磁盘i节点
*	形  参：无
******************************************/
struct inode* ialloc()
{
	int i;								//buf操作下标
	int count;							//计数找到的空闲磁盘i节点
	int block_end_flag;					//对一次分配的一个磁盘块大小的空间遍历完标志
	struct inode* temp_inode;			//暂存i节点
	unsigned int cur_di;				//临时铭记i节点

	//判断当前空闲i节点数，如果为0证明512个i节点全部被分配出去了
	if (filsys.s_ninode == 0)
	{
		printf(">Inode null!\n");		// 磁盘i节点使用完  
		return DISKFULL;
	}
	//如果空闲节点指针移动到50证明0-49空闲节点都已经被占用
	//需要更新超级块中的i节点空闲堆栈
	if (filsys.s_pinode == NICINOD)
	{
		i = 0;
		//对找到的空闲i节点计数
		count = 0;
		block_end_flag = 1;

		//准备更新超级块中的空闲i节点堆栈
		//首先指向堆栈的最后一个位置，准备将此处更新为空闲i节点
		filsys.s_pinode = NICINOD - 1;	//更新空闲i节点指针为49，即栈中的最后一个i
		//获取最近的空闲i节点，也就是铭记i节点
		cur_di = filsys.s_rinode;

		//当找到的空闲i节点没有超过50，并且没有超过剩余的空闲i节点时，可以继续查找
		//可以确定这里s_ninode不为0，所以从铭记节点向后移动，一定可以找到空闲磁盘块
		while ((count <= NICINOD) && (count <= filsys.s_ninode))
		{
			//是否找到了block_buf[16]长度的i节点，也就是能否开始下一个磁盘的寻找
			if (block_end_flag)
			{
				//铭记节点所处位置小于496，后面还有一个磁盘大小的位置可能存在空闲节点
				//因为铭记节点是已知的最小空闲节点
				if (cur_di <= (BLOCKSIZ * (DINODEBLK - 1) / DINODESIZ))
				{
					// 将铭记节点后的一个磁盘大小的内容直接读入block_buf中，注意这里包括铭记i节点
					fseek(fd, DINODESTART + cur_di * DINODESIZ, SEEK_SET);
					// 读blocksiz大小的内容到block_buf中，此时刚好读入512个字节给16大小的数组，每个元素32大小
					fread(block_buf, BLOCKSIZ, 1, fd);
					// 标志block_buf已经满，可以开始处理这一个磁盘，查看有多少空闲节点
					block_end_flag = 0;
					i = 0;
				}
				//铭记节点大于496，说明不足一个磁盘大小可能存在空闲节点
				else
				{
					fseek(fd, DINODESTART + cur_di * DINODESIZ, SEEK_SET);
					// 读不足512大小的内容给block_buf，因为512x32-496x32=512
					fread(block_buf, BLOCKSIZ * DINODEBLK - cur_di * DINODESIZ, 1, fd);
					block_end_flag = 0;
					i = 0;
				}
			}

			//开始查找当前磁盘块中有多少已被占用的i节点
			//如果是上述第二种情况，当读取到block_buf没有被赋值的元素也会接着遍历，不影响，因为只有DIEMPTY才会退出
			//直到找到一个没有被占用的，标识为DIEMPTY的空闲i节点，并且当前铭记节点指向他
			//i代表了16个节点中的哪一个
			//如果第一个就退出，说明铭记i节点为空，可使用磁盘中的铭记i节点存储内容
			while ((block_buf[i].di_mode != DIEMPTY) && i < (BLOCKSIZ / DINODESIZ))
			{
				cur_di++;
				i++;
			}
			//如果当前i节点全部被使用，block_end_flag=1
			//说明此盘搜索完毕，没有空闲i节点
			if (i == BLOCKSIZ / DINODESIZ)
				block_end_flag = 1;
			//如果在磁盘上找到序号最小的空闲i节点
			else
			{
				//用于判断找到最终状态的铭记i节点，即找到50个i节点后，再继续查找至找到一个空闲i节点可以作为铭记i节点
				//当前找到的空闲i节点没有超过50个，可以加入超级块的空闲i节点堆栈，将指针上移
				if (count != NICINOD)
				{
					//存入空闲i节点堆栈中
					filsys.s_inode[filsys.s_pinode--] = cur_di;
					//数量++
					count++;
					//铭记到未处理的i节点之后的位置，释放时铭记节点会向前移动
					cur_di++;
					//block_buf继续移动
					i++;
				}
				//如果超过50个，则证明不需要再添加至超级块中，直接加1即可，会退出最上层的while循环
				else
					count++;
			}
		}

		filsys.s_rinode = cur_di;			  //修改铭记i节点
		filsys.s_pinode++;					  //空闲i节点退回一个，指向栈中的第一个空闲i节点   
	}

	//如果当前还有空闲堆栈中还有i节点没有被分配，则直接开始分配
	//获取当前空闲i节点的磁盘号4-511，读入内存中（Hash%128）
	temp_inode = iget(filsys.s_inode[filsys.s_pinode]);
	//这里不需要释放
	fseek(fd, DINODESTART + filsys.s_inode[filsys.s_pinode] * DINODESIZ, SEEK_SET);
	//分配出去的磁盘i节点的各种信息写入磁盘，证明该磁盘的这块已被使用，不再空闲
	fwrite(&(temp_inode->di_number), sizeof(struct dinode), 1, fd);
	filsys.s_pinode++;						  //空闲i节点指针+1，这里有可能到50
	filsys.s_ninode--;					      //空闲i节点块数-1
	filsys.s_fmod = SUPDATE;				  //修改超级块修改标志
	//返回该i节点读入内容的指针，等修改后再进行iput操作，改变mode
	return temp_inode;
}

/*****************************************
*	函数名：ifree()
*	返回值：无
*	功  能：回收一个磁盘i节点
*	形  参：要回收的磁盘i节点id
******************************************/
//从释放函数可以看出，通过s_pinode的大小可以判断出当前堆栈中有多少空闲i节点
//当s_pinode=0时，此时所有i节点都没有使用
//当s_pinode=49时，说明此时的空闲i节点堆栈都在忙碌状态，都已经被使用
ifree(unsigned dinodeid)
{
	filsys.s_ninode++;
	//s_pinode==0说明当前空闲i节点栈未被使用，全是空闲i节点
	if (filsys.s_pinode != 0)							//空闲i节点栈未满
	{
		//将空闲i节点指针上移动
		//所以释放时，不再按照i节点大小，从0往后排序，而是减一的地方放置空闲i节点
		filsys.s_pinode--;
		//将回收的空闲i节点写入空闲i节点栈的对应位置
		filsys.s_inode[filsys.s_pinode] = dinodeid;
	}
	// 若回收的i节点小于铭记节点
	else if (dinodeid < filsys.s_rinode)
		// 更新回收的i节点为铭记节点
		filsys.s_rinode = dinodeid;
	// 将该i节点标志为空闲
	block_buf[0].di_mode = DIEMPTY;
	// 将新的空闲i节点（本质是其内部修改后的信息）写入磁盘
	fseek(fd, DINODESTART + dinodeid * DINODESIZ, SEEK_SET);
	//这里由于什么信息都没添加，所以是空闲i节点
	fwrite(&(block_buf[0].di_number), DINODESIZ, 1, fd);
}