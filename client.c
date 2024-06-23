#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#include <windows.h>

#pragma comment(lib, "ws2_32.lib")

#define PORT 8080

DWORD WINAPI receive_messages(LPVOID sock_desc) {
    SOCKET sock = *(SOCKET*)sock_desc;
    char server_reply[2000];
    int read_size;

    while ((read_size = recv(sock, server_reply, sizeof(server_reply), 0)) > 0) {
        server_reply[read_size] = '\0';
        printf("%s\n", server_reply);
    }

    if (read_size == 0) {
        printf("���� ����\n");
    }
    else if (read_size == SOCKET_ERROR) {
        printf("����: % d\n", WSAGetLastError());
    }

    return 0;
}

int main() {
    SetConsoleTitle(TEXT("��ī����")); //�ܼ� �̸�  

    WSADATA wsa;
    SOCKET sock;
    struct sockaddr_in server;
    char message[1000];
    char name[50];

    // Winsock �ʱ�ȭ
    printf("Winsock�� ����� �� �ֵ��� �ʱ�ȭ ���Դϴ�...\n");
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        printf("����. Error Code: %d\n", WSAGetLastError());
        return 1;
    }
    printf("Winsock�� ����� �� �ֽ��ϴ�.\n");

    // ���� ����
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET) {
        printf("Could not create socket : %d\n", WSAGetLastError());
        WSACleanup();
        return 1;
    }
    printf("���� ���� �Ϸ�.\n");

    // ���� �ּ� ����
    server.sin_family = AF_INET;
    server.sin_port = htons(PORT);
    if (inet_pton(AF_INET, "127.0.0.1", &server.sin_addr) <= 0) {
        printf("��ȿ�� �ּҰ��ƴմϴ�\n");
        closesocket(sock);
        WSACleanup();
        return 1;
    }

    // ������ ����
    if (connect(sock, (struct sockaddr*)&server, sizeof(server)) < 0) {
        printf("���� ����: %d\n", WSAGetLastError());
        closesocket(sock);
        WSACleanup();
        return 1;
    }
    printf("������ ����Ǿ����ϴ�.\n");

    // ����� �̸� �Է�
    printf("����� �̸��� �Է����ּ���: ");
    fgets(name, sizeof(name), stdin);
    name[strcspn(name, "\n")] = '\0';

    // ������ �̸� ����
    if (send(sock, name, strlen(name), 0) < 0) {
        printf("���� ����: %d\n");
        closesocket(sock);
        WSACleanup();
        return 1;
    }

    // �����κ��� �޽��� ������ ���� ������ ����
    HANDLE recv_thread = CreateThread(NULL, 0, receive_messages, (void*)&sock, 0, NULL);
    if (recv_thread == NULL) {
        printf("������ ����: %d\n", GetLastError());
        closesocket(sock);
        WSACleanup();
        return 1;
    }

    // �޽��� �۽�
    while (1) {
        printf("�޽��� �Է�(�������� exit) ");
        fgets(message, sizeof(message), stdin);
        message[strcspn(message, "\n")] = '\0';

        // 'exit' �Է� �� ����
        if (strcmp(message, "exit") == 0) {
            break;
        }

        // ��ɾ� ó��(����,����,����,�Ϲ� �޽���)
        if (strncmp(message, "modify ", 7) == 0) {
            send(sock, message, strlen(message), 0);
        }
        else if (strncmp(message, "delete ", 7) == 0) {
            send(sock, message, strlen(message), 0);
        }
        else if (strncmp(message, "restore ", 8) == 0) {
            send(sock, message, strlen(message), 0);
        }
        else {
            // �Ϲ� �޽��� ����
            char full_message[1050];
            snprintf(full_message, sizeof(full_message), "%s: %s", name, message);
            if (send(sock, full_message, strlen(full_message), 0) < 0) {
                printf("���� ����: %d\n", WSAGetLastError());
                break;
            }
        }
    }

    // ���� �ݱ� �� Winsock ����
    closesocket(sock);
    WSACleanup();
    return 0;
}