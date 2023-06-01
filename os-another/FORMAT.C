/**

*		修改：	1. 初始化超级块
					1.1 初始化空闲i节点
					1.2 课件上成组链接法的组织结构
				2. 初始i节点分配
					2.1 i密码占用的2号数据块的初始化
					2.2 由于pwd结构添加了username一项，所以pwd格式化大小做了相应修改
					2.3 修改了初始化密码数据区时的循环计数，以免发生堆栈溢出
					2.4 增加了i节点的初始化
				3. filesystem打开的方式由r+w+b改为wb
				4. 修改了成组连接分配法中的逻辑错误
*/
#include <stdio.h>
#include "filesys.h"
#include <stdlib.h>
#include <string.h>

/*****************************************
*	函数名：format()
*	返回值：无
*	功  能：格式化文件系统 初始化磁盘文件卷
*	形  参：无
******************************************/
format()
{
	//内存i节点指针
	struct inode* inode;
	//fcb数组，每个磁盘容纳的fcb数目，这里是32，初始化为0，所以若这里d_ino=0，证明此处fcb未被使用
	//每次跳转时保证fcb的紧凑性
	//这里用来存放主目录fcb数组信息
	struct direct dir_buf[BLOCKSIZ / (DIRSIZ + 2)] = { 0 };	   //BLOCKSIZ / (DIRSIZ+2)=512/16
	//用户数组，每个用户占用28长度的空间，这里一个磁盘块中最多能容纳用户数为18个
	struct pwdi passwd[BLOCKSIZ / (PWDSIZ * 2 + 2 + 2)] = { 0 }; //BLOCKSIZ/(PWDSIZ*2+ 4)=512/28

	//生成初始超级块，管理空闲磁盘块，和空闲i节点
	struct filsysi filsys; //超级块
	//开辟了546个物理块指针，每个物理块512字节
	char* buf;			   //磁盘文件卷

	/*	creat the file system file */
	/*“r+”以可读写方式打开文件，该文件必须存在。
	 *“w+”打开可读写文件，若文件存在则文件长度清为零，即该文件内容会消失。若文件不存在则建立该文件
	 *“b”	应该是二进制
	 * "wb" 二进制写，并不能读
	*/
	fd = fopen("filesystem", "wb"); //模拟磁盘
	//32个物理块，每个物理块512字节，每个i节点32字节，共有32*16=512个i节点
	//512个物理块，存放目录下的fcb数组和文件中的内容
	//2个物理块，存放超级块
	buf = (char*)malloc((DINODEBLK + FILEBLK + 2) * BLOCKSIZ * sizeof(char));		//初始化啊啊啊啊啊

	if (buf == NULL)
	{
		printf(">file system file creat failed! \n");
		exit(0);
	}


	fseek(fd, 0, SEEK_SET);
	//将buf初始化内容全部写入文件，此时全部都是0   这里大小是（32+2+512   ）/2   单位k
	fwrite(buf, (DINODEBLK + FILEBLK + 2) * BLOCKSIZ * sizeof(char), 1, fd); //将磁盘内容读入
	fclose(fd);
	//重新打开磁盘 r+b 读写
	fd = fopen("filesystem", "r+b");

	/*-1.initialize the inode area */
	int i, j;
	//一个磁盘块划分16个i节点
	static struct dinode block_buff[BLOCKSIZ / DINODESIZ];
	//遍历32个i节点物理块
	for (i = 0; i < DINODEBLK; i++)
	{
		fseek(fd, DINODESTART + i * BLOCKSIZ, SEEK_SET);
		//将文件中的内容，读入i节点中，此时都是0
		fread(block_buff, 1, BLOCKSIZ, fd);
		//对一个磁盘上的所有i节点权限赋值
		for (j = 0; j < BLOCKSIZ / DINODESIZ; j++)
			block_buff[j].di_mode = DIEMPTY;
		fseek(fd, DINODESTART + i * BLOCKSIZ, SEEK_SET);
		fwrite(block_buff, BLOCKSIZ, 1, fd);
	}

	/*0.initialize the passwd */
	passwd[0].p_uid = ROOT;//用户id是1
	passwd[0].p_gid = ROOT;//用户组id也是1
	strcpy(passwd[0].username, "root");//用户名是root
	strcpy(passwd[0].password, "root");//用户密码也是root

	/*	1.creat the main directory and its sub dir etc and the file password */
	/* 0 empty dinode id */
	//将0号磁盘i节点读入，生成内存i节点
	//其中addr的计算公式是：addr = DINODESTART + dinodeid * DINODESIZ;	
	//所以0号i节点在系统启动时就被占用，类型设置为空
	inode = iget(0);
	inode->di_mode = DIEMPTY;
	iput(inode);

	/* 1 main dir id */
	inode = iget(1);
	inode->di_number = 1;
	inode->di_mode = ROOTMODE | DIDIR; //1号i节点对应的磁盘块是ROOT权限和(主)目录
	inode->di_size = 3 * (DIRSIZ + 2);//包含了.././etc三个fcb
	inode->di_addr[0] = 0; //0号物理块用来存储主目录的fcb，多了可以再拓展
	//主目录的fcb初始化
	strcpy(dir_buf[0].d_name, "..");
	dir_buf[0].d_ino = 1;
	strcpy(dir_buf[1].d_name, ".");
	dir_buf[1].d_ino = 1;
	strcpy(dir_buf[2].d_name, "etc");
	dir_buf[2].d_ino = 2;
	//第一块磁盘存放了三个fcb:.././etc
	fseek(fd, DATASTART, SEEK_SET);
	fwrite(dir_buf, 3 * (DIRSIZ + 2), 1, fd);
	iput(inode);

	/* 2 etc dir id */
	inode = iget(2);
	inode->di_number = 1;
	inode->di_mode = ROOTMODE | DIDIR; //2号i节点对应的磁盘块是默认权限和目录
	inode->di_size = 3 * (DIRSIZ + 2);
	inode->di_addr[0] = 1;//1号物理块存放从目录的fcb
	strcpy(dir_buf[0].d_name, "..");
	dir_buf[0].d_ino = 1;
	strcpy(dir_buf[1].d_name, ".");
	dir_buf[1].d_ino = 2;
	strcpy(dir_buf[2].d_name, "password");
	dir_buf[2].d_ino = 3;
	//第二块磁盘也放了三个fcb
	fseek(fd, DATASTART + BLOCKSIZ * 1, SEEK_SET);
	fwrite(dir_buf, 3 * (DIRSIZ + 2), 1, fd);
	iput(inode);

	/* 3 password id */
	inode = iget(3);
	inode->di_number = 1;
	inode->di_mode = ROOTMODE | DIFILE;				//3号i节点对应得磁盘块是默认权限和文件
	//18*（24+4）=504，一个磁盘占有504个字节
	//uid+gid+username+password
	inode->di_size = PWDNUM * (2 * PWDSIZ + 2 + 2); //暂且认为格式化时所有的密码都被清空重置
	inode->di_addr[0] = 2;//2号物理块存放从从目录password的fcb

	//用户口令字信息初始化
	//当前只用一个磁盘存储密码等信息，所以这里最多只有18个用户
	for (int i = 1; i < BLOCKSIZ / (PWDSIZ * 2 + 2 + 2); i++) //第0号(1个)密码是root保留
	{
		//没有被使用的都是-1
		passwd[i].p_uid = -1;
		passwd[i].p_gid = -1;
		strcpy(passwd[i].password, " ");
		strcpy(passwd[i].username, " ");
	}
	fseek(fd, DATASTART + BLOCKSIZ * 2, SEEK_SET);
	fwrite(passwd, BLOCKSIZ, 1, fd);
	iput(inode);

	/******************************************* 2. 初始化超级块 ********************************************/
	//512个i节点
	filsys.s_isize = DINODEBLK;								  //i节点块总块数		s_pfree	52428	unsigned short
	//512个物理块存文件
	filsys.s_fsize = FILEBLK;								  //数据块总块数
	//i节点已经用去了0，1（root），2（etc），3（password）
	//512-4个可用
	filsys.s_ninode = (DINODEBLK * BLOCKSIZ) / DINODESIZ - 4;
	//空闲块块数 已使用三块磁盘 
	//存储物理块已经用去三块：0号用于根目录 1号用于etc 2号用于password，
	//还剩512-3块
	filsys.s_nfree = FILEBLK - 3;

	/**********	2.1 初始化空闲i节点栈 **********/
	//铭记i节点法
	//i节点个数最多为512个
	//这里只进行i节点的逻辑序号变更，不涉及到物理位置
	for (int i = 0; i < NICINOD; i++)
	{
		//从4开始，到53号磁盘被送入空闲i节点栈
		//0，1，2，3已经被使用，其中1，2，3每个i节点占用一个磁盘，一个磁盘可容纳32个fcb
		filsys.s_inode[i] = 4 + i;
	}
	//空闲i节点指针 从低到高序号分配
	//一开始指向4
	filsys.s_pinode = 0;
	//记录下一个组长节点信息
	//铭记i节点号  空闲i节点栈中指向的空闲i节点分配完后 从此处开始的50个i节点被重新加入空闲栈
	//这里使用了简单的线性存储i节点栈
	//铭记i节点记录54的位置
	filsys.s_rinode = NICINOD + 4;

	/********** 2.2 初始化空闲磁盘组 **********/
	//成组链接法
	unsigned int block_buf[BLOCKSIZ / sizeof(int)]; //组长块缓冲  512/4=128个元素 但只使用了51个元素 占128*4=512个字节
	//初始化第一个组长块，即第二组的组长块
	//0的位置记录本组的空闲块数，即第二组空闲块数为50
	//第一组空闲块数一定是49
	block_buf[0] = NICFREE;		//记录本组空闲块数，第二组是50个
	block_buf[1] = 0;			//结束标志，说明当前是第二组
	int freeblk_ID = 511;		//空闲块块号，这里反过来进行赋值，文件和fcb数组占用512个物理块

	for (int i = 2; i <= NICFREE; i++)
	{
		block_buf[i] = freeblk_ID--;//这里从511记录到463，一共49个，作为第一组的空闲块
	}

	//定位到462的位置，将前一组信息（49个空闲块）存入第462块（第二组组长块中）
	fseek(fd, DATASTART + BLOCKSIZ * freeblk_ID, SEEK_SET);
	fwrite(block_buf, BLOCKSIZ, 1, fd);

	//初始化后续的组长块，后续组长块每次都减去50，每次保证满足50个
	//i初始等于462，每次减50
	for (int i = freeblk_ID; i > 13; i -= NICFREE)
	{
		block_buf[0] = NICFREE;		 //第三组空闲块数
		block_buf[1] = freeblk_ID--; //462块 412 362 312 。。。。。12
		for (int j = 2; j <= NICFREE; j++)
		{
			block_buf[j] = freeblk_ID--;//储存剩余的49个空闲块
		}
		//将前一组（50个空闲块）存入下一组组长块中
		fseek(fd, DATASTART + BLOCKSIZ * freeblk_ID, SEEK_SET);
		fwrite(block_buf, BLOCKSIZ, 1, fd);
	}
	//退出时freeblk_ID=12
	//此时超级块中只需要存储3-12一共10个块内容就行

	//不足50个的磁盘块写入超级块空闲块栈
	int s_pfreetemp = 0;
	for (int i = 1; freeblk_ID > 2; i++)
	{
		filsys.s_free[i] = freeblk_ID--; //写入空闲块块号
		s_pfreetemp = i;
	}
	//超级块0号元素存储空闲块个数，这里是10
	filsys.s_free[0] = s_pfreetemp;
	//这里s_pfree指向10，也就是磁盘号为3的空闲磁盘
	filsys.s_pfree = s_pfreetemp;
	//将空闲磁盘的超级块信息写入第二块磁盘块的位置
	fseek(fd, BLOCKSIZ, SEEK_SET);
	fwrite(&filsys, sizeof(struct filsysi), 1, fd);

	fclose(fd);
}