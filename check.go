package main

import (
    "fmt"
    "io/ioutil"
    "net"
    "net/http"
    "net/url"
    "os"
    "strings"
    "time"
)

func main() {
    listPath := "C:\\list.txt"

    // 시작 시간 기록
    startTime := time.Now()

    // 파일 존재 여부 확인
    _, err := os.Stat(listPath)
    if os.IsNotExist(err) {
        fmt.Println("파일이 존재하지 않습니다. 스크립트를 종료합니다.")
        os.Exit(1)
    }

    // 결과 저장을 위한 파일 열기
    outputFilePath := "result.txt"
    _, err = os.Stat(outputFilePath)
    if err == nil {
        err = os.Remove(outputFilePath)
        if err != nil {
            fmt.Printf("Error deleting file: %v\n", err)
        }
    }
    outputFile, err := os.Create(outputFilePath)
    if err != nil {
        fmt.Printf("Error creating file: %v\n", err)
        os.Exit(1)
    }

    // 도메인 상태별 카운트 초기화
    upCount := 0
    resetCount := 0
    warningCount := 0
    nodomainCount := 0
    errorCount := 0

    // 파일 내의 각 도메인 주소 또는 URL 값 반복적으로 읽기
    fileContent, err := ioutil.ReadFile(listPath)
    if err != nil {
        fmt.Printf("Error reading file: %v\n", err)
        os.Exit(1)
    }
    lines := strings.Split(string(fileContent), "\n")
    for _, line := range lines {
        line = strings.TrimSpace(line)
        // Skip empty or invalid lines
        if len(line) == 0 || strings.HasPrefix(line, "#") {
            continue
        }

        // URL 파싱을 위한 url.URL 객체 생성
        url, err := url.Parse(line)
        if err != nil {
            fmt.Printf("Invalid URL: %s\n", line)
            continue
        }

        // Check if URL is valid
        if url.Hostname() == "" {
            fmt.Printf("Invalid URL: %s\n", line)
            continue
        }

        // IP 주소 확인
        ipAddresses, err := net.LookupIP(url.Hostname())
        if err != nil {
            fmt.Printf("Failed to get IP address for %s: %v\n", line, err)
        }
        var ipAddress string
        for _, ip := range ipAddresses {
            if ip.To4() != nil {
                ipAddress = ip.String()
                break
            }
        }

        // URL 요청을 위한 http.Client 객체 생성
        client := &http.Client{}

        // URL 요청 및 결과 확인
        resp, err := client.Get(url.String())
        if err == nil {
            defer resp.Body.Close()
            upCount++
            fmt.Fprintf(outputFile, "%s, %s, %s, %s %d\n", url.Hostname(), ipAddress, "UP", time.Now().Format("2006-01-02 15:04:05"), time.Now().Nanosecond()/1000000)
            continue
        }

                if netErr, ok := err.(*net.OpError); ok && netErr.Timeout() {
            errorCount++
            fmt.Fprintf(outputFile, "%s, %s, %s, %s %d\n", url.Hostname(), ipAddress, "ERROR", time.Now().Format("2006-01-02 15:04:05"), time.Now().Nanosecond()/1000000)
            continue
        } else if strings.Contains(err.Error(), "connection reset by peer") {
            resetCount++
            fmt.Fprintf(outputFile, "%s, %s, %s, %s %d\n", url.Hostname(), ipAddress, "RESET", time.Now().Format("2006-01-02 15:04:05"), time.Now().Nanosecond()/1000000)
            continue
        } else if strings.Contains(err.Error(), "no such host") {
            nodomainCount++
            fmt.Fprintf(outputFile, "%s, %s, %s, %s %d\n", url.Hostname(), ipAddress, "NODOMAIN", time.Now().Format("2006-01-02 15:04:05"), time.Now().Nanosecond()/1000000)
            continue
        } else if strings.Contains(url.Hostname(), "warning.or.kr") {
            warningCount++
            fmt.Fprintf(outputFile, "%s, %s, %s, %s %d\n", url.Hostname(), ipAddress, "WARNING", time.Now().Format("2006-01-02 15:04:05"), time.Now().Nanosecond()/1000000)
            continue
        } else {
            errorCount++
            fmt.Fprintf(outputFile, "%s, %s, %s, %s %d\n", url.Hostname(), ipAddress, "ERROR", time.Now().Format("2006-01-02 15:04:05"), time.Now().Nanosecond()/1000000)
            continue
        }
    }

    // 종료 시간 기록
    endTime := time.Now()

    // 실행 시간 계산
    executionTime := endTime.Sub(startTime)

    // 도메인 상태별 수와 비율 출력
    totalCount := upCount + resetCount + warningCount + nodomainCount + errorCount
    upRatio := float64(upCount) / float64(totalCount) * 100
    resetRatio := float64(resetCount) / float64(totalCount) * 100
    warningRatio := float64(warningCount) / float64(totalCount) * 100
    nodomainRatio := float64(nodomainCount) / float64(totalCount) * 100
    errorRatio := float64(errorCount) / float64(totalCount) * 100

    fmt.Fprintf(outputFile, "Total domains: %d / ", totalCount)
    fmt.Fprintf(outputFile, "UP domains: %d, Ratio: %.2f%% / ", upCount, upRatio)
    fmt.Fprintf(outputFile, "RESET domains: %d, Ratio: %.2f%% / ", resetCount, resetRatio)
    fmt.Fprintf(outputFile, "WARNING domains: %d, Ratio: %.2f%% / ", warningCount, warningRatio)
    fmt.Fprintf(outputFile, "NODOMAIN domains: %d, Ratio: %.2f%% / ", nodomainCount, nodomainRatio)
    fmt.Fprintf(outputFile, "ERROR domains: %d, Ratio: %.2f%% / ", errorCount, errorRatio)

        // 실행 시간 출력
    fmt.Fprintf(outputFile, "Execution time: %d hour(s), %d minute(s), %d second(s)", int(executionTime.Hours()), int(executionTime.Minutes()), int(executionTime.Seconds()))

    // 결과 저장 파일 닫기
    outputFile.Close()

    fmt.Println("결과가 성공적으로 저장되었습니다.")
}
