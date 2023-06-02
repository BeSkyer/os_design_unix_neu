/* name.c*/
/**

*		�޸ģ�	1. line 21 : return i -> retrun dir.direct[1].d_ino
*				2. line 54 : strcpy(name,dir.direct[i].d_name) -> strcpy(dir.direct[i].d_name, name)
*				3. �޸���namei�е��ж��Լ���iд��1�Ĵ���
*/
#include <string.h>
#include <stdio.h>
#include "filesys.h"
/***********************************
*	��������namei
*	����ֵ�����̽ڵ��
*	���ܣ���Ѱ��ǰĿ¼���ļ��Ĵ����������
*	�βΣ��ļ���
*************************************/
//���ҵ�ǰĿ¼�����޸��ļ�
unsigned int namei(char* name)
{
	int i, notfound = 1;
	for (i = 0; ((i < dir.size) && (notfound)); i++)
	{
		//�Աȵ�ǰĿ¼�е�������Ҫ���ҵ��ļ���������ͬ�Ҷ�Ӧ��i�ڵ�Ų�Ϊ0
		//d_ino==0˵���ǿյ�Ŀ¼��
		if ((strcmp(dir.direct[i].d_name, name) == 0) && (dir.direct[i].d_ino != 0))
			//���ظ��ļ���Ӧi�ڵ��ڴ����е����
			return dir.direct[i].d_ino;
	}
	/* notfind */
	return NULL;
};

	/**************************************
	*	��������iname
	*	����ֵ����Ŀ¼��Ŀ¼���е�λ��
	*	���ܣ���Ѱ��ǰĿ¼�µĿ�Ŀ¼��
	*	�βΣ��ļ���
	****************************************/
	unsigned short iname(char* name)
	{
		int i, notfound = 1;
		//�ڴ���һ���ܷ�128��Ŀ¼��
		for (i = 0; ((i < DIRNUM) && (notfound)); i++)
		{	//���ڿյ�Ŀ¼��
			if (dir.direct[i].d_ino == 0)
			{
				notfound = 0;
				break;
			}
		}
		//�Ҳ����յ�Ŀ¼������
		if (notfound)
		{
			printf(">The current directory is full!\n");
			return -1;
		}
		//�ҵ��յ�Ŀ¼������
		else
		{	/*��Ҫ������Ŀ¼(·����)�ŵ��ҵ��Ŀ���Ŀ¼��������*/
			strcpy(dir.direct[i].d_name, name);
			//���ؿ���Ŀ¼��
			return i;
		}
	}

