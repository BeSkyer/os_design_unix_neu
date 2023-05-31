/*rdwt.c*/

#/*初始化磁盘格式化程序format.c*/


//修改
// -> 一个这种符号
#include <stdio.h>
#include "filesys.h"
#include <stdlib.h>
#include <string.h>
format()
{
	struct inode * inode;
	struct direct dir_buf [BLOCKSIZ / (DIRSIZ+2)];
	struct pwdi passwd[BLOCKSIZ/(PWDSIZ+ 4)];

	struct filsysi filsys;
	unsigned int block_buf[BLOCKSIZ / sizeof (int)];
	char * buf;
	int i, j;
/*	creat the file system file */
	fd=fopen ("filesystem", "r+w+b");
	buf=(char * ) malloc ((DINODEBLK+FILEBLK+2) * BLOCKSIZ * sizeof(char));
	if (buf==NULL)
{
printf ("\nfile system file creat failed! \n");
exit(0);
}
fseek(fd,0, SEEK_SET);
fwrite(buf,1, (DINODEBLK+FILEBLK+2) * BLOCKSIZ * sizeof(char) ,fd);
/*0.initialize the passwd */
passwd[0].p_uid= 2116; passwd[0].p_gid= 03;
strcpy(passwd[0].password, "dddd");
passwd[1].p_uid= 2117; passwd[1].p_gid=03;
strcpy(passwd[1].password, "bbbb");
passwd[2].p_uid= 2118; passwd[2].p_gid=04;
strcpy(passwd[2].password, "abcd");
passwd[3].p_uid= 2119; passwd[3].p_gid=04;
strcpy(passwd[3].password, "cccc");
passwd[4].p_uid= 2220; passwd[4].p_gid=05;
strcpy(passwd[4].password, "eeee");
/*	1.creat the main directory and its sub dir etc and the file password */
inode=iget(0);	/* 0 empty dinode id */
inode->di_mode	=DIEMPTY;
iput(inode);
inode=iget(1);    /* 1 main dir id */
inode->di_number= 1;
inode->di_mode = DEFAULTMODE|DIDIR;
inode->di_size=3 *(DIRSIZ+2);
inode->di_addr[0]=0;    /* block 0tfl is used by the main directory */
strcpy(dir_buf[0].d_name, "..");
dir_buf[0].d_ino= 1;
strcpy(dir_buf[1].d_name,".");
dir_buf[1].d_ino= 1;
strcpy(dir_buf[2].d_name, "etc");
dir_buf[2].d_ino=2;
fseek(fd, DATASTART, SEEK_SET);
fwrite(dir_buf, 1, 3 * (DIRSIZ+2), fd);
iput(inode);
inode=iget(2);/* 2 etc dir id */
inode ->di_number =1;
inode->di_mode=DEFAULTMODE|DIDIR;
inode->di_size=3 * (DIRSIZ+2);
inode->di_addr[0]=0;    /* block 0# is used by the etc */
strcpy (dir_buf[0].d_name, "..");
dir_buf[0].d_ino=1;
strcpy(dir_buf[1].d_name, "..");
dir_buf[1].d_ino=2;
strcpy(dir_buf[2].d_name, "password");
dir_buf[2].d_ino=3;
fseek(fd, DATASTART+BLOCKSIZ * 1, SEEK_SET);
fwrite (dir_buf, 1,3* (DIRSIZ+2),fd);
iput(inode);
inode=iget(3);    /* 3 password id */
inode->di_number= 1;
inode->di_mode=DEFAULTMODE |  DIFILE;
inode->di_size=BLOCKSIZ;
inode->di_addr[0]=2;
for (i=5; i<PWDNUM; i++)
{	passwd[i].p_uid=0;
	passwd[i].p_gid=0;
	strcpy(passwd[i].password,"	");
}
fseek(fd,DATASTART+2*BLOCKSIZ, SEEK_SET);
fwrite(passwd, 1, BLOCKSIZ,fd);
iput(inode);
/*	2. initialize the superblock */
filsys.s_isize=DINODEBLK;
filsys.s_fsize=FILEBLK;
filsys.s_ninode=DINODEBLK * BLOCKSIZ/DINODESIZ-4;
filsys.s_nfree = FILEBLK-3;
for (i=0; i<NICINOD; i++)
{
/*	begin with 4, 0,1.2,3, is used by main, etc, password */
filsys.s_inode[i]=4+i;
}
filsys.s_pinode=0;
filsys.s_rinode=NICINOD+4;
/*FILEBLK+1 is a flag of end */
block_buf[NICFREE- 1]=FILEBLK+1;
for (i=0;i<NICFREE-1;i++)
	block_buf[NICFREE- 2-i]=FILEBLK -i;
fseek(fd, DATASTART+BLOCKSIZ * (FILEBLK-NICFREE- 1), SEEK_SET);
fwrite (block_buf, 1, BLOCKSIZ, fd);
for	(i= FILEBLK-NICFREE - 1; i>2; i-= NICFREE)
{
for (j=0;j<NICFREE;j++)
	{
block_buf[j]=i-j;
}
fseek(fd ,DATASTART+BLOCKSIZ * (i-1) , SEEK_SET);
fwrite(block_buf, 1, BLOCKSIZ,fd);
}
j=1;
for (i=i; i>2;i--)
{
filsys.s_free[NICFREE+i-j] =i;
}
filsys.s_pfree=NICFREE -j;
filsys.s_pinode=0;
fseek(fd, BLOCKSIZ, SEEK_SET);
fwrite (&filsys,1,sizeof(struct filsysi),fd);
}
#include <stdio.h>
#include "filesys.h"

