/**

*				1. �޸�ialloc����
					1.1 ���ӣ��������i�ڵ������
					1.2 �޸ģ�while ((count < NICINOD) && (count <= filsys.s_ninode))
					1.3 �޸ģ��ҵ�һ������i�ڵ�󣬶Ի�����block_buf�ķ��ʱ��i��1��cur_di���Ǻ�iͬ����1
					1.4 ���ӣ�while ((count <= NICINOD) && (count <= filsys.s_ninode)) ���Ӷ�����i�ڵ�����ո��� �ж�if (count != NICINOD)
					4.5 ���ӣ����ܷ����һ�����̿��С�Ŀռ���жϣ��Լ�����ķ�������if (cur_di <= (BLOCKSIZ * ( DINODEBLK - 1) / DINODESIZ))����
*				2. �޸�ifree����
					2.1 �޸ģ��жϿ���i�ڵ�ջδ��������
					2.2 �޸ģ�Ϊfilsys.s_pinode--;
					2.3 ɾ����filsys.s_inode[NICINOD] =dinodeid;
					2.4 ���ӣ����յ�i�ڵ��־Ϊ���У�block_buf[0].di_mode = DIEMPTY�����޸���Ϣд��Ĵ���i�ڵ�
*/
#include <stdio.h>
#include "filesys.h"

//ÿ�����̿ɷ���16��i�ڵ�
static struct dinode block_buf[BLOCKSIZ / DINODESIZ];

/*****************************************
*	��������ialloc()
*	����ֵ������Ĵ���i�ڵ��Ӧ���ڴ�i�ڵ�ָ��
*	��  �ܣ�����һ�����еĴ���i�ڵ�
*	��  �Σ���
******************************************/
struct inode* ialloc()
{
	int i;								//buf�����±�
	int count;							//�����ҵ��Ŀ��д���i�ڵ�
	int block_end_flag;					//��һ�η����һ�����̿��С�Ŀռ�������־
	struct inode* temp_inode;			//�ݴ�i�ڵ�
	unsigned int cur_di;				//��ʱ����i�ڵ�

	//�жϵ�ǰ����i�ڵ��������Ϊ0֤��512��i�ڵ�ȫ���������ȥ��
	if (filsys.s_ninode == 0)
	{
		printf(">Inode null!\n");		// ����i�ڵ�ʹ����  
		return DISKFULL;
	}
	//������нڵ�ָ���ƶ���50֤��0-49���нڵ㶼�Ѿ���ռ��
	//��Ҫ���³������е�i�ڵ���ж�ջ
	if (filsys.s_pinode == NICINOD)
	{
		i = 0;
		//���ҵ��Ŀ���i�ڵ����
		count = 0;
		block_end_flag = 1;

		//׼�����³������еĿ���i�ڵ��ջ
		//����ָ���ջ�����һ��λ�ã�׼�����˴�����Ϊ����i�ڵ�
		filsys.s_pinode = NICINOD - 1;	//���¿���i�ڵ�ָ��Ϊ49����ջ�е����һ��i
		//��ȡ����Ŀ���i�ڵ㣬Ҳ��������i�ڵ�
		cur_di = filsys.s_rinode;

		//���ҵ��Ŀ���i�ڵ�û�г���50������û�г���ʣ��Ŀ���i�ڵ�ʱ�����Լ�������
		//����ȷ������s_ninode��Ϊ0�����Դ����ǽڵ�����ƶ���һ�������ҵ����д��̿�
		while ((count <= NICINOD) && (count <= filsys.s_ninode))
		{
			//�Ƿ��ҵ���block_buf[16]���ȵ�i�ڵ㣬Ҳ�����ܷ�ʼ��һ�����̵�Ѱ��
			if (block_end_flag)
			{
				//���ǽڵ�����λ��С��496�����滹��һ�����̴�С��λ�ÿ��ܴ��ڿ��нڵ�
				//��Ϊ���ǽڵ�����֪����С���нڵ�
				if (cur_di <= (BLOCKSIZ * (DINODEBLK - 1) / DINODESIZ))
				{
					// �����ǽڵ���һ�����̴�С������ֱ�Ӷ���block_buf�У�ע�������������i�ڵ�
					fseek(fd, DINODESTART + cur_di * DINODESIZ, SEEK_SET);
					// ��blocksiz��С�����ݵ�block_buf�У���ʱ�պö���512���ֽڸ�16��С�����飬ÿ��Ԫ��32��С
					fread(block_buf, BLOCKSIZ, 1, fd);
					// ��־block_buf�Ѿ��������Կ�ʼ������һ�����̣��鿴�ж��ٿ��нڵ�
					block_end_flag = 0;
					i = 0;
				}
				//���ǽڵ����496��˵������һ�����̴�С���ܴ��ڿ��нڵ�
				else
				{
					fseek(fd, DINODESTART + cur_di * DINODESIZ, SEEK_SET);
					// ������512��С�����ݸ�block_buf����Ϊ512x32-496x32=512
					fread(block_buf, BLOCKSIZ * DINODEBLK - cur_di * DINODESIZ, 1, fd);
					block_end_flag = 0;
					i = 0;
				}
			}

			//��ʼ���ҵ�ǰ���̿����ж����ѱ�ռ�õ�i�ڵ�
			//����������ڶ������������ȡ��block_bufû�б���ֵ��Ԫ��Ҳ����ű�������Ӱ�죬��Ϊֻ��DIEMPTY�Ż��˳�
			//ֱ���ҵ�һ��û�б�ռ�õģ���ʶΪDIEMPTY�Ŀ���i�ڵ㣬���ҵ�ǰ���ǽڵ�ָ����
			//i������16���ڵ��е���һ��
			//�����һ�����˳���˵������i�ڵ�Ϊ�գ���ʹ�ô����е�����i�ڵ�洢����
			while ((block_buf[i].di_mode != DIEMPTY) && i < (BLOCKSIZ / DINODESIZ))
			{
				cur_di++;
				i++;
			}
			//�����ǰi�ڵ�ȫ����ʹ�ã�block_end_flag=1
			//˵������������ϣ�û�п���i�ڵ�
			if (i == BLOCKSIZ / DINODESIZ)
				block_end_flag = 1;
			//����ڴ������ҵ������С�Ŀ���i�ڵ�
			else
			{
				//�����ж��ҵ�����״̬������i�ڵ㣬���ҵ�50��i�ڵ���ټ����������ҵ�һ������i�ڵ������Ϊ����i�ڵ�
				//��ǰ�ҵ��Ŀ���i�ڵ�û�г���50�������Լ��볬����Ŀ���i�ڵ��ջ����ָ������
				if (count != NICINOD)
				{
					//�������i�ڵ��ջ��
					filsys.s_inode[filsys.s_pinode--] = cur_di;
					//����++
					count++;
					//���ǵ�δ�����i�ڵ�֮���λ�ã��ͷ�ʱ���ǽڵ����ǰ�ƶ�
					cur_di++;
					//block_buf�����ƶ�
					i++;
				}
				//�������50������֤������Ҫ��������������У�ֱ�Ӽ�1���ɣ����˳����ϲ��whileѭ��
				else
					count++;
			}
		}

		filsys.s_rinode = cur_di;			  //�޸�����i�ڵ�
		filsys.s_pinode++;					  //����i�ڵ��˻�һ����ָ��ջ�еĵ�һ������i�ڵ�   
	}

