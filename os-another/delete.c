

/*ɾ���ļ�����delete( )*/
#include <stdio.h>
#include "filesys.h"
deletefd(int user_id, char* filename)
{
	unsigned int dinodeid, i;
	struct inode* inode;
	//��ȡ����i�ڵ�λ��
	dinodeid = namei(filename);
	//��ȡ���ڴ�
	if (dinodeid != NULL)
		inode = iget(dinodeid);
	else
	{
		printf(">deleted file does not exist!\n");
		return;
	}
	//�ж�Ȩ��
	if (!access(user_id, inode, WRITE))    /* access denied */
	{
		printf(">failed to remove file because of no authority!\n");
		return;
	}
	//��Ŀ¼�����ļ�
	if ((inode->di_mode & DIDIR) != 0 || (inode->di_mode & DIFILE) != 0)
	{
		char flag;
		//�����Ŀ¼
		if ((inode->di_mode & DIDIR) != 0)                    //�����Ŀ¼
			printf(">Are you sure to remove the ��%s�� directory?(y/n):", filename);
		else printf(">Are you sure to remove the ��%s�� file?(y/n):", filename);
		do
		{
			scanf("%c", &flag);
			_flushall();
			if (flag == 'y' || flag == 'Y')break;
			else if (flag == 'n' || flag == 'N')return;
			else continue;
		} while (1);
		//����Ƿ�ǿ�
		//�����Ŀ¼
		if ((inode->di_mode & DIDIR) != 0)
		{
			//����1��λ�÷���root��0��λ�ò�ʹ��
			if (inode == iget(1)) {
				printf("root dir can't be deleted\n");
				return;
			}
			//��ת��Ҫɾ����Ŀ¼��
			//�ж�Ҫɾ����Ŀ¼�Ƿ�Ϊ��
			chdir(ROOT, filename);
			//����2��֤��������Ŀ¼����Ϊ2���ڴ������ָ��
			if (dir.size > 2)
			{
				printf("DIR %s is NOT EMPTY. You can't remove an unEmpty DIR.\n");
				chdir(ROOT, "..");                    //����ȥ
				return;
			}
			//��������ڣ�������ȥ
			chdir(ROOT, "..");
		}
	}
	//�ҵ���ǰҪɾ����Ŀ¼���ļ�
	for (i = 0; i < dir.size; i++)
		if (dir.direct[i].d_ino == dinodeid) break;  //�ҵ�Ҫɾ�����ļ�����Ŀ¼
	//�������Ŀ¼���ļ���ǰ�ƶ�
	for (i++; i < dir.size; i++)                  //ɾ��Ŀ¼���ļ������������ǰ��
	{
		strcpy(dir.direct[i - 1].d_name, dir.direct[i].d_name);
		dir.direct[i - 1].d_ino = dir.direct[i].d_ino;
	}
	//�ڳ��ռ�
	dir.direct[i - 1].d_ino = 0;
	dir.size = i - 1;
	cur_path_inode->di_size -= DIRSIZ + 2;
	//���û�����ʹ�ø��ļ������ļ�������һ
	inode->di_number--;
	//�������Ϊ0��������ÿ�
	if (inode->di_number == 0)inode->di_mode = DIEMPTY;
	//���di_numberΪ0��ɾ�������ݿ�
	iput(inode);
}
