/**

*		    �޸�:	1. �޸��ļ��Ƿ���ڵ��жϴ���
*					2. ���ע��
*					3. �����û����ļ������ʱ�ıȽ�ʽ��Ϊif (user[user_id].u_ofile[j] == SYSOPENFILE + 1) break;
*					4. �޸ļ�¼��ϵͳ���ļ����е�λ��Ϊ��user[user_id].u_ofile[j]=i;
*					5. ����������ļ�β�����Ԫ�أ����ͷ��ļ�֮�ɵĴ��̿�
*					6. ����ļ��Ѿ��򿪵��ж�  ***
*/
#include <stdio.h>
#include "filesys.h"
/******************************
���ڴ򿪻򴴽��ļ����ڴ򿪻򴴽��ļ�ʱ����ָ���ļ������Լ��û���Ȩ�޵ȸ��ֲ�����
user_id:�û�id
filename: �ļ���
openmode: ��ʲô��ʽ��
FREAD FWRITE FAPPEND
*******************************/
unsigned short aopen(int user_id, char* filename, unsigned short openmode)
{
	unsigned int dinodeid;
	struct inode* inode;
	int i, j;
	dinodeid = namei(filename);//�ڵ�ǰĿ¼��Ѱ�Ҷ�Ӧ��i�ڵ�
	if (dinodeid == NULL)    /* nosuchfile */
	{
		//�ļ��ڵ�ǰĿ¼�в�����
		printf(">file does not existed!\n");
		return NULL;
	}
	//��������ؽ��ڴ�
	inode = iget(dinodeid);
	//�ж���ǰ�û�id���û����е�λ���Ƿ���Ȩ�޴��ļ�
	//����Ȩ�޲���
	if (!access(user_id, inode, openmode))    /* access denied */
	{
		printf(">Failed to open file due to unqualified authority!\n");
		iput(inode);
		return NULL;
	}
	/**********����ϵͳ���ļ�����*********/
	for (i = 0; i < SYSOPENFILE; i++)
	{
		//�ҵ��ձ����˳�ѭ��
		if (sys_ofile[i].f_count == 0) break;
		//�Ѿ���
		if (sys_ofile[i].f_inode->i_ino == inode->i_ino)
		{
			printf(">File is open already!\n");
			return NULL;
		}
	}

	if (i == SYSOPENFILE)
	{//û�ҵ��ձ������
		printf(">Too much file open!\n");
		iput(inode);		//�ͷ�i�ڵ�
		return NULL;
	}
	//�ҵ��ձ���
	sys_ofile[i].f_inode = inode;				//��¼i�ڵ�
	sys_ofile[i].f_flag = openmode;			//��¼����
	sys_ofile[i].f_count = 1;					//���ü���+1
	if (openmode & FAPPEND)					//���ļ�β�����Ԫ��
		sys_ofile[i].f_off = inode->di_size;	//����дָ�������ļ�β�ˣ���ָ������ƶ�������д��׼��
	else
		sys_ofile[i].f_off = 0;				//�������дָ�������ļ�ͷ
	/**********�����û����ļ�����*********/
	for (j = 0; j < NOFILE; j++)									//�����û��ļ��򿪱�
		if (user[user_id].u_ofile[j] == SYSOPENFILE + 1) break;		//�ҵ��յı����˳�ѭ��
	if (j == NOFILE)
	{//û�ҵ��յı������
		printf(">Too much file opened by the user! \n");
		sys_ofile[i].f_count = 0;									//��Ӧ��ϵͳ�򿪱����ü�������
		iput(inode);											//�ͷ�i�ڵ�
		return NULL;
	}
	//�ҵ��յı���
	user[user_id].u_ofile[j] = i;									//��¼��ϵͳ�򿪱��е�λ��

	//��дʱ��Ҫ���ɵ��ļ����̿��ͷ�
	if ((openmode & FWRITE))
	{
		//���ļ��ɵĴ��̿��ͷţ�����һ�����̵�����Ҳȫ���ͷ�
		//���ļ���˵���̿��ǻ�����λ�������������ļ�ͬռ��һ�����̿�
		for (i = 0; i < inode->di_size / BLOCKSIZ + (inode->di_size % BLOCKSIZ != 0); i++)
			bfree(inode->di_addr[i]);
		//����i�ڵ��С���
		inode->di_size = 0;
		return j;
	}
	if ((openmode & FREAD))
	{
		//���ļ��ɵĴ��̿��ͷţ�����һ�����̵�����Ҳȫ���ͷ�
		//���ļ���˵���̿��ǻ�����λ�������������ļ�ͬռ��һ�����̿�
		for (i = 0; i < inode->di_size / BLOCKSIZ + (inode->di_size % BLOCKSIZ != 0); i++)
			bfree(inode->di_addr[i]);
		return j;
	}
	//�����ļ����û��򿪱��е�λ��

}