unsigned int read(fd, buf, size)
int fd;
char * buf;
unsigned int size;
{
	unsigned long off;
	int block, block_off, i, j;
	struct inode * inode;
	char * temp_buf;

	inode=sys_ofile[user[user_id].u_ofile[fd]].f_inode;
	if (!(sys_ofile[user[user_id].u_ofile[fd]].f_flag & FREAD))
	{
		printf ("\nthe file is not opened for read\n");
		return 0;
	}
	temp_buf=buf;
	off = sys_ofile[user[user_id].u_ofile[fd]].f_off;
	if ((off+size)>inode->di_size) size=inode->di_size-off;
	block_off=off % BLOCKSIZ;
	block =off/BLOCKSIZ;
	if (block_off+size<BLOCKSIZ)
	{
		fseek(fd, DATASTART + inode -> di_addr[block] * BLOCKSIZ + block_off, SEEK_SET);
		fread(buf,1, size, fd);
		return size;
	}
	fseek(fd,DATASTART+inode->di_addr[block]*BLOCKSIZ+block_off,SEEK_SET);
	fread(temp_buf,1,BLOCKSIZ-block_off,fd);
	temp_buf+=BLOCKSIZ-block_off;
	j= (inode->di_size-off-block_off)/BLOCKSIZ;
	for(i=0; i<(size-block_off) /BLOCKSIZ; i++)
	{
		fseek (fd, DATASTART+inode->di_addr[j+i]*BLOCKSIZ,SEEK_SET);
		fread(temp_buf,1,BLOCKSIZ,fd);
		temp_buf+=BLOCKSIZ;
	}

	block_off=(size-block_off)%BLOCKSIZ;
	block=inode->di_addr[off+size/BLOCKSIZ+ 1];
	fseek(fd,DATASTART+block*BLOCKSIZ,SEEK_SET);
	fread(temp_buf,1,block_off,fd);
	sys_ofile[user[user_id].u_ofile[fd]].f_off+=size;
	return size;

}

unsigned int write (fd,buf,size)
int fd;
char * buf;
unsigned int size;
{
	unsigned long off;
	int block,block_off,i,j;
	struct inode * inode;
	char * temp_buf;
	inode=sys_ofile[user[user_id].u_ofile[fd]].f_inode;
	if (!(sys_ofile[user[user_id].u_ofile[fd]].f_flag & FWRITE))
	{
		printf("\n the file is not opened for write\n");
		return 0;
	}
	temp_buf=buf;

	off=sys_ofile[user[user_id].u_ofile[fd]].f_off;
	block_off= off % BLOCKSIZ;
	block =off/BLOCKSIZ;

	if (block_off+size<BLOCKSIZ)
	{
		fseek (fd, DATASTART + inode ->di_addr[block] * BLOCKSIZ + block_off, SEEK_SET);
		fwrite(buf, 1, size, fd);
		return size;
	}

	fseek (fd, DATASTART+inode ->di_addr[block] * BLOCKSIZ+block_off, SEEK_SET);
	fwrite(temp_buf, 1, BLOCKSIZ-block_off, fd);
	temp_buf += BLOCKSIZ - block_off;
	for (i=0; i<(size-block_off)/BLOCKSIZ;i++)
	{
		inode->di_addr[block+1+i]=balloc( );
		fseek(fd ,DATASTART+inode ->di_addr[block + 1 +i] * BLOCKSIZ, SEEK_SET);
		fwrite(temp_buf, 1, BLOCKSIZ, fd);
		temp_buf += BLOCKSIZ;
	}
	block_off = (size-block_off) % BLOCKSIZ;
	block = inode ->di_addr[off+size/BLOCKSIZ + 1];
	fseek (fd ,DATASTART+block * BLOCKSIZ ,SEEK_SET);
	fwrite(temp_buf,1 ,block_off, fd);
	sys_ofile[user[user_id].u_ofile[fd]].f_off += size;
	return size;
}