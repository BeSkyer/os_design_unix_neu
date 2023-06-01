/**

*/
#include <stdio.h>
#include "filesys.h"
halt()
{
	//�ڴ�i�ڵ�
	struct inode* inode;
	int i, j;
	/**************step1: ��������û�����ؼ�¼*************************/
	for (i = 0; i < USERNUM; i++)   //�ر����д��ļ�
	{
		if (user[i].u_uid != 0)//���û�������û�
		{
			for (j = 0; j < NOFILE; j++)
			{
				if (user[i].u_ofile[j] != SYSOPENFILE + 1)//Ĭ��41�ǹر����û���ǰ���ļ�
				{
					//��ն�Ӧ��ϵͳ�򿪱����
					close(user[i].u_uid, user[i].u_ofile[j]);
					//�ر��û��򿪱��е��ļ�
					user[i].u_ofile[j] = SYSOPENFILE + 1;
				}
			}
		}
	}
	/**************step2: д�ش��̿鲢�ر��ļ�ϵͳ************************/
	//�������������д�ش��̣���������泬��������һ��
	fseek(fd, BLOCKSIZ, SEEK_SET);
	fwrite(&filsys, sizeof(struct filsysi), 1, fd);  //д�ش��̿�
	fclose(fd);
	printf(">Good Bye��See You Next Time.\n");
	exit(0);
}