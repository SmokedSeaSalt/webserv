package main

import (
	"io"
	"net/http"
	"os"
	"testing"
)

func getBaseURL() string {
	port := os.Getenv("FREE_PORT")
	if port == "" {
		port = "8080" // fallback or fail
	}
	return "http://127.0.0.1:" + port
}

func TestGetHelloWorldHTML(t *testing.T) {
	resp, err := http.Get(getBaseURL() + "/assets/helloWorld.html")
	if err != nil {
		t.Fatalf("Request failed: %v", err)
	}
	defer resp.Body.Close()
	if resp.StatusCode != 200 {
		t.Errorf("Expected 200, got %d", resp.StatusCode)
	}
	body, _ := io.ReadAll(resp.Body)
	if len(body) == 0 {
		t.Error("Body is empty")
	}
	expected := "<p>Hello world</p>"
	if string(body) != expected {
		t.Errorf("Expected body %q, got %q", expected, string(body))
	}
}

func TestGetExamplePNG(t *testing.T) {
	resp, err := http.Get(getBaseURL() + "/assets/example.png")
	if err != nil {
		t.Fatalf("Request failed: %v", err)
	}
	defer resp.Body.Close()
	if resp.StatusCode != 200 {
		t.Errorf("Expected 200, got %d", resp.StatusCode)
	}
}

func TestGetNonExistent(t *testing.T) {
	resp, err := http.Get(getBaseURL() + "/assets/nonExistent.txt")
	if err != nil {
		t.Fatalf("Request failed: %v", err)
	}
	defer resp.Body.Close()
	if resp.StatusCode == 200 {
		t.Errorf("Expected non-200 for missing file, got %d", resp.StatusCode)
	}
}