	//�����ǰ���п��ж�ջ�л���i�ڵ�û�б����䣬��ֱ�ӿ�ʼ����
	//��ȡ��ǰ����i�ڵ�Ĵ��̺�4-511�������ڴ��У�Hash%128��
	temp_inode = iget(filsys.s_inode[filsys.s_pinode]);
	//���ﲻ��Ҫ�ͷ�
	fseek(fd, DINODESTART + filsys.s_inode[filsys.s_pinode] * DINODESIZ, SEEK_SET);
	//�����ȥ�Ĵ���i�ڵ�ĸ�����Ϣд����̣�֤���ô��̵�����ѱ�ʹ�ã����ٿ���
	fwrite(&(temp_inode->di_number), sizeof(struct dinode), 1, fd);
	filsys.s_pinode++;						  //����i�ڵ�ָ��+1�������п��ܵ�50
	filsys.s_ninode--;					      //����i�ڵ����-1
	filsys.s_fmod = SUPDATE;				  //�޸ĳ������޸ı�־
	//���ظ�i�ڵ�������ݵ�ָ�룬���޸ĺ��ٽ���iput�������ı�mode
	return temp_inode;
}

/*****************************************
*	��������ifree()
*	����ֵ����
*	��  �ܣ�����һ������i�ڵ�
*	��  �Σ�Ҫ���յĴ���i�ڵ�id
******************************************/
//���ͷź������Կ�����ͨ��s_pinode�Ĵ�С�����жϳ���ǰ��ջ���ж��ٿ���i�ڵ�
//��s_pinode=0ʱ����ʱ����i�ڵ㶼û��ʹ��
//��s_pinode=49ʱ��˵����ʱ�Ŀ���i�ڵ��ջ����æµ״̬�����Ѿ���ʹ��
ifree(unsigned dinodeid)
{
	filsys.s_ninode++;
	//s_pinode==0˵����ǰ����i�ڵ�ջδ��ʹ�ã�ȫ�ǿ���i�ڵ�
	if (filsys.s_pinode != 0)							//����i�ڵ�ջδ��
	{
		//������i�ڵ�ָ�����ƶ�
		//�����ͷ�ʱ�����ٰ���i�ڵ��С����0�������򣬶��Ǽ�һ�ĵط����ÿ���i�ڵ�
		filsys.s_pinode--;
		//�����յĿ���i�ڵ�д�����i�ڵ�ջ�Ķ�Ӧλ��
		filsys.s_inode[filsys.s_pinode] = dinodeid;
	}
	// �����յ�i�ڵ�С�����ǽڵ�
	else if (dinodeid < filsys.s_rinode)
		// ���»��յ�i�ڵ�Ϊ���ǽڵ�
		filsys.s_rinode = dinodeid;
	// ����i�ڵ��־Ϊ����
	block_buf[0].di_mode = DIEMPTY;
	// ���µĿ���i�ڵ㣨���������ڲ��޸ĺ����Ϣ��д�����
	fseek(fd, DINODESTART + dinodeid * DINODESIZ, SEEK_SET);
	//��������ʲô��Ϣ��û��ӣ������ǿ���i�ڵ�
	fwrite(&(block_buf[0].di_number), DINODESIZ, 1, fd);
}