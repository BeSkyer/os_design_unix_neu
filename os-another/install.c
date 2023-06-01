/**

*		�޸ģ�	1. fd = fopen("filesystem", "w+r+b"); ��Ϊ r+b
				2. ��ӳ�ʼ��װ��password�Ĺ���
				3. ע�ͱ���
*/
#include <stdio.h>
#include <string.h>
#include "filesys.h"

install()
{
	int i, j;
	/**********װ��file column **********/
	fd = fopen("filesystem", "r+b");//�Զ����ƿɶ�д��ʽ��
	if (fd == NULL)
	{
		printf("\nfilesys can not be loaded\n");
		exit(0);
	}
	/*************װ��password*****************/
	//0�����������root��fcb
	//1�����������etc��fcb
	//2�����������password��18���û�������Ϣ
	//0��i�ڵ㲻ʹ��
	//1��i�ڵ��Ӧroot��Ϣ
	//2��i�ڵ��Ӧetc��Ϣ
	//3��i�ڵ��Ӧpassword��Ϣ
	//һ��18���û�������ֻ��ʾ4��
	{
		fseek(fd, DATASTART + 2 * BLOCKSIZ, SEEK_SET);
		fread(pwd, BLOCKSIZ, 1, fd);
		for (int i = 0; i < PWDNUM; i++) {
			if (strcmp(" ", pwd[i].username) != 0) {
				printf("%s", pwd[i].username);
				printf("   %s\n", pwd[i].password);
			}
			else break;
		}
	}
	/**********�ӳ������ж����ļ�ϵͳ **********/
	//������ռ���������̿飬�������̿ռ�����i�ڵ�ռ����
	fseek(fd, BLOCKSIZ, SEEK_SET);
	fread(&filsys, sizeof(struct filsysi), 1, fd);

	/**********��ʼ���ڴ�i�ڵ�Ĺ�ϣ��**********/
	for (i = 0; i < NHINO; i++)//NHINO=128
	{
		//�Ƚ�ÿһͰ����Ϊ��
		hinode[i].i_forw = NULL;
	}

	/**********��ʼ��ϵͳ���ļ���**********/
	for (i = 0; i < SYSOPENFILE; i++)//SYSOPENFILE=40
	{
		sys_ofile[i].f_count = 0;					//�ļ����ü�������
		sys_ofile[i].f_inode = NULL;				//�ڴ�i�ڵ�ָ������
	}

	/**********��ʼ���û��� **************/
	for (i = 0; i < USERNUM; i++)//USERNUM=10
	{
		user[i].u_uid = -1;						//�û�ID���
		user[i].u_gid = -1;						//�û����ID���
		for (j = 0; j < NOFILE; j++) // FILE = 20        //�û����ļ������
		{
			//�û����ļ�������ʼ��Ϊ(SYSOPENFILE+ 1)=41��41Ĭ�ϸñ���û���ļ���
			user[i].u_ofile[j] = SYSOPENFILE + 1;
		}
	}

	/**********����Ŀ¼���뵱ǰĿ¼�� **********/
	//����rootĿ¼�ڵ�
	cur_path_inode = iget(1);
	//��ǰĿ¼��ʼ��size������i�ڵ��fcbû�ж���dir��
	dir.size = cur_path_inode->di_size / (DIRSIZ + 2);	//Ŀ¼��ĸ���������ǰĿ¼��С��ÿ��Ŀ¼���СΪ16
	//��ʼ��fcb�е�name�Լ�i�ڵ㣬0�ǲ��õ�i�ڵ�
	//ѭ��������ǰĿ¼������Ŀ¼��
	//��ÿ��Ŀ¼������Ƴ�ʼ��Ϊ�ո��ַ���������Ϊ14���ո�
	// ��ÿ��Ŀ¼���i�ڵ������ų�ʼ��Ϊ0����ʾ��Ŀ¼�ǰδ��ʹ�á�
	for (i = 0; i < DIRNUM; i++)
	{
		strcpy(dir.direct[i].d_name, "              ");
		dir.direct[i].d_ino = 0;						//0��i�ڵ��ǲ�ʹ�õ�
	}

	/********** 5.1 �����������е�Ŀ¼�����Ŀ¼�� **********/
	/// BLOCKSIZ / (DIRSIZ + 2)					ÿ����̿��ܴ��µ�Ŀ¼�����32��
	/// dir.size / (BLOCKSIZ / (DIRSIZ + 2))	���е�Ŀ¼��ռ�õĴ��̿��� �����4�� = 128 /32
	int x = dir.size / (BLOCKSIZ / (DIRSIZ + 2));  //������Ŀ¼����Ҫ�Ĵ��̿�
	//��Ŀ¼�µ�fcb�� û���ó���һ�����̿�洢��Ҳ���ǳ���0���̿飬�� û���ñ�Ĵ��̿�洢
	//��֤Ŀ¼ռ������������
	for (i = 0; i < x; i++)
	{
		/// cur_path_inode->di_addr[i]			Ŀ¼���ڵĴ��̿��� i�����ȡֵΪ0 1 2 3 ��Ϊ����Ŀ¼�����ռ��4����̿�
		/// BLOCKSIZ / (DIRSIZ + 2)				��Ϊ�������Ӵ��̶�һ�����̿����Ŀ¼��
		//��ȡroot��inode�еĴ��̿ռ����飬һ��di_addr[10]�����Դ洢320��Ŀ¼fcb�����ǹ涨ֻ��128��
		//���ݵ�ǰĿ¼��i�ڵ�(cur_path_inode)�еĴ��̿���(di_addr)������Ŀ����̿��λ�ã������ļ�ָ��(fd)�ƶ�����λ��׼����ȡ����
		fseek(fd, DATASTART + BLOCKSIZ * cur_path_inode->di_addr[i], SEEK_SET);

		//ÿһ�ζ���32��Ŀ¼�һ�����̿飬��֤ȫ������
		fread(&dir.direct[(BLOCKSIZ / (DIRSIZ + 2)) * i], BLOCKSIZ, 1, fd);
	}
	/********** 5.2 ���벻��һ�����̿��С��Ŀ¼�� **********/
	/// cur_path_inode->di_size % BLOCKSIZ		ʣ���Ŀ¼��ռ���ֽ�������ռ�ô��̿����Ч�ֽ���
	fseek(fd, DATASTART + BLOCKSIZ * cur_path_inode->di_addr[i], SEEK_SET);
	fread(&dir.direct[BLOCKSIZ / (DIRSIZ + 2) * i], cur_path_inode->di_size % BLOCKSIZ, 1, fd);
}
