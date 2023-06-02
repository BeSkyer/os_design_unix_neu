/**

*			�ļ�˵�����ر�Ŀ¼�����ļ�
*/
#include <stdio.h>
#include "filesys.h"

/*****************************************
*	��������close
*	����ֵ����
*	���ܣ��޸��û����ļ����޸�ϵͳ���ļ���
*	�βΣ��û�id�����û����ļ����е�λ��
******************************************/
//�ر��ļ�����Ŀ¼�����ĵ���
close(unsigned int user_id, short cfd)
{
	//��ʱ�ļ��Ѷ����ڴ棬�����ڴ�i�ڵ�
	struct inode* inode;
	//user[user_id]�ҵ����û�
	//u_ofile[cfd]�ҵ��û����ļ���ı�����ָ��ϵͳ�ļ���������ݣ�Ҳ����ָ��
	//sys_ofile[user[user_id].u_ofile[cfd]]��ȡϵͳ�ļ�������
	//sys_ofile[user[user_id].u_ofile[cfd]].f_inode�ҵ��ڴ�i�ڵ�
	inode = sys_ofile[user[user_id].u_ofile[cfd]].f_inode;
	//�ͷŸ��û����ڴ�i�ڵ�
	if (inode != NULL)
		iput(inode);
	//��ϵͳ�ļ����У��ø��ļ����ô�����һ
	sys_ofile[user[user_id].u_ofile[cfd]].f_count--;
	//�û��ܴ򿪵�������+1
	user[user_id].u_ofile[cfd] = SYSOPENFILE + 1;
}
