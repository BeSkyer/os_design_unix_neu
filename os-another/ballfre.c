/**

*				�ļ�˵�����������ʹ��
*/
#include <stdio.h>
#include "filesys.h"

//���ڽ����鳤�������
static unsigned int block_buf[BLOCKSIZ];
/*****************************************
*	��������balloc()
*	����ֵ������Ĵ��̿���
*	��  �ܣ�����һ�����д��̿�
*	��  �Σ���
******************************************/

unsigned int balloc()
{
	unsigned int free_block;

	if (filsys.s_nfree == 0)//���п��ܿ���Ϊ0����ʱλ�ڵڶ�����鳤��λ��1��
	{
		printf("\nDisk Full!!! \n");
		return DISKFULL;
	}
	//�ӳ�������п��ջ��ȡ��һ�����п�ָ��ָ��Ŀ��п�
	//�����free_block��Ȼ���߼����
	free_block = filsys.s_free[filsys.s_pfree];

	//ֻʣ���һ���鳤�飬����Ҫ�ڸ�ֵ 
	if (filsys.s_nfree == 1)
	{
		filsys.s_pfree--;
		filsys.s_nfree--;
		filsys.s_fmod = SUPDATE;
		return free_block;
	}

	//���п�ջ�е����һ����--�鳤�鱻����
	//Ĭ�������±�1��λ�÷����鳤��
	//���鳤���м�¼����һ����п���Ϣ���볬������
	if (filsys.s_pfree == 1)
	{
		//�����ļ�ָ��fd��λ��(�ļ�ָ��,ƫ����,��׼)
		fseek(fd, DATASTART + free_block * BLOCKSIZ, SEEK_SET);
		//���ڽ������ݵ���ʼ��ַ,ÿ����������ֽ���,��������,������
		//���ļ��ж�Ӧ������д��block_buf�У�����block_buf��ֵ��������
		fread(block_buf, BLOCKSIZ, 1, fd);

		//���³�����
		for (int i = 0; i <= NICFREE; i++)
		{
			filsys.s_free[i] = block_buf[i];
		}
		//����һ����50����Ϊÿ�η�����п鶼�Ǵӳ������з���
		filsys.s_pfree = NICFREE;
	}
	//��������鳤�飬�����鳤�ȼ�һ
	else
		filsys.s_pfree--;	//�޸Ŀ��п�ָ��-1

	filsys.s_nfree--;		//���п��ܿ���-1
	filsys.s_fmod = SUPDATE;//�������޸ı�־

	return free_block;
}
/*****************************************
*	��������bfree()
*	����ֵ����
*	��  �ܣ�����һ�����̿�
*	��  �Σ�Ҫ���յĴ��̿��ţ���Ŵ�0��ʼ��
******************************************/
bfree(unsigned int block_num)
{
	filsys.s_nfree++;				//���п�����+1
	filsys.s_fmod = SUPDATE;		//�������޸ı�־

	if (filsys.s_pfree == NICFREE)	/* if s-free full */
	{
		for (int i = 0; i <= NICFREE; i++)
		{
			//���ճ������еĿ����̿���Ϣ����ֵ�������ɵ��鳤��
			block_buf[i] = filsys.s_free[i];
		}
		fseek(fd, DATASTART + BLOCKSIZ * block_num, SEEK_SET);
		//д�������ɵ��鳤��
		fwrite(block_buf, BLOCKSIZ, 1, fd);
		filsys.s_pfree = 1;					//�޸Ŀ��п�ָ��	
		filsys.s_free[0] = 1;				//�޸Ŀ��п�ջ�п��п���
		filsys.s_free[1] = block_num;		//�ջصĿ��п��д����п�ջ
	}
	else
	{
		filsys.s_free[0]++;								//���п�ջ�п���+1
		filsys.s_free[++(filsys.s_pfree)] = block_num;	//�Ƚ�ָ�����,���ջصĿ��п��д����п�ջ			
	}
}