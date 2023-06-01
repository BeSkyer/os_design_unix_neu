/**

				�ļ�˵�����ڴ�i�ڵ������i�ڵ��ת����ӳ��
*/
#include <stdio.h>
#include "filesys.h"

/*****************************************
*	��������iget()
*	����ֵ���ڴ�i�ڵ�ָ��
*	��  �ܣ�����һ���ڴ�i�ڵ�
*	��  �Σ�����i�ڵ�id
******************************************/
//�������е�i�ڵ�����ڴ���
struct inode* iget(unsigned int dinodeid)
{
	int existed = 0, inodeid;
	long addr;
	struct inode* temp = NULL, * newinode;
	//�Դ����е�i�ڵ����hash��һ��512��i�ڵ�
	inodeid = dinodeid % NHINO;			//��ϣ/ɢ�к�������

	//ͨ����ϣ��������ϣ������֯�ڴ�i�ڵ���߼��ṹ��ͼʾ���μ�
	//�����趨ÿ�е��׸�����hinode[inodeid]�ǲ��õ�
	//�����Ϊ��˵�������ڴ�i�ڵ㲻�Ǹ��е�����
	if (hinode[inodeid].i_forw != NULL)
	{
		temp = hinode[inodeid].i_forw;
		//�����������нڵ�
		while (temp)
		{
			if (temp->i_ino == dinodeid)//ԭ���д���  �޸ģ�inodeid->dinodeid
			{/* existed */
				existed = 1;
				temp->i_count++;		//���ü���+1
				return temp;
			}
			else/* not existed */
				temp = temp->i_forw;
		}
	}
	//�����ϣ������û��Ҫ�ҵ��ڴ�i�ڵ㣬���в����ڣ����߸���û��
	//˵����i�ڵ㻹δ�����䣬������з���
	/* 1. calculate the addr of the dinode in the file sys column */
	addr = DINODESTART + dinodeid * DINODESIZ;
	/* 2. malloc the new mode */
	newinode = (struct inode*)malloc(sizeof(struct inode));	//����һ���µ��ڴ�i�ڵ�
	/* 3. read the dinode to the mode */
	//��ȡ����i�ڵ�Ķ�Ӧ��Ϣ
	fseek(fd, addr, SEEK_SET);
	//д���ڴ�i�ڵ�Ķ�Ӧλ��
	fread(&(newinode->di_number), DINODESIZ, 1, fd);				//�Ӵ����ж�����Ӧ�Ĵ���i�ڵ�����ݣ�д���ڴ�i�ڵ���Ӧ�Ĳ���
	/* 4. put it into hinode[inodeid] queue */

	//����Ϊ��,�·����i�ڵ��Ǹ�������
	if (hinode[inodeid].i_forw == NULL)
	{
		hinode[inodeid].i_forw = newinode;
		//����������ǰָ��Ϊ��
		newinode->i_forw = NULL;
		//��ָ��ָ���ϣ���׵�ַ
		newinode->i_back = &hinode[inodeid];
	}
	//���в��գ������ϣ��ĩβ
	else
	{
		//��ȡ������ڴ�i�ڵ��ַ
		temp = hinode[inodeid].i_forw;
		//����ǰѰ�ң�ֱ��ĩβ
		while (temp->i_forw)
			temp = temp->i_forw;
		//temp�ƶ�����β
		//��β�������µ�i�ڵ�
		//��i�ڵ��Ϊ��β
		temp->i_forw = newinode;
		newinode->i_back = temp;
		newinode->i_forw = NULL;
	}

	/* 5. initialize the mode */
	newinode->i_count = 1;				//���ü�������ǰ�г���ʹ�øýڵ�
	newinode->i_flag = 0;				/* flag for not update */
	newinode->i_ino = dinodeid;			//����i�ڵ��ţ���ʶ�ڴ�i�ڵ�

	return newinode;					//�����·����i�ڵ�
}

/*****************************************
*	��������iput()
*	����ֵ����
*	��  �ܣ����ڴ�i�ڵ��ͷţ�ͬʱ�����Ƿ���Ҫɾ���ڴ��еĸ�i�ڵ������������ǰ���̲���ʹ�ø��ļ�
*	��  �Σ��ڴ�i�ڵ�ָ��
******************************************/
iput(struct inode* pinode)
{
	long addr;
	unsigned int block_num;
	//�鿴��ǰ����û���û����������̵��ø��ļ�
	if (pinode->i_count > 1)//�����ü���>=2
	{
		pinode->i_count--;	//���ü���-1
		return;
	}
	//���û��������ͷ�
	else					//�����ü���=1
	{
		//�ļ���ʶ�� �������ɾ�����ͽ�i�ڵ���Ϣд�ش���
		if (pinode->di_number != 0)
		{
			/* write back the mode */
			//��Ҫ��һ����˵����i�ڵ��Ƿ�ʹ�ã������ǿ���̬
			addr = DINODESTART + pinode->i_ino * DINODESIZ;
			fseek(fd, addr, SEEK_SET);
			fwrite(&(pinode->di_number), DINODESIZ, 1, fd);//���ڴ�i�ڵ��Ӧ�Ĵ���i�ڵ㲿�ֵ�����д�����
		}
		//���di_number=0����֤����ʱ����Ҫ��i�ڵ��Ӧ�Ĵ�������Ҳɾ��
		else
		{
			/* rm the mode & the block of the file in the disk */
			//���ڴ�i�ڵ��Ӧ���ļ���ռ�Ĵ��̿���
			block_num = pinode->di_size / BLOCKSIZ;
			for (int i = 0; i < block_num; i++)				 //�ͷŴ��̿飬һ��һ��
			{
				bfree(pinode->di_addr[i]);					 //���޸�ballocΪbfree

			}
			ifree(pinode->i_ino);							 //�ͷŴ���i�ڵ�
		}

		/* free the mode in the memory */
		//������ϣ���������ڴ�i�ڵ�������Ƴ�
		//��i�ڵ����������һ��
		if (pinode->i_forw == NULL)
			pinode->i_back->i_forw = NULL;
		//����������һ��
		else
		{
			pinode->i_forw->i_back = pinode->i_back;
			pinode->i_back->i_forw = pinode->i_forw;
		}
		free(pinode);
	}
}