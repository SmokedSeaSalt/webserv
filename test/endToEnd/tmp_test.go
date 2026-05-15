package main

import (
	// "io"
	"net/http"
	"os"
	// "strconv"
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

func TestTemp(t *testing.T) {


	// Try to get before post (should fail)
	// {
	// 	fileName := "/assets/new.html"
	// 	resp, err := http.Get(getBaseURL() + fileName)
	// 	if err != nil {
	// 		t.Fatalf("Request failed: %v", err)
	// 	}
	// 	defer resp.Body.Close()
	// 	if resp.StatusCode == 200 {
	// 		t.Errorf("Expected non-200 after delete, got %d", resp.StatusCode)
	// 	}
	// }
	// Post the new file
	{
		expectedBody := "<p>New page</p>"
		uploadStoreFileName := "/upload/new.html"
		resp, err := http.Post(getBaseURL()+uploadStoreFileName, "text/html", strings.NewReader(expectedBody))
		if err != nil {
			t.Fatalf("POST request failed: %v", err)
		}
		defer resp.Body.Close()
		if resp.StatusCode != 201 {
			t.Errorf("Expected 201 Created, got %d", resp.StatusCode)
		}
	}
}