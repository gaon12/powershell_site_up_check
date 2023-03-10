$list_path = "C:\list.txt"

# 시작 시간 기록
$start_time = Get-Date

# 파일 존재 여부 확인
if (!(Test-Path $list_path)) {
    Write-Host "파일이 존재하지 않습니다. 스크립트를 종료합니다."
    Exit
}

# 결과 저장을 위한 파일 열기
$output_file = "result.txt"
if (Test-Path $output_file) {
    Remove-Item $output_file
}
$output_stream = [System.IO.StreamWriter]::new($output_file)

# 도메인 상태별 카운트 초기화
$up_count = 0
$reset_count = 0
$warning_count = 0
$nodomain_count = 0
$error_count = 0

# 파일 내의 각 도메인 주소 또는 URL 값 반복적으로 읽기
foreach ($line in [System.IO.File]::ReadLines($list_path)) {
    # Skip empty or invalid lines
    if ([string]::IsNullOrWhiteSpace($line)) {
        continue
    }

    # URL 파싱을 위한 System.Uri 객체 생성
    $url = [System.Uri] $line

    # Check if URL is valid
    if ($null -eq $url.Host) {
        Write-Host "Invalid URL: $line"
        continue
    }

    # IP 주소 확인
    $ip_address = ""
    try {
        $ip_address = ([System.Net.Dns]::GetHostAddresses($url.Host) | Where-Object { $_.AddressFamily -eq 'InterNetwork' })[0].ToString()
    }
    catch {
        Write-Host "Failed to get IP address for $line"
    }

    # URL 요청을 위한 WebClient 객체 생성
    $client = [System.Net.WebClient]::new()

    # URL 요청 및 결과 확인
    try {
        $client.DownloadString($url)
        $is_up = "UP"
        $up_count += 1
    }
    catch [System.Net.WebException] {
        if ($_.Exception.InnerException) {
            $error_message = $_.Exception.InnerException.Message
        }
        else {
            $error_message = $_.Exception.Message
        }
        if ($error_message.Contains("ERR_CONNECTION_RESET")) {
            $is_up = "RESET"
            $reset_count += 1
        }
        elseif ($url.Host.Equals("warning.or.kr") -or $url.Host.Equals("www.warning.or.kr")) {
            $is_up = "WARNING"
            $warning_count += 1
        }
        elseif ($error_message.Contains("DNS_PROBE_FINISHED_NXDOMAIN")) {
            $is_up = "NODOMAIN"
            $nodomain_count += 1
        }
        else {
            $is_up = "ERROR"
            $error_count += 1
        }
    }

    # 체크 날짜/시간 확인
    $check_date = Get-date
    $check_date_millisecond = [int] (Get-Date).ToString("ffff")

    # 결과 파일에 쓰기
    $output_stream.WriteLine("{0}, {1}, {2}, {3} {4}", $url.Host, $ip_address, $is_up, $check_date.ToString("yyyy-MM-dd HH:mm:ss"), $check_date_millisecond)
}

# 종료 시간 기록
$end_time = Get-Date

# 실행 시간 계산
$execution_time = New-TimeSpan -Start $start_time -End $end_time

# 도메인 상태별 수와 비율 출력
$total_count = $up_count + $reset_count + $warning_count + $nodomain_count + $error_count
$up_ratio = $up_count / $total_count * 100
$reset_ratio = $reset_count / $total_count * 100
$warning_ratio = $warning_count / $total_count * 100
$nodomain_ratio = $nodomain_count / $total_count * 100
$error_ratio = $error_count / $total_count * 100

$output_stream.Write("Total domains: $total_count / ")
$output_stream.Write("UP domains: $up_count, Ratio: $up_ratio% / ")
$output_stream.Write("RESET domains: $reset_count, Ratio: $reset_ratio% / ")
$output_stream.Write("WARNING domains: $warning_count, Ratio: $warning_ratio% / ")
$output_stream.Write("NODOMAIN domains: $nodomain_count, Ratio: $nodomain_ratio% / ")
$output_stream.Write("ERROR domains: $error_count, Ratio: $error_ratio% / ")

# 실행 시간 출력
$output_stream.Write("Execution time: $($execution_time.Hours) hour(s), $($execution_time.Minutes) minute(s), $($execution_time.Seconds) second(s)")

# 결과 저장 파일 닫기
$output_stream.Close()

Write-Host "결과가 성공적으로 저장되었습니다."
