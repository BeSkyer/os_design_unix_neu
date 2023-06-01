#include "filesys.h"
#include <stdio.h>

/*****************************************
*	函数名：cpy()
*	返回值：无
*	功  能：读出一个内容到全局变量
*	形  参：文件名，全局变量
******************************************/
void cpy(char* filename, char* content)//读出内容到一个全局变量
{
	//粘贴缓存
	char cp_buf[512];
	//从目录中搜索该文件的磁盘i节点逻辑编号
	int cpdino = namei(filename);
	//得到文件名
	char* cpfilename = filename;
	//声明内存i节点指针
	struct inode* inode;
	//从磁盘读入内存
	inode = iget(cpdino);
	//先判断用户能否读取该文件
	//由于access是对inode进行操作的，这里需要进行一个转换
	if (!access(user_id, inode, READ))
	{

		printf("\n对不起，您没有复制该文件的权限！\n");
		iput(inode);
		return;
	}
	//如果有该权限还是先从内存中释放
	iput(inode);
	//返回在用户表中的位置
	int tfd = xfa(filename);
	//存在该文件
	if (tfd != -1)
	{
		//打开系统文件表对应项
		int sys_otpos = user[user_id].u_ofile[tfd];
		//得到文件大小
		int temp_file_size = sys_ofile[sys_otpos].f_inode->di_size;
		//将文件内容读入缓冲区，这里文件要是大于512该怎么办？
		//在用户打开文件中，读取对应内容
		read(tfd, cp_buf, temp_file_size);
		int i = 0;
		while (((cp_buf[i] >= '0' && cp_buf[i] <= '9') || (cp_buf[i] >= 'a' && cp_buf[i] <= 'z') || (cp_buf[i] >= 'A' && cp_buf[i] <= 'Z')))
			i++;
		cp_buf[i] = '\0';
		//读要赋值的文件copy到全局content中
		strcpy(content, cp_buf);
		printf("\n>content you want to copy: %s\n", content);//输出文件内容
	}
	else
	{
		printf(">Cannot read a file that is not open or not exixted in current directory\n");
		return;
	}
	//修改打开文件表
	close(user_id, tfd);
	return;
}

/*****************************************
*	函数名：pst
*	返回值：无
*	功  能：粘贴到指定位置
*	形  参：文件名，全局变量
******************************************/
void pst(char* filename, char* content)
{
	//找到文件的磁盘i节点号
	int cpdino = namei(filename);
	//生成内存结点
	struct inode* inode;
	inode = iget(cpdino);
	//判断权限
	if (!access(user_id, inode, READ))
	{

		printf("\n对不起，您没有复制该文件的权限！\n");
		iput(inode);
		return;
	}
	iput(inode);
	//得到用户文件列表
	int tfd = xfa(filename);
	//文件存在且被打开
	if (tfd != -1)
	{
		//找到系统事件项
		int sys_otpos = user[user_id].u_ofile[tfd];
		//获取文件大小
		int temp_file_size = sys_ofile[sys_otpos].f_inode->di_size;
		int ssize = 0;
		//一直到文件末尾
		while (content[ssize] != '\0')
			ssize++;
		//在用户打开文件中追加对应内容
		write(tfd, content, ssize);
	}
	else
		printf(">Cannot write a file that is not open or not exixted in current directory\n");
	//修改打开文件表
	close(user_id, tfd);
	return;
}
