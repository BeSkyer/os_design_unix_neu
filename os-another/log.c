/**

*		    �޸�:	1.login�����޸�
*						1.1 ������(unsigned short uid, char	*passwd)��Ϊ(char* username, char	*passwd)
*						1.2 �����߼��޸���д
*						1.3 ע�����
*						1.4 ��½���û���Ȩ��ΪUSERMODE,root�û���ΪROOTMODE
*					2.logout�����޸�
*						2.1 �����߼��޸�
*						2.2 ע�����
						2.3 ���ע������ת�ظ�Ŀ¼�Ĺ����������ǳ����صǵĴ���
*/
#include <stdio.h>
#include "filesys.h"

/***************************************
��½������ʵ���û����������ƥ��Ͳ�ѯ
username: ��ƥ���û���
passwd: ��ƥ������
����ֵ�� ��ǰ�û���id
****************************************/
int login(char* username, char* passwd)
{
	int i, j;
	int match_flag = 0, ret = 0;
	unsigned short tempuid = 0, tempgid = 0;
	for (i = 0; i < PWDNUM; i++)
	{
		if (strcmp(username, pwd[i].username) == 0) break;//�û���ƥ���˳�ѭ��
	}
	if (i == PWDNUM)
	{//�û�����ƥ��
		printf(">Incorrect username. please retry\n");
		return -1;
	}
	//�û���ƥ��
	for (i = 0; i < PWDNUM; i++)
	{
		if (strcmp(passwd, pwd[i].password) == 0)
		{//����ƥ��
			match_flag = 1;
			tempuid = pwd[i].p_uid;
			tempgid = pwd[i].p_gid;
			//�����û����û����е�λ��
			ret = i;
			break;
		}
	}
	if (i == PWDNUM)
	{//���벻ƥ��,���Ѿ��������β����
		printf(">Incorrect password. please retry\n");
		return -1;
	}
	//�������û�����������Ѿ�ƥ��
	for (j = 0; j < USERNUM; j++)
	{
		//���������û�����
		if (user[j].u_uid == -1)
		{
			//�ҵ����е��û�������û�ID����ID���û������и�ֵ
			//�������Ƿ���root�����費ͬ��Ȩ��
			user[j].u_uid = tempuid;
			user[j].u_gid = tempgid;
			if (i == 0)user[j].u_default_mode = ROOTMODE;
			else user[j].u_default_mode = ROOTMODE;//USERMODE;
			/*else user[j].u_default_mode = USERMODE;*/
			break;
		}
	}
	if (j == USERNUM)
	{
		//���������û������һ����Ȼû���ҵ������û�����
		printf(">Too many users. Please wait\n");
		return -1;
	}
	return ret;
}

/***************************************
ע������,ɾ���û�����ϵͳ�ļ��򿪱��е�
��Ӧ��
uid: Ҫɾ�����û�id
****************************************/
int logout(unsigned short uid)
{
	int i, j, sys_no;
	struct inode* inode;
	for (i = 0; i < USERNUM; i++)
		if (uid == user[i].u_uid) break;//�û�idƥ��
	if (i == USERNUM)//û���ҵ���Ӧ���û�
	{
		printf(">Cannot find the user you want to log out\n");
		return NULL;
	}
	/************************�ж�Ӧ���û�����ն�Ӧ���û�����*****************************/
	for (j = 0; j < NOFILE; j++)							//����û��򿪱�
	{
		if (user[i].u_ofile[j] != SYSOPENFILE + 1)
		{
			/* iput the mode free the sys_ofile and clear the user-ofile */
			sys_no = user[i].u_ofile[j];				//��ȡ��ϵͳ���ļ�����λ��
			inode = sys_ofile[sys_no].f_inode;		//��ȡ��Ӧ��i�ڵ�
			iput(inode);							//�ͷŶ�Ӧ���ڴ�i�ڵ�
			sys_ofile[sys_no].f_count--;			//��Ӧ���ļ����ü���-1
			user[i].u_ofile[j] = SYSOPENFILE + 1;	//�û��ļ����Ӧ������Ϊinstall���ֵ
		}
	}
	user[i].u_default_mode = -1;					//��ն�Ӧ�û����������
	user[i].u_gid = -1;								//��ն�Ӧ�û�������û�id
	user[i].u_uid = -1;								//��ն�Ӧ�û�������û���id
	user_id = -1;									//����ǰ�û�id��գ�׼���´ε�¼
	while (cur_path_inode->i_ino != 1)
		chdir(ROOT, "..");							//ѭ����ת����Ŀ¼
	//��λ���ڶ������
	fseek(fd, DATASTART + BLOCKSIZ * 2, SEEK_SET);
	//�����û���Ϣ������Ĭ��ֻ���˵ڶ�����̴洢�û���Ϣ
	//ע�������һ�ж�û��������pwd�������û���û����ʧ��ֻ���û��򿪱���ʧ��
	fwrite(pwd, BLOCKSIZ, 1, fd);
	return 1;
}
