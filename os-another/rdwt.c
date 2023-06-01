#include <stdio.h>
#include "filesys.h"

//cfd：文件在用户打开文件表中的位置
//buf：文件读入缓存区，最大有10个物理块
//size：获得当前文件大小
unsigned int read(int cfd, char* buf, unsigned int size)
{
	//偏移量
	unsigned long off;
	int block, block_off, i, j;
	//读入内存i节点的指针
	struct inode* inode;
	//临时缓冲区
	char* temp_buf;

	//通过用户打开文件表的表项，找到系统文件表中的内存i节点
	inode = sys_ofile[user[user_id].u_ofile[cfd]].f_inode;
	//判断文件是否打开
	if (!(sys_ofile[user[user_id].u_ofile[cfd]].f_flag & FREAD))
	{
		printf(">the file is not opened for read!\n");
		return 0;
	}
	//判断是否有操作权限
	if (!access(user_id, inode, READ))
	{
		printf(">fail to read file because of no authority!\n");
		iput(inode);				//释放i节点内容
		return NULL;
	}
	//先将读出缓存区初始化
	memset(buf, 0, sizeof(buf));
	for (i = 0, j = 0; i < inode->di_size / BLOCKSIZ + 1; i++, j += BLOCKSIZ)
	{
		//定位到文件开头的地方
		fseek(fd, DATASTART + inode->di_addr[i] * BLOCKSIZ, SEEK_SET);
		//读出到读出缓冲区buf中，不足一块内容也读出，因为全部都初始化为字符0了
		fread(buf + i * BLOCKSIZ, BLOCKSIZ, 1, fd);
	}

}

//cfd：文件在用户打开文件表中的位置
//buf：文件读入缓存区，最大有10个物理块
//size：获得当前文件大小
unsigned int write(int cfd, char* buf, unsigned int size)
{
	unsigned long off;
	unsigned int block, block_off, i, j;
	struct inode* inode;
	char* temp_buf;
	//存储10个物理块，每个物理块中要写的内容
	char buf1[10][BLOCKSIZ];
	//查看文件是否打开用作写操作，追加或者重写
	if (!(sys_ofile[user[user_id].u_ofile[cfd]].f_flag & FWRITE) &&
		!(sys_ofile[user[user_id].u_ofile[cfd]].f_flag & FAPPEND))
	{
		printf(">the file is not opened for write!\n");
		return 0;
	}
	inode = sys_ofile[user[user_id].u_ofile[cfd]].f_inode;

	//temp_buf = buf;				//指向字符串首
	//取得写/追加偏移指针
	off = sys_ofile[user[user_id].u_ofile[cfd]].f_off;
	//block_off = off % BLOCKSIZ;	//取得块内偏移地址
	block = size / BLOCKSIZ;		//取得inoden内相对块号
	//从buf中将内容存入buf1数组里
	for (i = 0; i < block + 1; i++) {
		memcpy(buf1[i], buf + i * BLOCKSIZ, BLOCKSIZ);
	}
	for (i = 0; i < block + 1; i++)
	{
		//如果该文件在申请权限时出现了65535，则重新申请，直到不为65535为止
		if (inode->di_addr[i] == 65535)
			inode->di_addr[i] = balloc();
		fseek(fd, DATASTART + inode->di_addr[i] * BLOCKSIZ, SEEK_SET);
		fwrite(buf1[i], BLOCKSIZ, 1, fd);
	}
	//更新读/写指针
	sys_ofile[user[user_id].u_ofile[cfd]].f_off += size;
	//更新文件大小
	inode->di_size += size;
	return size;
}
