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
        printf("서버 해제\n");
    }
    else if (read_size == SOCKET_ERROR) {
        printf("에러: % d\n", WSAGetLastError());
    }

    return 0;
}

int main() {
    SetConsoleTitle(TEXT("현카오톡")); //콘솔 이름  

    WSADATA wsa;
    SOCKET sock;
    struct sockaddr_in server;
    char message[1000];
    char name[50];

    // Winsock 초기화
    printf("Winsock을 사용할 수 있도록 초기화 중입니다...\n");
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        printf("실패. Error Code: %d\n", WSAGetLastError());
        return 1;
    }
    printf("Winsock을 사용할 수 있습니다.\n");

    // 소켓 생성
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET) {
        printf("Could not create socket : %d\n", WSAGetLastError());
        WSACleanup();
        return 1;
    }
    printf("소켓 생성 완료.\n");

    // 서버 주소 설정
    server.sin_family = AF_INET;
    server.sin_port = htons(PORT);
    if (inet_pton(AF_INET, "127.0.0.1", &server.sin_addr) <= 0) {
        printf("유효한 주소가아닙니다\n");
        closesocket(sock);
        WSACleanup();
        return 1;
    }

    // 서버에 연결
    if (connect(sock, (struct sockaddr*)&server, sizeof(server)) < 0) {
        printf("연결 오류: %d\n", WSAGetLastError());
        closesocket(sock);
        WSACleanup();
        return 1;
    }
    printf("서버와 연결되었습니다.\n");

    // 사용자 이름 입력
    printf("사용자 이름을 입력해주세요: ");
    fgets(name, sizeof(name), stdin);
    name[strcspn(name, "\n")] = '\0';

    // 서버에 이름 전송
    if (send(sock, name, strlen(name), 0) < 0) {
        printf("전송 실패: %d\n");
        closesocket(sock);
        WSACleanup();
        return 1;
    }

    // 서버로부터 메시지 수신을 위한 쓰레드 생성
    HANDLE recv_thread = CreateThread(NULL, 0, receive_messages, (void*)&sock, 0, NULL);
    if (recv_thread == NULL) {
        printf("쓰레드 오류: %d\n", GetLastError());
        closesocket(sock);
        WSACleanup();
        return 1;
    }

    // 메시지 송신
    while (1) {
        printf("메시지 입력(나가려면 exit) ");
        fgets(message, sizeof(message), stdin);
        message[strcspn(message, "\n")] = '\0';

        // 'exit' 입력 시 종료
        if (strcmp(message, "exit") == 0) {
            break;
        }

        // 명령어 처리(수정,삭제,복원,일반 메시지)
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
            // 일반 메시지 전송
            char full_message[1050];
            snprintf(full_message, sizeof(full_message), "%s: %s", name, message);
            if (send(sock, full_message, strlen(full_message), 0) < 0) {
                printf("전송 실패: %d\n", WSAGetLastError());
                break;
            }
        }
    }

    // 소켓 닫기 및 Winsock 정리
    closesocket(sock);
    WSACleanup();
    return 0;
}