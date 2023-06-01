
#include <stdio.h>
#include "filesys.h"
#include <stdlib.h>
#include <conio.h>

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
    int exit_flag = 1;
    char tprecord[1000] = "";
    char cmd[10];
    int user_id = -1;

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
    } while (exit_flag);
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
}