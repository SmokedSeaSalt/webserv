package main

import (
	"io"
	"net/http"
	"os"
	"strconv"
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
	contentLenStr := resp.Header.Get("content-length")
	expectedContentLenStr := strconv.Itoa(len(expected))
	if contentLenStr != expectedContentLenStr {
		t.Errorf("Expected content length %s, got %s", expectedContentLenStr, contentLenStr)
	}
	t.Logf("TestGetHelloWorldHTML passed successfully")
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
	t.Logf("TestGetExamplePNG passed successfully")
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
	t.Logf("TestGetNonExistent passed successfully")
}

func TestHeadHelloWorldHtml(t *testing.T) {
	req, err := http.NewRequest(http.MethodHead, getBaseURL()+"/assets/helloWorld.html", nil)
	if err != nil {
		t.Fatalf("HEAD request failed: %v", err)
	}
	resp, err := http.DefaultClient.Do(req)
	if err != nil {
		t.Fatalf("HEAD request failed: %v", err)
	}
	defer resp.Body.Close()
	if resp.StatusCode != 200 {
		t.Errorf("Expected 200, got %d", resp.StatusCode)
	}
	// Content-Length check
	expectedBody := "<p>Hello world</p>"
	expectedContentLengthStr := strconv.Itoa(len(expectedBody))
	contentLengthStr := resp.Header.Get("Content-Length")
	if contentLengthStr == "" {
		t.Error("Content-Length header missing")
	}
	if contentLengthStr != expectedContentLengthStr {
		t.Errorf("Expected content length %s, got %s", expectedContentLengthStr, contentLengthStr)
	}

	// Body should be empty for HEAD
	body, _ := io.ReadAll(resp.Body)
	if len(body) != 0 {
		t.Errorf("Expected empty body for HEAD, got %d bytes", len(body))
	}
	t.Logf("TestHeadHelloWorldHtml passed successfully")
}

func TestPostGetDeleteHtmlFile(t *testing.T) {
	expectedBody := "<p>New page</p>"
	fileName := "/assets/uploads/new.html"
	uploadStoreFileName := "/upload/new.html"

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
		resp, err := http.Post(getBaseURL()+uploadStoreFileName, "text/html", strings.NewReader(expectedBody))
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
		req, err := http.NewRequest(http.MethodDelete, getBaseURL()+uploadStoreFileName, nil)
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
		resp, err := http.Get(getBaseURL() + uploadStoreFileName)
		if err != nil {
			t.Fatalf("Request failed: %v", err)
		}
		defer resp.Body.Close()
		if resp.StatusCode == 200 {
			t.Errorf("Expected non-200 after delete, got %d", resp.StatusCode)
		}
	}
	// t.Logf("TestPostGetDeleteHtmlFile passed successfully")
}

func TestPythonCGI(t *testing.T) {


    t.Run("Get hello.py", func(t *testing.T) {
        helloExpectedBody := "<html><head><title>Hello from CGI in Python</title></head><body><h1>Hello World!</h2></body></html>"
        helloFileName := "/cgi-bin/hello.py"
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

    t.Run("Post toUpper.py", func(t *testing.T) {
        requestBody := "tHis SHouLd All BE uppErCAsE 123"
        responseExpectedBody := "THIS SHOULD ALL BE UPPERCASE 123"
        toUpperFileName := "/cgi-bin/toUpper.py"
        resp, err := http.Post(getBaseURL() + toUpperFileName, "text/html", strings.NewReader(requestBody))
        if err != nil {
            t.Fatalf("Request failed: %v", err)
        }
        defer resp.Body.Close()
        if resp.StatusCode != 200 {
            t.Errorf("Expected 200, got %d", resp.StatusCode)
        }
        respBody, _ := io.ReadAll(resp.Body)
        if string(respBody) != responseExpectedBody {
            t.Errorf("Expected body %q, got %q", responseExpectedBody, string(respBody))
        }
    })

	t.Run("Get divideByZero.py", func(t *testing.T) {
		expectedBody := "<h>500 internal server error</h>"
        divideByZeroFileName := "/cgi-bin/divideByZero.py"
        resp, err := http.Get(getBaseURL() + divideByZeroFileName)
        if err != nil {
            t.Fatalf("Request failed: %v", err)
        }
        defer resp.Body.Close()
        if resp.StatusCode != 500 {
            t.Errorf("Expected 500, got %d", resp.StatusCode)
        }
        respBody, _ := io.ReadAll(resp.Body)
        if string(respBody) != expectedBody {
            t.Errorf("Expected body %q, got %q", expectedBody, string(respBody))
        }
    })

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

func TestShellCGI(t *testing.T) {
    t.Run("Get hello.sh", func(t *testing.T) {
        helloExpectedBody := "<html><head><title>Hello from CGI in Shell</title></head><body><h1>Hello World!</h1></body></html>"
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

	t.Run("Post toUpper.sh", func(t *testing.T) {
        requestBody := "tHis SHouLd All BE uppErCAsE 123"
        responseExpectedBody := "THIS SHOULD ALL BE UPPERCASE 123"
        toUpperFileName := "/cgi-bin/toUpper.sh"
        resp, err := http.Post(getBaseURL() + toUpperFileName, "text/html", strings.NewReader(requestBody))
        if err != nil {
            t.Fatalf("Request failed: %v", err)
        }
        defer resp.Body.Close()
        if resp.StatusCode != 200 {
            t.Errorf("Expected 200, got %d", resp.StatusCode)
        }
        respBody, _ := io.ReadAll(resp.Body)
        if string(respBody) != responseExpectedBody {
            t.Errorf("Expected body %q, got %q", responseExpectedBody, string(respBody))
        }
    })

}