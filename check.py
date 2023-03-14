import os
import socket
import time
import datetime
import sys
import re
from alive_progress import alive_bar
import concurrent.futures
from selenium import webdriver
from selenium.webdriver.chrome.options import Options
from selenium.common.exceptions import WebDriverException

# 파일 경로 설정
list_path = "C:\\list.txt"

# 대기 시간 설정
wait_time = 5

# 시작 시간 기록
start_time = datetime.datetime.now()

# 파일 존재 여부 확인
if not os.path.exists(list_path):
    print("파일이 존재하지 않습니다. 프로그램을 종료합니다.")
    sys.exit(1)

# 결과 저장을 위한 파일 열기
output_file = "result2.txt"
output_stream = open(output_file, "w")

# 도메인 상태별 카운트 초기화
up_count = 0
reset_count = 0
not_resolved_count = 0
timeout_count = 0
warning_count = 0
nodomain_count = 0
error_count = 0

# URL 리스트 읽기
with open(list_path, "r") as f:
    url_list = f.readlines()

# Selenium 설정
options = Options()
options.headless = True
options.add_argument("--disable-extensions")
options.add_argument("--disable-gpu")
options.add_argument("--no-sandbox")
options.add_argument("--ignore-certificate-errors")
driver_path = "./chromedriver.exe"


def check_url(url):
    # URL 접속
    if not url.startswith("http"):
        url = "http://" + url

    try:
        driver = webdriver.Chrome(executable_path=driver_path, options=options)
        driver.get(url)
        time.sleep(wait_time)
        if "404" in driver.title or "Page Not Found" in driver.page_source:
            print(f"{url}: Page Not Found")
            status = "RESET"
        else:
            print(f"{url}: UP")
            status = "UP"
        driver.quit()

    except (socket.gaierror, socket.timeout) as ex:
        print(f"{url}: {str(ex)}")
        status = "RESET"

    except WebDriverException as ex:
        if "ERR_CONNECTION_RESET" in str(ex):
            print(f"{url}: RESET")
            status = "RESET"
        elif "ERR_NAME_NOT_RESOLVED" in str(ex):
            print(f"{url}: NOT_RESOLVED")
            status = "RESET"
        elif "ERR_CONNECTION_TIMED_OUT" in str(ex):
            print(f"{url}: TIMEOUT")
            status = "TIMEOUT"
        elif "DNS_PROBE_FINISHED_NXDOMAIN" in str(ex):
            print(f"{url}: NXDOMAIN")
            status = "NODOMAIN"
        elif "warning.or.kr" in url or "www.warning.or.kr" in url:
            print(f"{url}: WARNING")
            status = "WARNING"
        else:
            print(f"{url}: ERROR({str(ex)})")
            status = f"ERROR({str(ex)})"

    # 결과값 저장
    try:
        ip = socket.gethostbyname(url.split('//')[-1].split('/')[0])
    except:
        ip = "N/A"

    domain_status_list.append((url, ip, status, datetime.datetime.now() - start_time))

# URL별로 상태 확인
results = []
domain_status_list = []
with concurrent.futures.ThreadPoolExecutor(max_workers=None) as executor:
    futures = []
    for url in url_list:
        url = url.strip()  # 개행 문자 제거
        future = executor.submit(check_url, url)
        futures.append(future)

    # 모든 작업이 완료될 때까지 대기하며 progress bar 출력
    with alive_bar(len(futures)) as bar:
        for f in concurrent.futures.as_completed(futures):
            bar()

#도메인별 결과값 저장
results = []
for domain, ip, status, time_taken in domain_status_list:
    result = f"{domain}, {ip}, {status}, {time_taken.total_seconds():.0f}"
    results.append(result)
    output_stream.write(result + "\n")

#도메인 상태별 수와 비율 출력
total_count = len(domain_status_list)
up_count = len([d for d in domain_status_list if d[2] == "UP"])
reset_count = len([d for d in domain_status_list if d[2] == "RESET"])
not_resolved_count = len([d for d in domain_status_list if d[2] == "NOT_RESOLVED"])
timeout_count = len([d for d in domain_status_list if d[2] == "TIMEOUT"])
warning_count = len([d for d in domain_status_list if d[2] == "WARNING"])
nodomain_count = len([d for d in domain_status_list if d[2] == "NODOMAIN"])
error_count = len([d for d in domain_status_list if "ERROR" in d[2]])
up_ratio = up_count / total_count * 100 if total_count else 0
down_ratio = (reset_count + warning_count + nodomain_count + error_count) / total_count * 100 if total_count else 0

#결과 출력
output_stream.write(f"\nTotal domains: {total_count} / ")
output_stream.write(f"UP domains: {up_count}, Ratio: {up_ratio:.2f}% / ")
output_stream.write(f"DOWN domains: {reset_count + warning_count + nodomain_count + error_count}, Ratio: {down_ratio:.2f}% / ")
output_stream.write(f"Execution time: {(datetime.datetime.now() - start_time).total_seconds():.0f} seconds\n")

#결과 저장 파일 닫기
output_stream.close()

print("결과가 성공적으로 저장되었습니다.")
