
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
                // Call format() function here
                printf("\n> System is being formatted now.\n");
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