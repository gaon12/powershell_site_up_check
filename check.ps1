#########################################################
#                                                       #
#                  도메인 주소, ip주소 표시               #
#                                                       #
#                                                       #
#########################################################

$list_path = "C:\list.txt"

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
        }
        elseif ($url.Host.Equals("warning.or.kr") -or $url.Host.Equals("www.warning.or.kr")) {
            $is_up = "WARNING"
        }
        elseif ($error_message.Contains("DNS_PROBE_FINISHED_NXDOMAIN")) {
            $is_up = "NODOMAIN"
        }
        else {
            $is_up = "ERROR"
        }
    }

    # 체크 날짜/시간 확인
    $check_date = Get-date
    $check_date_millisecond = [int] (Get-Date).ToString("ffff")

    # 결과 파일에 쓰기
    $output_stream.WriteLine("{0}, {1}, {2}, {3} {4}", $url.Host, $ip_address, $is_up, $check_date.ToString("yyyy-MM-dd HH:mm:ss"), $check_date_millisecond)
}

# 결과 저장 파일 닫기
$output_stream.Close()
Write-Host "결과가 성공적으로 저장되었습니다."