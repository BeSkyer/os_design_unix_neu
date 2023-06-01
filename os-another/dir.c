/**

*		    �޸�:		1. _dir()�޸�
*							1.1 �������Ŀ¼�����ʱ�ļ�������
*							1.2 �������Ȩ�޵Ĵ���
*							1.3 �޸�����ĸ�ʽ(�ļ����������Ʊ��)
*						2. mkidr�޸�
*							2.1 ����Ŀ¼�����д������̵Ĵ���
*							2.2 ��Ӳ��� user_id �Ա�ʵ�ֶ༶Ŀ¼
*							2.3 �޸�����д��Ŀ¼��ʱi�ڵ��ȡ���߼����󣬸�����Ϊdirid= inode->i_ino;
							2.4 �����µ�Ŀ¼Ҫ��д�µ�dir_bufʱ��ӳ�ʼ�����ܣ��������Ŀ¼ʱ����Ŀ¼
*						3. chdir�޸�
*							3.1 �޸�Ŀ¼�������ж�Ȩ�޵Ĵ���
*							3.2 �޸ķ�������
*							3.3 �����Ȩ�޲���force���Ʊ���
*							3.4 �޸���ѹ��Ŀ¼����ʱ�Ĵ���
*							3.5 �޸���chdir�и�����cur_path_inode��û�����µ�Ŀ¼���ȵ�����
*/
#include <stdio.h>
#include <string.h>
#include "filesys.h"

/*************************
��ʾ��ǰĿ¼����
**************************/
_dir()	/* _dir */
{
	unsigned int di_mode;
	int i, one, j, x = 0;
	//�����ڴ�i�ڵ�
	struct inode* temp_inode;
	printf("CURRENT DIRECTORY :\n");
	//�ڵ�ǰĿ¼�в��ң�˳���ӡ��Ŀ¼
	for (int i = 0; i < dir.size; i++)
	{
		//��ǰĿ¼��d_ino��Ϊ0,��֤������Ŀ¼������root��d_inoΪ1
		//dirָ���ǵ�ǰĿ¼
		if (dir.direct[i].d_ino != DIEMPTY)
		{
			//����ļ���
			printf("%-20s", dir.direct[i].d_name);
			//��ȡ��Ӧ���ڴ�i�ڵ�
			temp_inode = iget(dir.direct[i].d_ino);
			//��ȡ�ļ����ͺʹ�ȡȨ��
			di_mode = temp_inode->di_mode;
			//����ΪĿ¼���ӡd
			if (di_mode & DIDIR)printf("d");
			//����Ϊ�ļ����ӡf
			else { printf("f"); x = 1; }

			/*****************�����ȡȨ��*******************/
			//���Ч������d(Ŀ¼)rwx(������Ȩ��)r-x(�봴����ͬ��Ȩ��)r-x(�����û�Ȩ��) 
			for (int j = 0; j < 9; j++)
			{
				int temp = j % 3;
				one = di_mode % 2;					//step1��step2��ͬ����ж�Ȩ�޵Ĺ���
				di_mode = di_mode / 2;
				if (one)
				{
					switch (temp)
					{
					case 0:printf("r"); break;
					case 1:printf("w"); break;
					case 2:printf("x"); break;
					default:printf(">error in print di_mode!\n");
					}
				}
				else
					printf("-");
			}
			printf("\t");
			/***************�ļ��������С��������Ŀ¼�������ʾ***************/
			if (x)//i�ڵ�ָ������ļ�
			{
				//��־���
				x = 0;
				//����ļ���С
				printf("file size%ld\t", temp_inode->di_size);
				//�������
				printf("block chain:");
				int s = (temp_inode->di_size / BLOCKSIZ) + 1;
				for (j = 0; j < s; j++)
				{
					//�������ʱ���ýڵ�ǡ��û�пռ���䣬��ýڵ�Ϊ65535���ô��̱�־λ��Ϊ65535
					if (temp_inode->di_addr[j] == 65535)
						continue;
					//�ɹ����䵽�˿ռ䣬��ʱ��ʾռ�õĿռ�
					else
						printf("%d ", temp_inode->di_addr[j]);
				}
				printf("\n");
			}
			else
				printf("<dir>\n");								//i�ڵ�ָ�����Ŀ¼
			iput(temp_inode);
		}
	}//for
}

