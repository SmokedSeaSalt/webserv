package main

import (
	"io"
	"net/http"
	"os"
	"strings"
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

func TestPostGetDeleteHtmlFile(t *testing.T) {
	expectedBody := "<p>New page</p>"
	fileName := "/assets/new.html"
	// Try to get before post (should fail)
	{
		resp, err := http.Get(getBaseURL() + fileName)
		if err != nil {
			t.Fatalf("Request failed: %v", err)
		}
		defer resp.Body.Close()
		if resp.StatusCode == 200 {
			t.Errorf("Expected non-200 after delete, got %d", resp.StatusCode)
		}
	}
	// Post the new file
	{
		resp, err := http.Post(getBaseURL()+fileName, "text/html", strings.NewReader(expectedBody))
		if err != nil {
			t.Fatalf("POST request failed: %v", err)
		}
		defer resp.Body.Close()
		if resp.StatusCode != 201 {
			t.Errorf("Expected 201 Created, got %d", resp.StatusCode)
		}
	}
	// Get the newly posted file
	{
		resp, err := http.Get(getBaseURL() + fileName)
		if err != nil {
			t.Fatalf("Request failed: %v", err)
		}
		defer resp.Body.Close()
		if resp.StatusCode != 200 {
			t.Errorf("Expected 200, got %d", resp.StatusCode)
		}
		   respBody, _ := io.ReadAll(resp.Body)
		   if len(respBody) == 0 {
			   t.Error("Body is empty")
		   }
		   if string(respBody) != expectedBody {
			   t.Errorf("Expected body %q, got %q", expectedBody, string(respBody))
		   }
	}
	// Delete the newly posted file
	{
		req, err := http.NewRequest(http.MethodDelete, getBaseURL()+fileName, nil)
		if err != nil {
			t.Fatalf("DELETE request failed: %v", err)
		}
		resp, err := http.DefaultClient.Do(req)
		if err != nil {
			t.Fatalf("DELETE request failed: %v", err)
		}
		defer resp.Body.Close()
		if resp.StatusCode != 204 && resp.StatusCode != 200 {
			t.Errorf("Expected 204 No Content or 200 OK, got %d", resp.StatusCode)
		}
	}
	// Try to get the deleted file (should fail)
	{
		resp, err := http.Get(getBaseURL() + fileName)
		if err != nil {
			t.Fatalf("Request failed: %v", err)
		}
		defer resp.Body.Close()
		if resp.StatusCode == 200 {
			t.Errorf("Expected non-200 after delete, got %d", resp.StatusCode)
		}
	}
}
