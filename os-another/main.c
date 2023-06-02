/**
*		    �޸�:	1. getch��ʽ������
*					2. �����Ϣ����
					3. ���ƿ���ע���¼����
					4. ����ж�ִ�и�ʽ��Ȩ�޵Ĺ���
					5. ��½ʱ���������Ŀ¼�Ĺ����޸�
					6. ��ʽ��fmt�����Ĵ������
					7. fmt ������ ����ת����Ŀ¼ Ȼ���ͷ�cur_path_inode
					8. creat���������޸Ĵ����ı仯 ***
					9. �޸Ĵ��ļ�ģ���е��߼����ж� �Լ���������***
					10. ���ӱ���int pos_in_sysofile;�����ļ���д�ʹ���
*/
#include <stdio.h>
#include "filesys.h"
#include <stdlib.h>
#include <conio.h>

//�������ڴ��i�ڵ����Ͱ�У���ϣ����dino % NHINO��һ��512��i�ڵ㣬NHINO=128
//����Ͱ�м���ֱ�ӵ��ã������ٽ��д����е�IO����
struct hinodei hinode[NHINO];
//�ڴ��е�Ŀ¼
struct diri dir;
//ϵͳ�򿪱�
struct file sys_ofile[SYSOPENFILE];
//�ļ�������
struct filsysi filsys;
//�û������֣�ռ��28�ֽڣ�λ�ڴ���2���������
struct pwdi pwd[PWDNUM];
//�û����ļ���
struct useri user[USERNUM];
//���̶�дͷ
FILE* fd;
//��ǰĿ¼i�ڵ�
struct inode* cur_path_inode;
//��ǰ�û����û�id����¼�û����û��������б��е�λ��
int user_id;
//ϵͳ�򿪱��λ��
int pos_in_sysofile;
//�ļ���������������Ŀ¼i�ڵ㻹���ļ�i�ڵ㣬���ֻ����10�������
char read_buffer[10 * BLOCKSIZ] = { 0 };
//���ƻ�����
char cbuf[512] = { 0 };
//�����ļ���
char cname[14] = { 0 };