/**************************
����Ŀ¼����
**************************/
mkdir(int user_id, char* dirname)
{
	int dirid, dirpos;
	struct inode* inode;
	struct direct buf[BLOCKSIZ / (DIRSIZ + 2)];
	unsigned int block;

	dirid = namei(dirname);//���ڵ�ǰĿ¼�����Ƿ����ͬ��Ŀ¼��
	if (dirid != NULL)
	{//���ڳ�ͻ����������
		inode = iget(dirid);
		if (inode->di_mode & DIDIR)	//����ͬ��Ŀ¼��
			printf("\n%s directory already existed! ! \n");
		else						//���ļ�����ͻ
			printf("\n%s is a file name, &can't create a dir the same name", dirname);
		iput(inode);
		return;
	}
	else
	{//������
		dirpos = iname(dirname);//�ڵ�ǰĿ¼�в��ҿ�Ŀ¼��
		if (dirpos == -1)
		{//��ǰĿ¼����
			printf(">dir is full\n");
			return;
		}
		else
		{//���п�Ŀ¼��
			inode = ialloc();							//�����µĴ���i�ڵ㣬�������ڴ�
			dirid = inode->i_ino;						//��ȡ�·����i�ڵ��
			dir.direct[dirpos].d_ino = dirid;			//��д����i�ڵ��
			dir.size++;									//��ǰĿ¼����+1
			/*	fill the new dir buf */
			for (int x = 0; x < BLOCKSIZ / (DIRSIZ + 2); x++)
			{//��ʼ��Ҫ��ΪĿ¼�����ݿ����������Ŀ¼ʱ�������
				strcpy(buf[x].d_name, "");				//�������				
				buf[x].d_ino = 0;						//i�ڵ�����
			}
			strcpy(buf[0].d_name, ".");					//.�ǵ�ǰĿ¼����˼
			buf[0].d_ino = dirid;						//��ǰĿ¼�����´�����Ŀ¼
			strcpy(buf[1].d_name, "..");				//..�ǵ�ǰĿ¼����һ��Ŀ¼
			buf[1].d_ino = cur_path_inode->i_ino;		//��һ��Ŀ¼���ݿ��Ӧ��i�ڵ���cur_path_inode��
			block = balloc();										//�����µĴ��̿�*
			//���ļ�i�ڵ�����д�����
			fseek(fd, DATASTART + block * BLOCKSIZ, SEEK_SET);		//��λ����Ĵ���
			fwrite(buf, BLOCKSIZ, 1, fd);							//д�����
			inode->di_size = 2 * (DIRSIZ + 2);						//�ļ�����/��С
			inode->di_number = 1;									//�ļ���������
			inode->di_mode = user[user_id].u_default_mode | DIDIR;	//��ȡȨ��������
			inode->di_uid = user[user_id].u_uid;					//�û�ID
			inode->di_gid = user[user_id].u_gid;					//�û���ID
			inode->di_addr[0] = block;								//������
			iput(inode);
			return;
		}
	}
}

