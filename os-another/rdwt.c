#include <stdio.h>
#include "filesys.h"

//cfd���ļ����û����ļ����е�λ��
//buf���ļ����뻺�����������10�������
//size����õ�ǰ�ļ���С
unsigned int read(int cfd, char* buf, unsigned int size)
{
	//ƫ����
	unsigned long off;
	int block, block_off, i, j;
	//�����ڴ�i�ڵ��ָ��
	struct inode* inode;
	//��ʱ������
	char* temp_buf;

	//ͨ���û����ļ���ı���ҵ�ϵͳ�ļ����е��ڴ�i�ڵ�
	inode = sys_ofile[user[user_id].u_ofile[cfd]].f_inode;
	//�ж��ļ��Ƿ��
	if (!(sys_ofile[user[user_id].u_ofile[cfd]].f_flag & FREAD))
	{
		printf(">the file is not opened for read!\n");
		return 0;
	}
	//�ж��Ƿ��в���Ȩ��
	if (!access(user_id, inode, READ))
	{
		printf(">fail to read file because of no authority!\n");
		iput(inode);				//�ͷ�i�ڵ�����
		return NULL;
	}
	//�Ƚ�������������ʼ��
	memset(buf, 0, sizeof(buf));
	for (i = 0, j = 0; i < inode->di_size / BLOCKSIZ + 1; i++, j += BLOCKSIZ)
	{
		//��λ���ļ���ͷ�ĵط�
		fseek(fd, DATASTART + inode->di_addr[i] * BLOCKSIZ, SEEK_SET);
		//����������������buf�У�����һ������Ҳ��������Ϊȫ������ʼ��Ϊ�ַ�0��
		fread(buf + i * BLOCKSIZ, BLOCKSIZ, 1, fd);
	}

}

//cfd���ļ����û����ļ����е�λ��
//buf���ļ����뻺�����������10�������
//size����õ�ǰ�ļ���С
unsigned int write(int cfd, char* buf, unsigned int size)
{
	unsigned long off;
	unsigned int block, block_off, i, j;
	struct inode* inode;
	char* temp_buf;
	//�洢10������飬ÿ���������Ҫд������
	char buf1[10][BLOCKSIZ];
	//�鿴�ļ��Ƿ������д������׷�ӻ�����д
	if (!(sys_ofile[user[user_id].u_ofile[cfd]].f_flag & FWRITE) &&
		!(sys_ofile[user[user_id].u_ofile[cfd]].f_flag & FAPPEND))
	{
		printf(">the file is not opened for write!\n");
		return 0;
	}
	inode = sys_ofile[user[user_id].u_ofile[cfd]].f_inode;

	//temp_buf = buf;				//ָ���ַ�����
	//ȡ��д/׷��ƫ��ָ��
	off = sys_ofile[user[user_id].u_ofile[cfd]].f_off;
	//block_off = off % BLOCKSIZ;	//ȡ�ÿ���ƫ�Ƶ�ַ
	block = size / BLOCKSIZ;		//ȡ��inoden����Կ��
	//��buf�н����ݴ���buf1������
	for (i = 0; i < block + 1; i++) {
		memcpy(buf1[i], buf + i * BLOCKSIZ, BLOCKSIZ);
	}
	for (i = 0; i < block + 1; i++)
	{
		//������ļ�������Ȩ��ʱ������65535�����������룬ֱ����Ϊ65535Ϊֹ
		if (inode->di_addr[i] == 65535)
			inode->di_addr[i] = balloc();
		fseek(fd, DATASTART + inode->di_addr[i] * BLOCKSIZ, SEEK_SET);
		fwrite(buf1[i], BLOCKSIZ, 1, fd);
	}
	//���¶�/дָ��
	sys_ofile[user[user_id].u_ofile[cfd]].f_off += size;
	//�����ļ���С
	inode->di_size += size;
	return size;
}
