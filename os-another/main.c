
#include <stdio.h>
#include "filesys.h"
#include <stdlib.h>
#include <conio.h>

struct hinodei hinode[NHINO];
//内存中的目录
struct diri dir;
//系统打开表
struct file sys_ofile[SYSOPENFILE];
//文件超级块
struct filsysi filsys;
//用户口令字，占用28字节，位于磁盘2号物理块上
struct pwdi pwd[PWDNUM];
//用户打开文件表
struct useri user[USERNUM];
//磁盘读写头
FILE* fd;                                                            
//当前目录i节点
struct inode* cur_path_inode;
//当前用户的用户id，记录用户在用户口令字列表中的位置
int user_id;
//系统打开表的位置
int pos_in_sysofile;
//文件缓冲区，无论是目录i节点还是文件i节点，最多只能有10个物理块
char read_buffer[10 * BLOCKSIZ] = { 0 };
//复制缓存区
char cbuf[512] = { 0 };
//复制文件名
char cname[14] = { 0 };


void main()
{
	char login_name[PWDSIZ], login_password[PWDSIZ];
	int exit_flag = 1;
	char tprecord[1000] = "";
	char cmd[10];
	int user_id = -1;

	printf("Unix File System VER1.0[版本1.0 2023-5-25]\n");
	printf("\nDo you want to format the disk? (y(es)/n(o)) :\n");

	char format_choice[3];
	fgets(format_choice, sizeof(format_choice), stdin);
	if (format_choice[0] == 'y')
	{
		printf("\nFormat will erase all content on the disk. Are you sure? (y(es)/n(o)) :\n");

		char format_confirm[3];
		fgets(format_confirm, sizeof(format_confirm), stdin);
		if (format_confirm[0] == 'y')
		{
			printf("Please input root password: ");

			char tp[PWDSIZ];
			fgets(tp, sizeof(tp), stdin);

			size_t len = strlen(tp);
			if (len > 0 && tp[len - 1] == '\n') // Remove trailing newline character
			{
				tp[len - 1] = '\0';
			}
			else // Input exceeded buffer size, clear input buffer
			{
				clear_input_buffer();
				printf("\n> Input exceeded maximum length. System is not being formatted...\n");
				return 0;
			}

			if (strcmp(tp, "root") == 0)
			{
				format();
				printf("\n>System is formative now\n");
			}
			else
			{
				printf("\n> Incorrect root password! System is not being formatted...\n");
			}
		}
		else
		{
			printf("\n> System is not being formatted.\n");
		}
	}
	install();
	_dir();
	do {
		while (user_id == -1)
		{
			memset(login_name, 0, sizeof(login_name));		//初始化login_id缓冲区，用0填充该数组
			memset(login_password, 0, sizeof(login_password));		//初始化login_password缓冲区，用0填充该数组
			printf("\nPlease enter your account to log in \n");
			printf("\tUSERNAME:   ");
			scanf("%s", login_name);								//输入用户ID到login_name缓冲区
			printf("\tPASSWORD:   ");
			char pwdt;
			for (int i = 0; (i < PWDSIZ) && ((pwdt = _getch()) != '\r'); i++)
				login_password[i] = pwdt;							//输入密码到login_name缓冲区
			putchar('\n');
			//获得用户在18个用户口令列表中的位置
			//默认打开主目录文件表
			user_id = login(login_name, login_password);				//用户登录
			if (user_id != -1)
			{
				//登陆成功
				//注意root的id为1，其余用户的id从2开始，而用户文件表的下标从0开始
				if (user[user_id].u_uid != ROOT)
				{
					//会从root跳转到对应的用户目录下
					//创建用户时，会在root下新建目录
					chdir(1, login_name);
					sprintf(tprecord, "%s@VirtualUnix:~/root/%s", login_name, login_name);
				}
				else
				{
					strcpy(tprecord, "root@VirtualUnix:~/root");
				}
			}
		}
		printf("%s$ ", tprecord);
		fgets(cmd, sizeof(cmd), stdin); // 获取输入的命令
		// 移除命令末尾的换行符
		size_t len = strlen(cmd);
		if (len > 0 && cmd[len - 1] == '\n') {
			cmd[len - 1] = '\0';
		}


		//命令端

		/**********************1.显示当前目录命令***********************/
		if (strcmp(cmd, "ls") == 0) {
			_dir();
		}
		/**********************2.创建目录命令**************************/
		else if (strcmp(cmd, "mkdir") == 0)
		{
			char temp_dir_name[DIRSIZ];
			fgets(temp_dir_name, sizeof(temp_dir_name), stdin);
			temp_dir_name[strcspn(temp_dir_name, "\n")] = '\0'; // 移除末尾的换行符

			if (user_id == 0)
				mkdir(user_id, temp_dir_name);
			else
				mkdir_user(user_id, temp_dir_name);
		}
		/**********************3.跳转目录命令**************************/
		else if (strcmp(cmd, "cd") == 0)
		{
			int success_flag = 0;
			char temp_dir_name[DIRSIZ];
			fgets(temp_dir_name, sizeof(temp_dir_name), stdin);
			temp_dir_name[strcspn(temp_dir_name, "\n")] = '\0'; // 移除末尾的换行符
			success_flag = chdir(0, temp_dir_name);
			if (success_flag == 1)//如果跳转成功修改窗口中输出的路径
				chpath(tprecord, temp_dir_name, cur_path_inode->i_ino);
			else
				printf(">Failed to change directory.Please retry\n");
		}
		/**********************4.创建文件命令***********************/
		else if (strcmp(cmd, "creat") == 0)
		{
			char temp_file_name[DIRSIZ];//文件名长度14个字节 
			//int  tsize;
			scanf("%s", temp_file_name);
			if (user_id == 0)
				creat(user_id, temp_file_name, ROOTMODE | GDIREAD | DIFILE);
			else
				creat(user_id, temp_file_name, ROOTMODE | ODIWRITE | ODIREAD | ODIEXICUTE | DIFILE);

			pos_in_sysofile = aopen(user_id, temp_file_name, UDIWRITE);
			//plus1
			struct inode* temp_inode = iget(sys_ofile[pos_in_sysofile].f_inode);
			if (temp_inode->i_ino != 0 && temp_inode->di_size != 0)printf(">file open success!\n");
			//plus2
			char write_buffer[10 * BLOCKSIZ];//文件写入缓冲区,10*BLOCKSIZ因为一个i节点最多指向10个数据块
			memset(write_buffer, 0, sizeof(write_buffer));
			int tfd = xfa(temp_file_name);
			//printf("%d", tfd);
			if (tfd != -1)
			{
				//文件存在且被打开
				int sys_otpos = user[user_id].u_ofile[tfd];					//获取当前文件在系统文件打开表中的位置
				int temp_file_size = sys_ofile[sys_otpos].f_inode->di_size;	//获取当前文件大小
				printf(">Write file [%s],please input:\n", temp_file_name);			//输出提示信息
				scanf("%s", write_buffer);									//获取用户写入
				int ssize = 0;
				while (write_buffer[ssize] != '\0')
					ssize++;
				write(tfd, write_buffer, ssize);				//将文件内容读入缓冲区
			}
			else
				printf(">Cannot write a file that is not open or not exixted in current directory\n");
			close(user_id, tfd);

		}

		/**********************5.关闭已经打开的文件命令***********************/
		else if (strcmp(cmd, "close") == 0)
		{
			//close(unsigned int user_id, unsigned short cfd);
			char temp_file_name[DIRSIZ];//文件名长度14个字节
			scanf("%s", temp_file_name);
			int tet = xfa(temp_file_name);//获取在用户打开表中的位置
			//printf("%d", tet);
			if (tet != -1)
			{//文件存在当前目录且被打开
				close(user_id, tet);
				printf(">File close successful\n");
			}
			else
				printf(">Cannot close a file that is not open or not exixted in current directory\n");
		}
		/**********************6.删除文件命令***********************/
		else if (strcmp(cmd, "delete") == 0)
		{//Feat.CXX
			char tfn[DIRSIZ];//文件名缓冲区
			memset(tfn, 0, sizeof(tfn));
			fgets(tfn, sizeof(tfn), stdin); //获取文件名
			tfn[strcspn(tfn, "\n")] = '\0'; // Remove the newline character from the input
			deletefd(user_id, tfn);
		}
		/**********************7.关机命令***********************/
		else if (strcmp(cmd, "halt") == 0)
		{//Feat.CXX
			chdir(ROOT, "..");	//循环跳转到根目录
			iput(cur_path_inode);	//***
			halt();
			exit_flag = 0;
		}
		/**********************8.退出账户命令***********************/
		else if (strcmp(cmd, "logout") == 0)
		{
			printf(">Log out current user account?(y(es)/n(0))\n");
			if (_getch() == 'y')
				logout(user_id);
			else(">No change\n");
		}
		/**********************9.打开文件命令***********************/
		else if (strcmp(cmd, "aopen") == 0)
		{
			char input_buffer[100]; // 用于存储输入的一行文本
			fgets(input_buffer, sizeof(input_buffer), stdin);
			char temp_file_name[DIRSIZ];//文件名长度14个字节
			char temp_openmode[3];//文件打开方式
			// 从输入的一行文本中提取文件名和打开方式
			sscanf(input_buffer, "%s %s", temp_file_name, temp_openmode);
			//scanf("%s %s", temp_file_name, temp_openmode);//输入文件名及打开方式以空格隔开
			if (strcmp(temp_openmode, "-r") == 0)
				pos_in_sysofile = aopen(user_id, temp_file_name, UDIREAD);
			else if (strcmp(temp_openmode, "-w") == 0)
				pos_in_sysofile = aopen(user_id, temp_file_name, UDIWRITE);
			else if (strcmp(temp_openmode, "-a") == 0)
				pos_in_sysofile = aopen(user_id, temp_file_name, UDIWRITE | UDIEXICUTE | UDIREAD);
			struct inode* temp_inode = iget(sys_ofile[pos_in_sysofile].f_inode);
			if (temp_inode->i_ino != 0 && temp_inode->di_size != 0)printf(">file open success!\n");
		}
		/**********************10.显示帮助命令***********************/
		else if (strcmp(cmd, "help") == 0)
		{
			printf(" $ ls\t\t显示当前目录\n $ mkdir\t创建目录\n $ chdir\t跳转目录\n $ creat\t创建文件\n $ close\t关闭已经打开的文件\n $ delete\t删除文件\n $ halt\t\t关机\n");
			printf(" $ logout\t退出账户\n $ aopen\t打开文件\n $ help\t\t显示帮助\n $ fmt\t\t格式化\n $ rd\t\t读文件\n $ wr\t\t写文件\n $ cls\t\t清屏\n");
		}
			/**********************11.格式化命令***********************/
		else if (strcmp(cmd, "fmt") == 0)
		{
			if (user[user_id].u_uid != ROOT)
			{//当前用户不是rooter
				printf(">Failed to format the disk because of unqualified authority!\n");
				break;
			}
			else
			{//当前用户是rooter可以进行格式化
				chdir(ROOT, "..");	//循环跳转到根目录
				iput(cur_path_inode);	//***
				format();				//格式化
				install();				//装载
			}
		}
		/**********************12.读文件命令***********************/
		else if (strcmp(cmd, "rd") == 0)
		{
			//初始化10个物理块的读入内存
			memset(read_buffer, 0, sizeof(read_buffer));
			//要打开的文件名初始化
			char tfn[DIRSIZ];
			memset(tfn, 0, sizeof(tfn));
			fgets(tfn, sizeof(tfn), stdin); // 获取文件名
			tfn[strcspn(tfn, "\n")] = '\0'; // 去除换行符
			//判断文件是否被打开
			int tfd = xfa(tfn);
			//文件已经打开
			if (tfd != -1)
			{	//获取当前文件在系统文件打开表中的位置
				int sys_otpos = user[user_id].u_ofile[tfd];
				//获取当前文件大小
				int temp_file_size = sys_ofile[sys_otpos].f_inode->di_size;
				//将文件内容读入缓冲区
				read(tfd, read_buffer, temp_file_size);
				int i = 0;
				while (((read_buffer[i] >= '0' && read_buffer[i] <= '9') || (read_buffer[i] >= 'a' && read_buffer[i] <= 'z') || (read_buffer[i] >= 'A' && read_buffer[i] <= 'Z')))
					i++;
				read_buffer[i] = '\0';
				printf("\n>Content of the file: %s\n", read_buffer);
			}
			else {
				printf(">Cannot read a file that is not open or not exixted in current directory\n");
			}
		}		
		/**********************13.写文件命令***********************/
		else if (strcmp(cmd, "wr") == 0)
		{
			//初始化为10个BLOCKSIZ的大小
			char write_buffer[10 * BLOCKSIZ];
			memset(write_buffer, 0, sizeof(write_buffer));
			char tfn[DIRSIZ];//文件名缓冲区
			memset(tfn, 0, sizeof(tfn));

			fgets(tfn, sizeof(tfn), stdin); // 获取文件名
			tfn[strcspn(tfn, "\n")] = '\0'; // 去除换行符

			int tfd = xfa(tfn);
			if (tfd != -1)
			{//文件存在且被打开
				int sys_otpos = user[user_id].u_ofile[tfd];					//获取当前文件在系统文件打开表中的位置
				int temp_file_size = sys_ofile[sys_otpos].f_inode->di_size;	//获取当前文件大小
				printf(">Write file [%s],please input:\n", tfn);			//输出提示信息
				fgets(write_buffer, sizeof(write_buffer), stdin); // 获取用户写入
				write_buffer[strcspn(write_buffer, "\n")] = '\0';							//获取用户写入
				int ssize = 0;
				while (write_buffer[ssize] != '\0')
					ssize++;
				//写入缓冲区
				write(tfd, write_buffer, ssize);
			}
			else {
				printf(">Cannot write a file that is not open or not exixted in current directory\n");
			}
		}
		/**********************14清屏命令***********************/
		else if (strcmp(cmd, "cls") == 0)
			system("cls");
			/**********************15复制命令***********************/
		else if (strcmp(cmd, "copy") == 0)
		{
			char tfn[DIRSIZ];//文件名缓冲区
			memset(cname, 0, sizeof(cname));
			fgets(cname, sizeof(cname), stdin); // 获取文件名
			cname[strcspn(cname, "\n")] = '\0'; // 去除换行符
			//strcpy(cname, tfn);
			pos_in_sysofile = aopen(user_id, cname, UDIREAD);
			struct inode* temp_inode = iget(sys_ofile[pos_in_sysofile].f_inode);
			cpy(cname, cbuf);
			printf("%s", cbuf);
		}
		/************************16粘贴****************************/
		else if (strcmp(cmd, "pst") == 0)
		{
			printf("%s", cname);
			creat(user_id, cname, ROOTMODE | GDIREAD | DIFILE); //文件权限和类型 权限为ROOTMODE(创建者rwx) GDIREAD（同组可读r--）
			pos_in_sysofile = aopen(user_id, cname, UDIWRITE);
			struct inode* temp_inode = iget(sys_ofile[pos_in_sysofile].f_inode);
			if (temp_inode->i_ino != 0 && temp_inode->di_size != 0)printf(">file open success!\n");
			pst(cname, cbuf);
			}
			/**************************17创建用户*************************/
		else if (strcmp(cmd, "cruser") == 0)
		{
			int i;
			if (strcmp(login_name, "root") != 0) {
				printf("\nYou dont have the right to create a role!\n");
				continue;
			}
			char uname[14];
			while (1) {
				//输入用户名
				printf("\nplease scanf user id:");
				memset(uname, 0, sizeof(uname));
				fgets(uname, sizeof(uname), stdin);
				uname[strcspn(uname, "\n")] = '\0'; // 去除换行符
				//判断是否有相同的用户
				for (i = 0; i < PWDNUM; i++)
				{
					if (strcmp(uname, pwd[i].username) == 0) {
						printf("\nThe same role has been created!please input the different name!");
						break;
					}
				}
				if (i == PWDNUM)break;
			}
			//输入用户密码
			printf("\nplease scanf user password:");
			char upword[14];
			memset(upword, 0, sizeof(upword));
			memset(upword, 0, sizeof(upword));
			fgets(upword, sizeof(upword), stdin);
			{
				int i, j;
				int match_flag = 0, ret = 0;
				unsigned short tempuid = 0, tempgid = 0;

				for (i = 0; i < PWDNUM; i++)
				{
					//找到空项
					if (strcmp(" ", pwd[i].username) == 0) {
						strcpy(pwd[i].username, uname);
						strcpy(pwd[i].password, upword);
						pwd[i].p_uid = i;//普通用户从1开始
						pwd[i].p_gid = i;
						break;
					}
				}
				//用户名已经满
				if (i == PWDNUM)
				{
					printf(">please check the number of users!\n");
					printf(">Incorrect username. please retry\n");
					continue;
				}
				fseek(fd, DATASTART + BLOCKSIZ * 2, SEEK_SET);
				fwrite(pwd, BLOCKSIZ, 1, fd);
				//创建含有该用户名的目录
				while (cur_path_inode->i_ino != 1)
				{
					chpath(tprecord, "..", cur_path_inode->i_ino);
					chdir(ROOT, "..");
				}				//循环跳转到根目录
				//对i用户赋予权限
				mkdir_user(i, uname);
			}
		}
		else {
			printf(">Command not found\n");
		}
	} while (exit_flag);
}
    /*
	install();

	printf("\nCommand : dir  \n");
	_dir();

	login(2118, "abcd");
	user_id= 0;
	mkdir("a2118");
	chdir("a2118");
	ab_fd1=creat(2118,"ab_file0.c",01777);
	buf = (char *) malloc(BLOCKSIZ* 6+5);
	awrite(ab_fd1, buf, BLOCKSIZ* 6+5);
	close(user_id, ab_fd1);
	free (buf);

	mkdir("subdir");
	chdir("subdir");
	ab_fd2=creat(2118,"file1.c", 01777);
	buf= (char *) malloc(BLOCKSIZ *4+20);
	awrite (ab_fd2, buf, BLOCKSIZ * 4+20);
	close(user_id, ab_fd2);
	free (buf);

	chdir("..");
	ab_fd3=creat(2118,"_file2.c", 01777);
	buf= (char * ) malloc (BLOCKSIZ * 10+255);
	awrite(ab_fd3, buf, BLOCKSIZ *3+255);
	close(ab_fd3);
	free(buf);

	delete("ab_file0.c");
	ab_fd4=creat(2118, "ab_file3.c", 01777);
	buf=(char * ) malloc (BLOCKSIZ * 8+300);
	write(ab_fd4, buf, BLOCKSIZ *8+300);
	close(ab_fd4);
	free(buf);

	ab_fd3=aopen(2118,"ab_file2.c", FAPPEND);
	buf=(char *)malloc(BLOCKSIZ * 3+100);
	awrite(ab_fd3, buf, BLOCKSIZ* 3+100);
	close(ab_fd3);
	free (buf);

	_dir();
	chdir ("..");
	logout();
	halt();
*/