mkdir_user(int user_id, char* dirname)
{
	int dirid, dirpos;
	struct inode* inode;
	struct direct buf[BLOCKSIZ / (DIRSIZ + 2)];
	unsigned int block;

	dirid = namei(dirname);//���ڵ�ǰĿ¼�����Ƿ����ͬ��Ŀ¼��
	if (dirid != NULL)
	{//���ڳ�ͻ����������
		inode = iget(dirid);
		if (inode->di_mode & DIDIR)	//����ͬ��Ŀ¼��
			printf("\n%s directory already existed! ! \n");
		else						//���ļ�����ͻ
			printf("\n%s is a file name, &can't create a dir the same name", dirname);
		iput(inode);
		return;
	}
	else
	{//������
		dirpos = iname(dirname);//�ڵ�ǰĿ¼�в��ҿ�Ŀ¼��
		if (dirpos == -1)
		{//��ǰĿ¼����
			printf(">dir is full\n");
			return;
		}
		else
		{//���п�Ŀ¼��
			inode = ialloc();							//�����µĴ���i�ڵ㣬�������ڴ�
			dirid = inode->i_ino;						//��ȡ�·����i�ڵ��
			dir.direct[dirpos].d_ino = dirid;			//��д����i�ڵ��
			dir.size++;									//��ǰĿ¼����+1
			/*	fill the new dir buf */
			for (int x = 0; x < BLOCKSIZ / (DIRSIZ + 2); x++)
			{//��ʼ��Ҫ��ΪĿ¼�����ݿ����������Ŀ¼ʱ�������
				strcpy(buf[x].d_name, "");				//�������				
				buf[x].d_ino = 0;						//i�ڵ�����
			}
			strcpy(buf[0].d_name, ".");					//.�ǵ�ǰĿ¼����˼
			buf[0].d_ino = dirid;						//��ǰĿ¼�����´�����Ŀ¼
			strcpy(buf[1].d_name, "..");				//..�ǵ�ǰĿ¼����һ��Ŀ¼
			buf[1].d_ino = cur_path_inode->i_ino;		//��һ��Ŀ¼���ݿ��Ӧ��i�ڵ���cur_path_inode��
			block = balloc();										//�����µĴ��̿�*
			//���ļ�i�ڵ�����д�����
			fseek(fd, DATASTART + block * BLOCKSIZ, SEEK_SET);		//��λ����Ĵ���
			fwrite(buf, BLOCKSIZ, 1, fd);							//д�����
			inode->di_size = 2 * (DIRSIZ + 2);						//�ļ�����/��С
			inode->di_number = 1;									//�ļ���������
			inode->di_mode = ODIWRITE | ODIEXICUTE | ODIREAD | DIDIR;	//��ȡȨ��������
			inode->di_uid = user[user_id].u_uid;					//�û�ID
			inode->di_gid = user[user_id].u_gid;					//�û���ID
			inode->di_addr[0] = block;								//������
			iput(inode);
			return;
		}
	}
}

