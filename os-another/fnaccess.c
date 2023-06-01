/**

*			�޸ģ� 1.�ж��߼�
*/

#include <stdio.h>
#include "filesys.h"

/********************************
*	��������xfa()
*	����ֵ��-1����ĳһ��
*	���ܣ��ж��Ƿ����û����ļ����У������򷵻�����һ��
*	�βΣ��ļ���
**********************************/
int xfa(char* filename)
{
	int i, j, ino;
	int flag = 1;
	//�ڵ�ǰĿ¼�в���
	ino = namei(filename);
	//�Ƿ���ڸ��ļ�
	if (ino == NULL)
		flag = 0;
	//�����û����ļ���
	for (i = 0; i < NOFILE; i++)
	{
		//user[user_id].u_ofile[cfd] = SYSOPENFILE + 1;			//��ʶ����
		//��ǰ������У���������
		if (user[user_id].u_ofile[i] == SYSOPENFILE + 1) {}
		//��������У����ҵ��ڸ��ļ���i�ڵ���̺ţ����˳�
		else
		{
			int t = user[user_id].u_ofile[i];
			//tΪ��ǰ�û����û����ļ����е�ĳһ�ֵΪ��ϵͳ���ļ����е�λ��
			//sys_ofile[t].f_inode->i_inoΪϵͳ�򿪱�ĳһ�����Ӧ��i�ڵ��i�ڵ��
			//������ȣ�˵�����ļ����ڵ�ǰĿ¼�����б���
			if (sys_ofile[t].f_inode->i_ino == ino)
			{
				break;
			}
		}

	}
	if (i == NOFILE)
		flag = 0;
	if (flag)//Ҫ�رյ��ļ��Ѿ����򿪣����Խ�һ������
		return i;//�������û��ļ��򿪱��е�λ��
	return -1;
}