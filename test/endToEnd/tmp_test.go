package main

import (
	"io"
	"net/http"
	"os"
	// "strconv"
	// "strings"
	"testing"
)

func getBaseURL() string {
	port := os.Getenv("FREE_PORT")
	if port == "" {
		port = "8080" // fallback or fail
	}
	return "http://127.0.0.1:" + port
}


func TestPythonCGI(t *testing.T) {

    t.Run("Get hello.sh", func(t *testing.T) {
        helloExpectedBody := "<html><head><title>Hello from CGI in Shell</title></head><body><h1>Hello World!</h2></body></html>"
        helloFileName := "/cgi-bin/hello.sh"
        resp, err := http.Get(getBaseURL() + helloFileName)
        if err != nil {
            t.Fatalf("Request failed: %v", err)
        }
        defer resp.Body.Close()
        if resp.StatusCode != 200 {
            t.Errorf("Expected 200, got %d", resp.StatusCode)
        }
        respBody, _ := io.ReadAll(resp.Body)
        if string(respBody) != helloExpectedBody {
            t.Errorf("Expected body %q, got %q", helloExpectedBody, string(respBody))
        }
    })
}