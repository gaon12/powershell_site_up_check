#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include <fstream>
#include <string>
#include <ctime>
#include <chrono>
#include <cstdlib>
#include <cstring>
#include <regex>
#include <exception>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#include <io.h>
#include <wininet.h> // wininet.h 헤더 파일 추가

#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "wininet.lib") // wininet.lib 라이브러리 링크

using namespace std;

int main()
{
    // 파일 경로 설정
    string list_path = "C:\\list.txt";

    // 시작 시간 기록
    auto start_time = chrono::system_clock::now();

    // 파일 존재 여부 확인
    if (_access(list_path.c_str(), 0) != 0)
    {
        cout << "파일이 존재하지 않습니다. 프로그램을 종료합니다." << endl;
        return 1;
    }

    // 결과 저장을 위한 파일 열기
    string output_file = "result.txt";
    ofstream output_stream(output_file);

    // 도메인 상태별 카운트 초기화
    int up_count = 0;
    int reset_count = 0;
    int warning_count = 0;
    int nodomain_count = 0;
    int error_count = 0;

    // URL 파싱을 위한 hints 구조체 선언
    addrinfo hints = {};

    // 파일 내의 각 도메인 주소 또는 URL 값 반복적으로 읽기
    ifstream input_stream(list_path);
    string line;
    while (getline(input_stream, line))
    {
        // Skip empty or invalid lines
        if (line.empty())
        {
            continue;
        }

        // URL 파싱을 위한 regex 객체 생성
        regex url_regex("^(https?://)?([a-zA-Z0-9\\-.]+\\.[a-zA-Z]{2,}(\\:[0-9]+)?)");
        smatch url_match;
        if (!regex_search(line, url_match, url_regex))
        {
            cout << "Invalid URL: " << line << endl;
            continue;
        }

        // URL 호스트 확인
        string host = url_match[2];
        WSADATA wsaData;
        int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
        if (result != 0) {
            cout << "WSAStartup failed: " << result << endl;
            return 1;
        }

        hints.ai_family = AF_UNSPEC;
        hints.ai_socktype = SOCK_STREAM;
        hints.ai_protocol = IPPROTO_TCP;

        addrinfo* pResult = NULL;
        int iResult = getaddrinfo(host.c_str(), NULL, &hints, &pResult);
        if (iResult != 0)
        {
            if (iResult == WSAHOST_NOT_FOUND) {
                cout << "DNS name not found" << endl;
                nodomain_count += 1;
            }
            else {
                cout << "getaddrinfo failed: " << iResult << endl;
                error_count += 1;
            }
            continue;
        }

        string ip_address;
        for (ADDRINFOA* ptr = pResult; ptr != NULL; ptr = ptr->ai_next)
        {
            void* addr;
            char ipstr[INET6_ADDRSTRLEN];
            if (ptr->ai_family == AF_INET)
            {
                struct sockaddr_in* ipv4 = (struct sockaddr_in*)ptr->ai_addr;
                addr = &(ipv4->sin_addr);
            }
            else
            {
                struct sockaddr_in6* ipv6 = (struct sockaddr_in6*)ptr->ai_addr;
                addr = &(ipv6->sin6_addr);
            }
            inet_ntop(ptr->ai_family, addr, ipstr, sizeof(ipstr));
            ip_address = ipstr;
            break;
        }

        freeaddrinfo(pResult);
        WSACleanup();

        // URL 요청 및 결과 확인
        HINTERNET hInternet = InternetOpen(L"WinHTTP", INTERNET_OPEN_TYPE_DIRECT, NULL, NULL, 0);
        if (hInternet == NULL)
        {
            cout << "InternetOpen failed: " << GetLastError() << endl;
            return 1;
        }

        HINTERNET hConnect = InternetConnectA(hInternet, host.c_str(), INTERNET_DEFAULT_HTTP_PORT, NULL, NULL, INTERNET_SERVICE_HTTP, 0, 0);
        if (hConnect == NULL)
        {
            cout << "InternetConnect failed: " << GetLastError() << endl;
            return 1;
        }

        HINTERNET hRequest = HttpOpenRequestA(hConnect, "GET", "/", NULL, NULL, NULL, INTERNET_FLAG_RELOAD | INTERNET_FLAG_NO_CACHE_WRITE, 0);
        if (hRequest == NULL)
        {
            cout << "HttpOpenRequest failed: " << GetLastError() << endl;
            return 1;
        }

        BOOL bResult = HttpSendRequestA(hRequest, NULL, 0, NULL, 0);
        if (!bResult)
        {
            DWORD dwError = GetLastError();
            if (dwError == ERROR_INTERNET_CONNECTION_RESET)
            {
                cout << "Connection reset by peer" << endl;
                reset_count += 1;
            }
            else if (host == "warning.or.kr" || host == "www.warning.or.kr")
            {
                cout << "WARNING" << endl;
                warning_count += 1;
            }
            else if (dwError == ERROR_INTERNET_NAME_NOT_RESOLVED)
            {
                cout << "DNS name not resolved" << endl;
                nodomain_count += 1;
            }
            else
            {
                cout << "Error: " << dwError << endl;
                error_count += 1;
            }
        }
        else
        {
            cout << "UP" << endl;
            up_count += 1;
        }

        InternetCloseHandle(hRequest);
        InternetCloseHandle(hConnect);
        InternetCloseHandle(hInternet);

        // 체크 날짜/시간 확인
        auto now = chrono::system_clock::now();
        auto now_time = chrono::system_clock::to_time_t(now);
        auto now_millisecond = chrono::duration_cast<chrono::milliseconds>(now.time_since_epoch()).count() % 1000;

        // 결과 파일에 쓰기
        output_stream << host << ", " << ip_address << ", " << (bResult ? "UP" : "DOWN") << ", " << put_time(localtime(&now_time), "%Y-%m-%d %H:%M:%S") << " " << now_millisecond << endl;
    }

    input_stream.close();

    // 종료 시간 기록
    auto end_time = chrono::system_clock::now();

    // 실행 시간 계산
    auto execution_time = chrono::duration_cast<chrono::seconds>(end_time - start_time);

    // 도메인 상태별 수와 비율 출력
    int total_count = up_count + reset_count + warning_count + nodomain_count + error_count;
    float up_ratio = up_count / static_cast<float>(total_count) * 100;
    float reset_ratio = reset_count / static_cast<float>(total_count) * 100;
    float warning_ratio = warning_count / static_cast<float>(total_count) * 100;
    float nodomain_ratio = nodomain_count / static_cast<float>(total_count) * 100;
    float error_ratio = error_count / static_cast<float>(total_count) * 100;

    output_stream << "Total domains: " << total_count << " / ";
    output_stream << "UP domains: " << up_count << ", Ratio: " << up_ratio << "% / ";
    output_stream << "RESET domains: " << reset_count << ", Ratio: " << reset_ratio << "% / ";
    output_stream << "WARNING domains: " << warning_count << ", Ratio: " << warning_ratio << "% / ";
    output_stream << "NODOMAIN domains: " << nodomain_count << ", Ratio: " << nodomain_ratio << "% / ";
    output_stream << "ERROR domains: " << error_count << ", Ratio: " << error_ratio << "% / ";

    // 실행 시간 출력
    int hours = execution_time.count() / 3600;
    int minutes = (execution_time.count() % 3600) / 60;
    int seconds = execution_time.count() % 60;
    output_stream << "Execution time: " << hours << " hour(s), " << minutes << " minute(s), " << seconds << " second(s)" << endl;

    // 결과 저장 파일 닫기
    output_stream.close();

    cout << "결과가 성공적으로 저장되었습니다." << endl;

    return 0;
}