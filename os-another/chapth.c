/**

*			�ļ�˵������ǰ·������ʾ
*/
#include <stdio.h>
#include "filesys.h"

/******************************************************************
*	��������chpath
*	����ֵ����
*	���ܣ��ı���Ļ��precord�����
*	�βΣ���ǰprecord�������Ҫ�ı��Ŀ¼cname����ǰ·��i�ڵ���߼����
*******************************************************************/

void chpath(char* precord, char* cname, int lastInodeId)
{
	int i = 0;
	//ֱ�ӷ���
	if (strcmp(cname, ".") == 0)
		return;
	//�����ϲ�
	else if (strcmp(cname, "..") == 0)
	{
		//ָ���Ƶ�ĩβ
		while (precord[i++] != '\0');
		i--;
		//�жϵ�ǰ�Ƿ���root�㣬����ֱ�ӷ���
		if (precord[i - 1] == 't' && precord[i - 2] == 'o' && precord[i - 3] == 'o' && precord[i - 4] == 'r' && lastInodeId == 1)
			return;
		//����Ըò��ַ������нض�
		for (int j = i; j >= 0; j--)
		{
			if (precord[j] == '/')
			{
				precord[j] = '\0';
				return;
			}
		}
	}
	//��һ��ֱ����ת
	else
	{
		strcat(precord, "/");
		strcat(precord, cname);
	}
}