void main()
{
	char login_name[PWDSIZ], login_password[PWDSIZ];
	int  exit_flag = 1;
	char tprecord[1000] = "";
	char cmd[10];
	user_id = -1;

	printf("Unix File System VER1.0[�汾1.0 2023-5-25]\n");
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
				format();	//format��
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
			memset(login_password, 0, sizeof(login_password));		//��ʼ��login_id����������0��������
			memset(login_password, 0, sizeof(login_password));		//��ʼ��login_password����������0��������
			printf("\nPlease enter your account to log in \n");
			printf("\tUSERNAME:   ");
			scanf("%s", login_name);								//�����û�ID��login_name������
			printf("\tPASSWORD:   ");
			char pwdt;
			for (int i = 0; (i < PWDSIZ) && ((pwdt = _getch()) != '\r'); i++)
				login_password[i] = pwdt;							//�������뵽login_name������
			putchar('\n');
			//����û���18���û������б��е�λ��
			//Ĭ�ϴ���Ŀ¼�ļ���
			user_id = login(login_name, login_password);				//�û���¼
			if (user_id != -1)
			{
				//��½�ɹ�
				//ע��root��idΪ1�������û���id��2��ʼ�����û��ļ�����±��0��ʼ
				if (user[user_id].u_uid != ROOT)
				{
					//���root��ת����Ӧ���û�Ŀ¼��
					//�����û�ʱ������root���½�Ŀ¼
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
		//fgets(cmd, sizeof(cmd), stdin); // ��ȡ���������
		scanf("%s", cmd);//��ȡ���������


		/**********************1.��ʾ��ǰĿ¼����***********************/
		if (strcmp(cmd, "ls") == 0)
			_dir();
		/**********************2.����Ŀ¼����**************************/
		else if (strcmp(cmd, "mkdir") == 0)
		{

			char temp_dir_name[DIRSIZ];
			scanf("%s", temp_dir_name);
			if (user_id == 0)
				mkdir(user_id, temp_dir_name);
			else
				mkdir_user(user_id, temp_dir_name);
		}
		/**********************3.��תĿ¼����**************************/
		else if (strcmp(cmd, "cd") == 0)
		{
			int success_flag = 0;
			char temp_dir_name[DIRSIZ];
			scanf("%s", temp_dir_name);
			success_flag = chdir(0, temp_dir_name);
			if (success_flag == 1)//�����ת�ɹ��޸Ĵ����������·��
				chpath(tprecord, temp_dir_name, cur_path_inode->i_ino);
			else
				printf(">Failed to change directory.Please retry\n");
		}
		/**********************4.�����ļ�����***********************/
		else if (strcmp(cmd, "creat") == 0)
		{
			char temp_file_name[DIRSIZ];//�ļ�������14���ֽ� 
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
			char write_buffer[10 * BLOCKSIZ];//�ļ�д�뻺����,10*BLOCKSIZ��Ϊһ��i�ڵ����ָ��10�����ݿ�
			memset(write_buffer, 0, sizeof(write_buffer));
			int tfd = xfa(temp_file_name);
			//printf("%d", tfd);
			if (tfd != -1)
			{
				//�ļ������ұ���
				int sys_otpos = user[user_id].u_ofile[tfd];					//��ȡ��ǰ�ļ���ϵͳ�ļ��򿪱��е�λ��
				int temp_file_size = sys_ofile[sys_otpos].f_inode->di_size;	//��ȡ��ǰ�ļ���С
				printf(">Write file [%s],please input:\n", temp_file_name);			//�����ʾ��Ϣ
				scanf("%s", write_buffer);									//��ȡ�û�д��
				int ssize = 0;
				while (write_buffer[ssize] != '\0')
					ssize++;
				write(tfd, write_buffer, ssize);				//���ļ����ݶ��뻺����
			}
			else
				printf(">Cannot write a file that is not open or not exixted in current directory\n");
			close(user_id, tfd);

		}

		/**********************5.�ر��Ѿ��򿪵��ļ�����***********************/
		else if (strcmp(cmd, "close") == 0)
		{
			//close(unsigned int user_id, unsigned short cfd);
			char temp_file_name[DIRSIZ];//�ļ�������14���ֽ�
			scanf("%s", temp_file_name);
			int tet = xfa(temp_file_name);//��ȡ���û��򿪱��е�λ��
			//printf("%d", tet);
			if (tet != -1)
			{//�ļ����ڵ�ǰĿ¼�ұ���
				close(user_id, tet);
				printf(">File close successful\n");
			}
			else
				printf(">Cannot close a file that is not open or not exixted in current directory\n");
		}
		/**********************6.ɾ���ļ�����***********************/
		else if (strcmp(cmd, "delete") == 0)
		{//Feat.CXX
			char tfn[DIRSIZ];//�ļ���������
			memset(tfn, 0, sizeof(tfn));
			scanf("%s", tfn);//��ȡ�ļ���
			deletefd(user_id, tfn);
		}
		/**********************7.�ػ�����***********************/
		else if (strcmp(cmd, "halt") == 0)
		{//Feat.CXX
			chdir(ROOT, "..");	//ѭ����ת����Ŀ¼
			iput(cur_path_inode);	//***
			halt();
			exit_flag = 0;
		}
		/**********************8.�˳��˻�����***********************/
		else if (strcmp(cmd, "logout") == 0)
		{
			printf(">Log out current user account?(y(es)/n(0))\n");
			if (_getch() == 'y')
				logout(user_id);
			else(">No change\n");
		}
		/**********************9.���ļ�����***********************/
		else if (strcmp(cmd, "aopen") == 0)
		{
			char temp_file_name[DIRSIZ];//�ļ�������14���ֽ�
			char temp_openmode[3];//�ļ��򿪷�ʽ
			scanf("%s %s", temp_file_name, temp_openmode);//�����ļ������򿪷�ʽ�Կո����
			if (strcmp(temp_openmode, "-r") == 0)
				pos_in_sysofile = aopen(user_id, temp_file_name, UDIREAD);
			else if (strcmp(temp_openmode, "-w") == 0)
				pos_in_sysofile = aopen(user_id, temp_file_name, UDIWRITE);
			else if (strcmp(temp_openmode, "-a") == 0)
				pos_in_sysofile = aopen(user_id, temp_file_name, FAPPEND | UDIEXICUTE | UDIREAD);
			struct inode* temp_inode = iget(sys_ofile[pos_in_sysofile].f_inode);
			if (temp_inode->i_ino != 0 && temp_inode->di_size != 0)printf(">file open success!\n");
		}
		/**********************10.��ʾ��������***********************/
		else if (strcmp(cmd, "help") == 0)
		{
			printf(" $ ls\t\t��ʾ��ǰĿ¼\n $ mkdir\t����Ŀ¼\n $ cd\t\t��תĿ¼\n $ creat\t�����ļ�\n $ cruser\t�����û�\n $ close\t�ر��Ѿ��򿪵��ļ�\n $ delete\tɾ���ļ�\n $ halt\t\t�ػ�\n");
			printf(" $ copy\t\t����\n $ pst\t\tճ��\n $ logout\t�˳��˻�\n $ aopen\t���ļ�\n $ help\t\t��ʾ����\n $ fmt\t\t��ʽ��\n $ rd\t\t���ļ�\n $ wr\t\tд�ļ�\n $ rmdir\tɾ��Ŀ¼\n $ cls\t\t����\n");

		}
		/**********************11.��ʽ������***********************/
		else if (strcmp(cmd, "fmt") == 0)
		{
			if (user[user_id].u_uid != ROOT)
			{//��ǰ�û�����rooter
				printf(">Failed to format the disk because of unqualified authority!\n");
				break;
			}
			else
			{//��ǰ�û���rooter���Խ��и�ʽ��
				chdir(ROOT, "..");	//ѭ����ת����Ŀ¼
				iput(cur_path_inode);	//***
				format();				//��ʽ��
				install();				//װ��
			}
		}
		/**********************12.���ļ�����***********************/
		else if (strcmp(cmd, "rd") == 0)
		{
			//��ʼ��10�������Ķ����ڴ�
			memset(read_buffer, 0, sizeof(read_buffer));
			//Ҫ�򿪵��ļ�����ʼ��
			char tfn[DIRSIZ];
			memset(tfn, 0, sizeof(tfn));
			scanf("%s", tfn);//��ȡ�ļ���
			//�ж��ļ��Ƿ񱻴�
			int tfd = xfa(tfn);
			//�ļ��Ѿ���
			if (tfd != -1)
			{	//��ȡ��ǰ�ļ���ϵͳ�ļ��򿪱��е�λ��
				int sys_otpos = user[user_id].u_ofile[tfd];
				//��ȡ��ǰ�ļ���С
				int temp_file_size = sys_ofile[sys_otpos].f_inode->di_size;
				//���ļ����ݶ��뻺����
				read(tfd, read_buffer, temp_file_size);
				int i = 0;
				while (((read_buffer[i] >= '0' && read_buffer[i] <= '9') || (read_buffer[i] >= 'a' && read_buffer[i] <= 'z') || (read_buffer[i] >= 'A' && read_buffer[i] <= 'Z')))
					i++;
				read_buffer[i] = '\0';
				printf("\n>Content of the file: %s\n", read_buffer);
			}
			else
				printf(">Cannot read a file that is not open or not exixted in current directory\n");
		}
		/**********************13.д�ļ�����***********************/
		else if (strcmp(cmd, "wr") == 0)
		{
			//��ʼ��Ϊ10��BLOCKSIZ�Ĵ�С
			char write_buffer[10 * BLOCKSIZ];
			memset(write_buffer, 0, sizeof(write_buffer));
			char tfn[DIRSIZ];//�ļ���������
			memset(tfn, 0, sizeof(tfn));
			scanf("%s", tfn);//��ȡ�ļ���
			int tfd = xfa(tfn);
			if (tfd != -1)
			{//�ļ������ұ���
				int sys_otpos = user[user_id].u_ofile[tfd];					//��ȡ��ǰ�ļ���ϵͳ�ļ��򿪱��е�λ��
				int temp_file_size = sys_ofile[sys_otpos].f_inode->di_size;	//��ȡ��ǰ�ļ���С
				printf(">Write file [%s],please input:\n", tfn);			//�����ʾ��Ϣ
				scanf("%s", write_buffer);									//��ȡ�û�д��
				int ssize = 0;
				while (write_buffer[ssize] != '\0')
					ssize++;
				//д�뻺����
				write(tfd, write_buffer, ssize);
			}
			else {
				printf(">Cannot write a file that is not open or not exixted in current directory\n");
			}
		}
		/**********************14��������***********************/
		else if (strcmp(cmd, "cls") == 0)
			system("cls");
		/**********************15��������***********************/
		else if (strcmp(cmd, "copy") == 0)
		{
			char tfn[DIRSIZ];//�ļ���������
			memset(cname, 0, sizeof(cname));
			scanf("%s", cname);//��ȡ�ļ���
			//strcpy(cname, tfn);
			pos_in_sysofile = aopen(user_id, cname, UDIREAD);
			struct inode* temp_inode = iget(sys_ofile[pos_in_sysofile].f_inode);
			cpy(cname, cbuf);
		}
		/************************16ճ��****************************/
		else if (strcmp(cmd, "pst") == 0)
		{
			printf("%s", cname);
			creat(user_id, cname, ROOTMODE | GDIREAD | DIFILE); //�ļ�Ȩ�޺����� Ȩ��ΪROOTMODE(������rwx) GDIREAD��ͬ��ɶ�r--��
			pos_in_sysofile = aopen(user_id, cname, UDIWRITE);
			struct inode* temp_inode = iget(sys_ofile[pos_in_sysofile].f_inode);
			if (temp_inode->i_ino != 0 && temp_inode->di_size != 0)printf(">file open success!\n");
			pst(cname, cbuf);
		}
		/**************************17�����û�*************************/
		else if (strcmp(cmd, "cruser") == 0)
		{
			int i;
			if (strcmp(login_name, "root") != 0) {
				printf("\nYou dont have the right to create a role!\n");
				continue;
			}
			char uname[14];
			while (1) {
				//�����û���
				printf("\nplease scanf user id:");
				memset(uname, 0, sizeof(uname));
				scanf("%s", uname);
				//�ж��Ƿ�����ͬ���û�
				for (i = 0; i < PWDNUM; i++)
				{
					if (strcmp(uname, pwd[i].username) == 0) {
						printf("\nThe same role has been created!please input the different name!");
						break;
					}
				}
				if (i == PWDNUM)break;
			}
			//�����û�����
			printf("\nplease scanf user password:");
			char upword[14];
			memset(upword, 0, sizeof(upword));
			scanf("%s", upword);
			{
				int i, j;
				int match_flag = 0, ret = 0;
				unsigned short tempuid = 0, tempgid = 0;

				for (i = 0; i < PWDNUM; i++)
				{
					//�ҵ�����
					if (strcmp(" ", pwd[i].username) == 0) {
						strcpy(pwd[i].username, uname);
						strcpy(pwd[i].password, upword);
						pwd[i].p_uid = i;//��ͨ�û���1��ʼ
						pwd[i].p_gid = i;
						break;
					}
				}
				//�û����Ѿ���
				if (i == PWDNUM)
				{
					printf(">please check the number of users!\n");
					printf(">Incorrect username. please retry\n");
					continue;
				}
				fseek(fd, DATASTART + BLOCKSIZ * 2, SEEK_SET);
				fwrite(pwd, BLOCKSIZ, 1, fd);
				//�������и��û�����Ŀ¼
				while (cur_path_inode->i_ino != 1)
				{
					chpath(tprecord, "..", cur_path_inode->i_ino);
					chdir(ROOT, "..");
				}				//ѭ����ת����Ŀ¼
				//��i�û�����Ȩ��
				mkdir_user(i, uname);
			}
		}
		/**************************18rmdir*************************/
		else if (strcmp(cmd, "rmdir") == 0) {
			char temp_dir_name[DIRSIZ];//�ļ�������14���ֽ� 
			memset(temp_dir_name, 0, sizeof(temp_dir_name));
			//int  tsize;
			scanf("%s", temp_dir_name);
			if (strcmp(temp_dir_name, ".") == 0 || strcmp(temp_dir_name, "..") == 0)
			{
				printf(">Operation not permitted\n");
			}
			else {
				int success_flag = 0;
				success_flag = chdir(0, temp_dir_name);
				if (success_flag == 1)//�����ת�ɹ��޸Ĵ����������·��
					rmdir(user_id, temp_dir_name);
				else
					printf(">Failed to change directory.Please retry\n");
			
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
