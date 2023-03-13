> You can also read to [English!](#english)
# 이 저장소는
현재 PC를 기준으로 도메인에 접속할 수 있는지 확인하는 코드입니다. 파워쉘, C++ 언어, GO언어 코드이며, 출력값은 다음과 같습니다.

* 사이트 주소
* ip주소
* 사이트 상태

# 준비물
1. 파워쉘(PowerShell)(웬만하면 최신 버전을 설치해주세요. 최신 버전은 [여기](https://github.com/PowerShell/PowerShell/releases)에서 다운로드 받을 수 있습니다.
2. 도메인 목록. 도메인 목록은 list.txt 파일에 한줄 씩 아래와 같이 입력해 주세요.

```
https://www.naver.com
https://www.gaonwiki.com
https://uiharu.gaon.xyz
```

list.txt 파일은 <code>C:\list.txt</code>에 위치할 수 있도록 해주세요.

# 결과값
결과값은 스크립트를 실행한 디렉토리에 <code>result.txt</code>로 저장되어 있습니다. 결과는 다음과 같이 나옵니다.

```
www.naver.com, 223.130.200.107, UP, 2023-03-10 09:39:52 2994
www.gaonwiki.com, 104.21.82.129, UP, 2023-03-10 09:39:53 3378
uiharu.gaon.xyz, 129.154.210.92, UP, 2023-03-10 09:39:53 4083
```

도메인, ip주소, 결과 코드, 확인 시간(연-월-일 시:분:초 밀리초) 형태로 결과가 나옵니다.

## 결과 코드 종류
### 정상 작동(UP)
정상적으로 작동하는 경우, UP이라고 표시됩니다.

### 정상 작동(UP)
정상적으로 작동하는 경우, UP이라고 표시됩니다.

### 연결 재설정(RESET)
연결이 재설정 되었다는 오류인 <code>ERR_CONNECTION_RESET</code>인 경우, RESET이라고 표시됩니다.

대부분 검열로 인해 저런 코드가 뜹니다. ~~히토미~~

### warning.or.kr로 이동(WARNING)
warning.or.kr 또는 www.warning.or.kr로 이동하는 경우, WARNING이라고 표시됩니다.

### 사이트가 존재하지 않는 경우(NODOMAIN)
사이트가 존재하지 않는다는 코드인 <code>DNS_PROBE_FINISHED_NXDOMAIN</code>인 경우, NODOMAIN이라고 표시됩니다.

### 기타 오류(ERROR)
기타 오류는 ERROR라 뜹니다.

# 주의사항
다음 주의사항을 꼼꼼히 읽어주세요!

## HTML 코드가 쉘 창에 뜹니다.
이건 어쩔 수가 없어요. 제 능력 밖 인 거 같아요.

## 인터넷에 연결되어 있어야 돼요.
당연하지만 인터넷에 연결되어 있어야 확인이 가능해요.

## list.txt 파일이 반드시 C드라이브 바로 밑에 있어야 해요.
그러지 않으면 작동하지 않아요.

# 언어별 속도
GO언어가 빠르고, C++ 언어가 두번째, 파워쉘 코드가 가장 느려요. 네트워크나 PC 사양에 따라 달라질 수 있어요.

# [](#English)What is this Repository?
Code to check if a domain is reachable based on the current PC. This is Powershell code, and the output is as follows.

* site address
* IP address
* Site status

# Preparation
1. PowerShell (preferably the latest version. The latest version can be downloaded [here](https://github.com/PowerShell/PowerShell/releases).
2. A list of domains. Please enter the domain list in the list.txt file as below, line by line.
```
https://www.naver.com
https://www.gaonwiki.com
https://uiharu.gaon.xyz
```
Please ensure that the list.txt file is located at <code>C:\list.txt</code>.

## Result Code Type

### Normal operation (UP)
If operating normally, UP is displayed.

### Reset connection (RESET)
If <code>ERR_CONNECTION_RESET</code> is an error stating that the connection has been reset, RESET is displayed.

Most censorship causes that code to pop up. ~~Hitomi~~

### Move to warning.or.kr(WARNING)
If you go to warning.or.kr or www.warning.or.kr, it says WARNING.

### Site does not exist (NODOMAIN)
If <code>DNS_PROBE_FINISHED_NXDOMAIN</code> is the code that the site does not exist, it will be displayed as NODOMAIN.

### Other Errors
Other errors appear as Error.

# Precautions
Please read the following precautions carefully!

## The HTML code appears in the Shell window.
There's nothing we can do about this. I think it's beyond my ability.

## It needs to be connected to the Internet.
Of course, you need to be connected to the Internet to check.

## The list.txt file must be right under drive C.
Otherwise, it won't work.

# Compare speed
Go lang > C++ lang >> Powershell

# License
[MIT](https://github.com/gaon12/powershell_site_up_check/blob/main/LICENSE)
