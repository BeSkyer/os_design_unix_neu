#include "filesys.h"
#include <stdio.h>

/*****************************************
*	��������cpy()
*	����ֵ����
*	��  �ܣ�����һ�����ݵ�ȫ�ֱ���
*	��  �Σ��ļ�����ȫ�ֱ���
******************************************/
void cpy(char* filename, char* content)//�������ݵ�һ��ȫ�ֱ���
{
	//ճ������
	char cp_buf[512];
	//��Ŀ¼���������ļ��Ĵ���i�ڵ��߼����
	int cpdino = namei(filename);
	//�õ��ļ���
	char* cpfilename = filename;
	//�����ڴ�i�ڵ�ָ��
	struct inode* inode;
	//�Ӵ��̶����ڴ�
	inode = iget(cpdino);
	//���ж��û��ܷ��ȡ���ļ�
	//����access�Ƕ�inode���в����ģ�������Ҫ����һ��ת��
	if (!access(user_id, inode, READ))
	{

		printf("\n�Բ�����û�и��Ƹ��ļ���Ȩ�ޣ�\n");
		iput(inode);
		return;
	}
	//����и�Ȩ�޻����ȴ��ڴ����ͷ�
	iput(inode);
	//�������û����е�λ��
	int tfd = xfa(filename);
	//���ڸ��ļ�
	if (tfd != -1)
	{
		//��ϵͳ�ļ����Ӧ��
		int sys_otpos = user[user_id].u_ofile[tfd];
		//�õ��ļ���С
		int temp_file_size = sys_ofile[sys_otpos].f_inode->di_size;
		//���ļ����ݶ��뻺�����������ļ�Ҫ�Ǵ���512����ô�죿
		//���û����ļ��У���ȡ��Ӧ����
		read(tfd, cp_buf, temp_file_size);
		int i = 0;
		while (((cp_buf[i] >= '0' && cp_buf[i] <= '9') || (cp_buf[i] >= 'a' && cp_buf[i] <= 'z') || (cp_buf[i] >= 'A' && cp_buf[i] <= 'Z')))
			i++;
		cp_buf[i] = '\0';
		//��Ҫ��ֵ���ļ�copy��ȫ��content��
		strcpy(content, cp_buf);
		printf("\n>content you want to copy: %s\n", content);//����ļ�����
	}
	else
	{
		printf(">Cannot read a file that is not open or not exixted in current directory\n");
		return;
	}
	//�޸Ĵ��ļ���
	close(user_id, tfd);
	return;
}

/*****************************************
*	��������pst
*	����ֵ����
*	��  �ܣ�ճ����ָ��λ��
*	��  �Σ��ļ�����ȫ�ֱ���
******************************************/
void pst(char* filename, char* content)
{
	//�ҵ��ļ��Ĵ���i�ڵ��
	int cpdino = namei(filename);
	//�����ڴ���
	struct inode* inode;
	inode = iget(cpdino);
	//�ж�Ȩ��
	if (!access(user_id, inode, READ))
	{

		printf("\n�Բ�����û�и��Ƹ��ļ���Ȩ�ޣ�\n");
		iput(inode);
		return;
	}
	iput(inode);
	//�õ��û��ļ��б�
	int tfd = xfa(filename);
	//�ļ������ұ���
	if (tfd != -1)
	{
		//�ҵ�ϵͳ�¼���
		int sys_otpos = user[user_id].u_ofile[tfd];
		//��ȡ�ļ���С
		int temp_file_size = sys_ofile[sys_otpos].f_inode->di_size;
		int ssize = 0;
		//һֱ���ļ�ĩβ
		while (content[ssize] != '\0')
			ssize++;
		//���û����ļ���׷�Ӷ�Ӧ����
		write(tfd, content, ssize);
	}
	else
		printf(">Cannot write a file that is not open or not exixted in current directory\n");
	//�޸Ĵ��ļ���
	close(user_id, tfd);
	return;
}
