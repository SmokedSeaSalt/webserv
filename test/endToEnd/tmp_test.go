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
	t.Run("Get greetings.py", func(t *testing.T) {
        greetingsExpectedBody := "<h1>Hello john!</h1>"
        greetingsFileNameQueryStr := "/cgi-bin/greetings.py?name=john"
        resp, err := http.Get(getBaseURL() + greetingsFileNameQueryStr)
        if err != nil {
            t.Fatalf("Request failed: %v", err)
        }
        defer resp.Body.Close()
        if resp.StatusCode != 200 {
            t.Errorf("Expected 200, got %d", resp.StatusCode)
        }
        respBody, _ := io.ReadAll(resp.Body)
        if string(respBody) != greetingsExpectedBody {
            t.Errorf("Expected body %q, got %q", greetingsExpectedBody, string(respBody))
        }
    })
}