#include "pch.h"
#include <stdio.h>
#include <stdlib.h>
#include <winsock.h>
#include <string.h>
#include "md5_2.h"
#pragma comment (lib,"wsock32.lib")

#define MAX 2048
#define STATU_INIT 0
#define STATU_CHECK 1
#define STATU_UPLOAD 2
#define STATU_DOWNLOAD 3
#define STATU_CHANGEKEY 4
#define STATU_EXIT 5

struct checknum
{
	int a;
};

struct frame
{
	int type;
	bool judge;
	int x;
	char message[200];
};

unsigned char* MD5(char encrypt[],unsigned char *de)
{
	unsigned char decrypt[16];
	de = decrypt;
	MD5_CTX md5;
	MD5Init(&md5);
	MD5Update(&md5,(unsigned char*)encrypt, strlen((char *)encrypt));
	MD5Final(&md5, decrypt);
	return de;
}
int main(int argc, char*argv[])
{
	SOCKET s;
	WSAData wsa;
	sockaddr_in local, remote;
	struct checknum CheckNum[MAX];
	struct frame Frame;
	int retral,r,checksize,checklen;
	int checksum=0;
	int key = 12345;
	char send_buf[MAX];
	char recv_buf[MAX];
	char *se = send_buf;
	char *re = recv_buf;
	struct checknum*c = CheckNum;
	struct frame*f = &Frame;
	FILE *Fid;
	errno_t err;
	int len;

	local.sin_family = AF_INET;
	local.sin_addr.S_un.S_addr = htonl(INADDR_ANY);
	local.sin_port = htons(0x3412);

	remote.sin_family = AF_INET;
	remote.sin_addr.S_un.S_addr = inet_addr("192.168.1.106");
	remote.sin_port = htons(0x1234);

	WSAStartup(0x101, &wsa);
	s = socket(AF_INET, SOCK_STREAM, 0);
	bind(s, (sockaddr*)&local, sizeof(local));

	do
	{
		retral = connect(s, (sockaddr*)&remote, sizeof(remote));
		if (retral == SOCKET_ERROR)
		{
			r = WSAGetLastError();
			printf_s("Error: %d\n", &r);
			closesocket(s);
		}
	} while (retral != 0);
	
	recv(s, re, MAX, 0);
	memset(recv_buf, 0, MAX);
	checksize = recv(s, (char*)c,sizeof(CheckNum), 0);
	
	do 
	{
		char Username[8];
		printf("Please enter your Username:\n ");
		scanf_s("%s", &Username, 7);
		printf("Please enter your password:\n");
		scanf_s("%d", &key, sizeof(int));
		int XorResult;
		for (int i = 0; i < checksize / sizeof(checknum); i++)
			checksum += CheckNum[i].a;
		XorResult = checksum ^ key;
		Frame.type = 11;
		Frame.x = XorResult;
		memcpy(Frame.message, Username, strlen(Username));
		send(s, (char*)f, sizeof(frame), 0);
		memset(&Frame, 0, sizeof(frame));
		recv(s, (char*)f, sizeof(frame), 0);
		if (Frame.type == 2 && Frame.judge == TRUE)
		{
			printf("Login the server successfully.\n");
			memset(&Frame, 0, sizeof(frame));
			break;
		}
		else
		{
			printf("Login the server failed, please enter the key again.\n");
			memset(&Frame, 0, sizeof(frame));
		}
	} while (1);

	int next_statu;
	next_statu = STATU_INIT;
	int flag = 1;
	while (flag)
	{
		switch (next_statu)
		{
		case STATU_INIT:
		{
			printf_s("Please choice the service you want:\n");
			scanf_s("%d", &next_statu);
			break; 
		}
		case STATU_CHECK:
		{
			Frame.type = 12;
			send(s, (char*)f, sizeof(frame), 0);
			memset(&Frame, 0, sizeof(frame));
			retral = recv(s, re, MAX, 0);
			printf("The files on the server:\n");
			int i = 0;
			while (i<retral)
			{
				if (recv_buf[i] != ' ')
				{
					printf("%c", recv_buf[i]);
					i++;
				}
				else
				{
					printf("\n");
					i++;
				}
			}
			printf("\n");
			next_statu = STATU_INIT;
			break;
		}
		case STATU_UPLOAD:
		{
			while (1)
			{
				Frame.type = 15;
				char filename[20];
				printf("Please enter the file you want upload:\n");
				scanf_s("%s", &filename, 19);
				err = fopen_s(&Fid, filename, "ab+");
				if (err != 0)
				{
					printf("File open failed, please try again.\n");
					break;
				}
				else
				{
					fseek(Fid, 0, SEEK_END);
					len = ftell(Fid);
					fseek(Fid, 0, SEEK_SET);
					strcpy_s(Frame.message, sizeof(Frame.message), filename);
					Frame.x = len;
					send(s, (char*)f, sizeof(frame), 0);
					memset(&Frame, 0, sizeof(frame));
					recv(s, (char*)f, sizeof(frame), 0);
					if (Frame.judge == TRUE)
					{
						memset(&Frame, 0, sizeof(frame));
						break;
					}
					else
					{
						memset(&Frame, 0, sizeof(frame));
						printf("Invaild Filename.\n");
					}
				}

			}
			if (len <= MAX)
			{
				fread_s(send_buf, MAX, 1, len, Fid);
				unsigned char md5[16];
				unsigned char *m = md5;
				MD5(send_buf,m);
				send(s, se, strlen(send_buf), 0);
				memset(send_buf, 0, MAX);
				recv(s, re, MAX, 0);
				if (!strcmp((char*)md5, recv_buf))
				{
					printf("Successfully.\n");
				}
			}
			else
			{
				int epoch = len / 2048;
				int lastepoch = len % 2048;
				for (int i = 1; i <= epoch; i++)
				{
					fread_s(send_buf, MAX, 1, MAX, Fid);
					send(s, se, strlen(send_buf), 0);
					memset(send_buf, 0, MAX);
				}
				fread_s(send_buf, MAX, 1, lastepoch, Fid);
				send(s, se, strlen(send_buf), 0);
				memset(send_buf, 0, MAX);
			}
			fclose(Fid);
			next_statu = STATU_INIT;
			break;
		}
		case STATU_DOWNLOAD:
		{
			while (1)
			{
				char filename[20];
				printf("Please enter the filename you want download:\n");
				scanf_s("%s", &filename, 19);
				Frame.type = 13;
				strcpy_s(Frame.message, sizeof(Frame.message), filename);
				send(s, (char*)f, sizeof(frame), 0);
				memset(&Frame, 0, sizeof(frame));
				recv(s, (char*)f, sizeof(frame), 0);
				if (Frame.type == 4 && Frame.judge == TRUE)
				{
					err = fopen_s(&Fid, filename, "ab+");
					len = Frame.x;
					if (len <= MAX)
					{
						recv(s, re, len, 0);
						fwrite(recv_buf, 1, len, Fid);
						memset(recv_buf, 0, MAX);
						fclose(Fid);
						next_statu = STATU_INIT;
						break;
					}
					else
					{
						int epoch = len / MAX;
						printf("epoch:%d\n", epoch);
						int lastpeoch = len % MAX;
						printf("lastepoch:%d\n", lastpeoch);
						for (int i = 0; i < epoch; i++)
						{
							recv(s, re, MAX, 0);
							fwrite(recv_buf, 1, MAX, Fid);
							memset(recv_buf, 0, MAX);
						}
						recv(s, re, lastpeoch, 0);
						fwrite(recv_buf, 1, lastpeoch, Fid);
						memset(recv_buf, 0, MAX);
						fclose(Fid);
						next_statu = STATU_INIT;
						break;
					}
				}
				else
				{
					printf("Error.\n");
					memset(&Frame, 0, sizeof(frame));
				}
			}
			break;
		}
		case STATU_CHANGEKEY:
		{
			int newkey;
			printf("Please enter the new password.\n");
			scanf_s("%d", &newkey);
			Frame.type = 17;
			Frame.x = newkey;
			send(s, (char*)f, sizeof(frame), 0);
			memset(&Frame, 0, sizeof(frame));
			next_statu = STATU_INIT;
			break;
		}
		case STATU_EXIT:
			Frame.type = 18;
			send(s, (char*)f, sizeof(frame), 0);
			shutdown(s, 1);
			memset(&Frame, 0, sizeof(frame));
			recv(s, (char*)f, sizeof(frame), 0);
			if (Frame.type == 8 && Frame.judge == TRUE)
			{
				flag = FALSE;
				shutdown(s, 0);
				memset(&Frame, 0, sizeof(frame));
			}
			break;
		default:
			break;
		}
	}
}
