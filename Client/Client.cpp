#include "pch.h"
#include <stdio.h>
#include <stdlib.h>
#include <winsock.h>
#include <string.h>
#pragma comment (lib,"wsock32.lib")

#define MAX 20000
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
int main(int argc, char*argv[])
{
	SOCKET s;
	WSAData wsa;
	sockaddr_in local, remote;
	struct checknum CheckNum[MAX];
	struct frame Frame;
	struct frame Frames[10];
	int retral,r,checksize,checklen;
	int checksum=0;
	int key = 12345;
	char send_buf[MAX];
	char recv_buf[MAX];
	char *se = send_buf;
	char *re = recv_buf;
	struct checknum*c = CheckNum;
	struct frame*f = &Frame;
	struct frame*fs = Frames;

	local.sin_family = AF_INET;
	local.sin_addr.S_un.S_addr = htonl(INADDR_ANY);
	local.sin_port = htons(0x3412);

	remote.sin_family = AF_INET;
	remote.sin_addr.S_un.S_addr = htonl(INADDR_LOOPBACK);
	remote.sin_port = htons(0x7856);

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

	/*checksize = recv(s, (char*)c,sizeof(CheckNum), 0);
	
	do 
	{
		int XorResult;
		for (int i = 0; i < checksize / sizeof(checknum); i++)
			checksum += CheckNum[i].a;
		XorResult = checksum ^ key;
		Frame.type = 11;
		Frame.x = XorResult;
		send(s, (char*)f, sizeof(frame), 0);
		memset(&Frame, 0, sizeof(frame));
		recv(s, (char*)f, sizeof(frame), 0);
		if (Frame.type == 2 && Frame.judge == TRUE)
		{
			printf("Login The Server Successfully.\n");
			memset(&Frame, 0, sizeof(frame));
		}
		else
		{
			printf("Login The Server Failed, Please Enter The Key Again.\n");
			memset(&Frame, 0, sizeof(frame));
		}
	} while (!(Frame.type == 2 && Frame.judge == TRUE));*/

	int next_statu;
	next_statu = STATU_INIT;
	int flag = 1;
	while (flag)
	{
		switch (next_statu)
		{
		case STATU_INIT:
			printf_s("Please Choice the service you want:\n");
			scanf_s("%d",&next_statu);
			break;
		case STATU_CHECK:
			Frame.type = 12;
			send(s, (char*)f, sizeof(frame), 0);
			memset(&Frame, 0, sizeof(frame));
			retral=recv(s, (char*)fs, sizeof(Frames), 0);
			printf("The Files On The Server:\n");
			for (int i = 0; i < retral/sizeof(frame); i++)
			{
				printf("%s", &Frames[i].message);
				printf("\n");
			}
			memset(Frames, 0,sizeof(Frames));
			break;
		case STATU_UPLOAD:
			FILE *Fid;
			errno_t err;
			int len;
			fseek(Fid, 0, SEEK_END);
			len = ftell(Fid);
			fseek(Fid, 0, SEEK_SET);
			while (1)
			{
				Frame.type = 15;
				char filename[20];
				printf("Please Enter The File You Want Upload:\n");
				scanf_s("%s", &filename,19);
				err = fopen_s(&Fid, filename, "ab+");
				if (err == 0)
				{
					printf("File Open Failed, Please Try Again.\n");
					break;
				}
				else
				{
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
				send(s, se, strlen(send_buf), 0);
				memset(send_buf, 0, MAX);
			}
			else
			{
				int epoch = len / 20000;
				int lastepoch = len % 20000;
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
		case STATU_DOWNLOAD:
			FILE*Fid;
			errno_t err;
			int len = 0;
			while (1)
			{
				char filename[20];
				printf("Please Enter The Filename You Want Download:\n");
				scanf_s("%s", &filename, 19);
				Frame.type = 13;
				strcpy_s(Frame.message, sizeof(Frame.message), filename);
				send(s, (char*)f, sizeof(frame), 0);
				memset(&Frame, 0, sizeof(frame));
				recv(s, (char*)f, sizeof(frame), 0);
				if (Frame.type == 4 && Frame.judge == TRUE)
				{

				}

			}
			scanf("%s", &send_buf);
			err = fopen_s(&Fid, send_buf, "wb+");
			send(s, re, strlen(send_buf), 0);
			memset(send_buf, 0, MAX);
			recv(s, recv_buf, MAX, 0);
			if (recv_buf[0] == 'N')
			{
				printf("Wrong Filename, Please Enter Again\n");
				memset(recv_buf, 0, MAX);
			}
			else if (recv_buf[0] == 'Y')
			{
				memset(recv_buf, 0, MAX);
				break;
			}
			recv(s, (char*)p,MAX, 0);
			if (CheckNum[0].a != NULL)
				len = CheckNum[0].a;
			else
				len = CheckNum[0].b;
			if (len < MAX)
			{
				recv(s, re, MAX, 0);
				fwrite(recv_buf, 1, len, Fid);
				memset(recv_buf, 0, MAX);
			}
			else
			{
				int epoch = len / MAX;
				int lastepoch = len % MAX;
				for (int i = 1; i <= epoch; i++)
				{
					recv(s, re, MAX, 0);
					fwrite(recv_buf, 1, MAX, Fid);
					memset(recv_buf, 0, MAX);
				}
				recv(s, re, lastepoch, 0);
				fwrite(recv_buf, 1, lastepoch, Fid);
				memset(recv_buf, 0, MAX);
			}
			fclose(Fid);
			
		/*case STATU_CHANGEKEY:
			send_buf[0] = 'C';
			send(s, se, strlen(send_buf), 0);
			memset(send_buf, 0, MAX);
			printf("Please enter the new key you want:\n");
			scanf("%s", &send_buf);
			send(s, se, strlen(send_buf), 0);
			memset(send_buf, 0, MAX);
			break;

		case STATU_EXIT:
			flag = FALSE;
			break;
		default:
			break;*/
		}
	}
}
