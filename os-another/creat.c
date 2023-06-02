/**

*            �޸�:	1. �����µ��ļ�ʱ�ļ���ʼ����СΪ1���̿�Ĵ�С
*					2. ԭ����ʼ��ʱ�½����ļ����������ϵͳ���ļ�����û��ļ��򿪱��������ǲ�Ҫ
*                   3. ����Ŀ¼�Ĺ��̷������  ***
*					4. �޸��˶�i�ڵ��addr�ĳ�ʼ�� ***
*					5. ����˺����Ĳ��� size ***
					6. ������iput
*/
#include <stdio.h>
#include "filesys.h"

/***********************************
*	��������creat()
*	����ֵ����
*	���ܣ������ļ�����Ŀ¼
*	�βΣ������ˣ�Ҫ�������ļ������ļ�Ȩ��
***********************************/
unsigned int creat(int user_id, char* filename, unsigned short mode)
{
	unsigned int di_pos, di_ino;
	//�ڴ�i���
	struct inode* inode;
	unsigned int i, j;
	//��ǰĿ¼�Ƿ����ͬ���ļ���Ŀ¼
	di_ino = namei(filename);
	//�����򷵻�
	if (di_ino != NULL)
	{
		printf(">'%s' has already existed as file or subdirectory\n", filename);
		return -1;
	}
	//���򴴽�
	else
	{
		//�ҵ�ǰĿ¼��i����ڴ����е����
		di_ino = namei(".");
		//����ǰĿ¼�����ڴ�
		inode = iget(di_ino);
		//�鿴��ǰĿ¼��Ȩ�ޣ�д��ִ�У������򷵻�
		//���Ƶ�ǰĿ¼����Ȩ�޴���
		if (!access(user_id, inode, WRITE) || !access(user_id, inode, EXICUTE))
		{
			printf(">failed to creat file because of no authority!\n");
			iput(inode);
			return -1;
		}
		//��ѯ����
		iput(inode);
		//�����ڴ�i�ڵ�
		inode = ialloc();
		//�ļ�Ȩ�޺����� Ȩ��ΪROOTMODE(������rwx) GDIREAD��ͬ��ɶ�r--��
		inode->di_mode = mode;
		//�û�id
		inode->di_uid = user[user_id].u_uid;
		//�û���id
		inode->di_gid = user[user_id].u_gid;
		//�ļ���С
		inode->di_size = 0;
		//�ļ���������Ϊ1
		inode->di_number = 1;
		int i;
		//��Ϊû�����ݣ����Ը�i�ڵ㲻ָ����̿�
		for (i = 0; i < NADDR; i++)
		{
			inode->di_addr[i] = -1;
		}
		//��ǰĿ¼�µĿ�Ŀ¼�һ��128��Ŀ¼���
		//�ҵ���ǰĿ¼�µĿ���Ŀ¼��
		di_pos = iname(filename);
		//����ʧ�ܣ�����
		if (di_pos == -1)return;
		//����ɹ���Ŀ¼����+1�����ǲ��ܳ���128
		dir.size++;
		//�Դ���i�ڵ��߼���Ž��и�ֵ
		dir.direct[di_pos].d_ino = inode->i_ino;
		//�޸ĵ�ǰ·��i�ڵ���ļ���С���ļ�������+i�ڵ�����ָ�볤��
		cur_path_inode->di_size += (DIRSIZ + 2);
		iput(inode);
	}
}