/**********************************
�޸�Ŀ¼����(Ŀ¼��ת)
force: �ⲿ�����ź�
		1:�����û�����ǿ������
		0:��Ҫ�����ж�
dirname: Ŀ��Ŀ¼��
**********************************/
int chdir(int force, char* dirname)
{
	unsigned int dirid;
	struct inode* inode;
	unsigned short block;
	int i, j, low = 0, high = 0;
	dirid = namei(dirname);//�ڵ�ǰĿ¼��Ѱ��

	if (dirid == NULL)
	{//���ڵ�ǰĿ¼��
		printf(">%s does not existed\n", dirname);
		return -1;
	}
	else
	{//���ڵ�ǰĿ¼�У����ҵ�����Ӧi�ڵ��
		inode = iget(dirid);//��ȡ�ڴ�i�ڵ�
		if (inode->di_mode & DIFILE)
		{//�ҵ������ļ�
			printf(">cannot use the command on file!\n");
			iput(inode);
			return -1;
		}
		if ((!force) && (!access(user_id, inode, EXICUTE)))
		{	//�û���ִ�������Ȩ��
			printf(">has not access to the directory %s\n", dirname);
			iput(inode);
			return -1;
		}
		//ÿ����ת֮ǰ��Ҫ���ڱ�Ŀ¼�½��еĲ����������ϡ�����
		//����Ҫ���ǽ��оۺϣ������Ժ����Ŀ�չ
		//dir�ǵ�ǰ���ڵ�Ŀ¼�������˵�ǰĿ¼�µ��ļ���size���Լ��洢��FCB[128]����
		//dir�Ǵ洢���ڴ��е�
		//ÿ��FCB����Ӧ��һ��d_ino��Ҳ����i�ڵ���̺ţ�root��1
		//�Ƚ�Ŀ¼�е�fcb��Ϣ������һ�𣬷�������д��һ��������б���
		/********* pack the current directory **********/
		for (i = 0; i < dir.size; i++)
		{
			//�����ǰĿ¼��û��ʹ��
			if (dir.direct[i].d_ino == 0)
			{
				//��������б�ʹ�õ�Ŀ¼���Ŀ¼��ۺ���һ��
				for (j = i + 1; j < DIRNUM; j++)
					if (dir.direct[j].d_ino != 0)
						break;
				memcpy(&dir.direct[i], &dir.direct[j], DIRSIZ + 2);
				dir.direct[j].d_ino = 0;
			}
		}
		//���µ�ǰĿ¼i�ڵ���ռ�õĴ��̿�
		//cur_path_inodeָ���˵�ǰĿ¼i�ڵ㣬ע��͵�ǰĿ¼FCB�Ĵ��̺�ָ���λ����ͬ
		//Ҳ����dir��������ǰĿ¼�¼���Ϣ
		//cur_path_inode�������������Ϣ
		/*	write back the current directory *//************************************************/

		//���Ȼ�õ�ǰĿ¼��Сռ�õĴ��̿���
		//�ͷ�Ŀ¼��ռ�õĴ��̿�
		//�ͷŵ���Ϣ��Ȼ����ͨ��dir.direct[j]��ȡ
		//Ҳ�����ͷ��˴��������FCB��Ϣ�����ǵ�ǰĿ¼dir���Եõ�FCB��Ϣ
		for (i = 0; i < cur_path_inode->di_size / BLOCKSIZ + 1; i++)
		{
			bfree(cur_path_inode->di_addr[i]);
		}
		//Ϊ��ǰĿ¼��i�ڵ����·�����̿飬����ʱ��������Ŀ¼���٣�һ���̿�������32��Ŀ¼
		//�����128��Ŀ¼�ÿ����������32��Ŀ¼��
		//��Ŀ¼dir��fcb��Ϣд��һ��������б��棬�����´���תʱ��ȡ��dir
		for (i = 0, j = 0; i < dir.size; i += BLOCKSIZ / (DIRSIZ + 2), j++)
		{
			//����һ�����̿飬����32��Ŀ¼
			block = balloc();
			//��Ŀ¼���䵽������̿��ϴ洢
			cur_path_inode->di_addr[j] = block;
			//��λ���ÿ��д��̴�
			fseek(fd, DATASTART + block * BLOCKSIZ, SEEK_SET);
			//��dir.direct[j]������д�뵽�ÿ��д��̴�
			//��Ŀ¼�е��㹻���fcb��Ϣ����д�����
			//����fwrite���Ծ���д��
			fwrite(&dir.direct[j], BLOCKSIZ, 1, fd);
		}
		//����cur_path_inode�Ĵ�С/����(�ֽ�)
		cur_path_inode->di_size = dir.size * (DIRSIZ + 2);
		//����ǰĿ¼�ͷţ���ʱ�Ѿ�����ˣ�1.��Ŀ¼i�ڵ��¼����Ϣ���£������ȣ�ָ���ļ��������ַ��
		//2.�ļ�ָ��������ַ��֤�Ǿ������յķ��ڴ�����
		iput(cur_path_inode);
		//����Ŀ¼��i�ڵ㸳ֵ��cur_path_inode
		cur_path_inode = inode;
		//��һ��Ŀ¼��dir.size�Ļ�ȡ
		dir.size = cur_path_inode->di_size / (DIRSIZ + 2);
		/*	read the change dir from disk */
		//�Ӵ����ж�ȡ������fcb��dir��fcb�洢����
		for (i = 0, j = 0; i < inode->di_size / BLOCKSIZ + (inode->di_size % BLOCKSIZ != 0); i++, j += BLOCKSIZ / (DIRSIZ + 2))
		{
			//��ȡ�µ�Ŀ¼���ݿ�
			fseek(fd, DATASTART + inode->di_addr[i] * BLOCKSIZ, SEEK_SET);
			//��ȡfcb��dir
			fread(&dir.direct[j], BLOCKSIZ, 1, fd);
		}
		return;
	}
}