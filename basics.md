At its core, an **HTTP server** is just a program that listens for incoming network connections, reads a request written in a specific format (HTTP), and sends back a response in that same format.

Think of it like a restaurant waiter: you (the client/browser) ask for something from the menu (a URL), the waiter (the server) goes to the kitchen (the file system/database), and brings you back your food (the HTML/image) along with a status update ("Here is your food" or "Sorry, we are out of that").

---

## 1. How an HTTP Server Works (The Lifecycle)

An HTTP server follows a strict, repeatable lifecycle for every single request:

1. **Listen:** The server opens a network socket on a specific port (usually port 80 for HTTP or 8080 for development) and waits for a client to connect.
2. **Accept:** A client (like Chrome or `curl`) connects. The server accepts the connection.
3. **Read:** The server reads raw text data sent by the client. This text is the **HTTP Request**.
4. **Parse:** The server parses (interprets) the text to figure out what the client wants (e.g., "Get the file `index.html`").
5. **Respond:** The server finds the resource, formats a piece of text called the **HTTP Response**, and sends it back over the network.
6. **Close:** The server closes the connection (in basic HTTP/1.0) and goes back to waiting for the next client.

---

## 2. Understanding the HTTP Protocol (The Text Format)

HTTP is entirely text-based. If you can read and write strings, you can build an HTTP server.

### The Request

When you type a URL into a browser, it sends a plain text block to the server that looks like this:

```http
GET /index.html HTTP/1.1
Host: localhost:8080
User-Agent: Mozilla/5.0

```

* **`GET`**: The HTTP method (what the client wants to do. `GET` means fetch, `POST` means send data).
* **`/index.html`**: The path to the requested file or resource.
* **`HTTP/1.1`**: The version of the protocol being used.
* **Headers:** The lines below it (like `Host:` and `User-Agent:`) provide extra metadata.
* **Crucial detail:** An HTTP request *always* ends with a completely blank line (`\r\n\r\n`). That’s how your server knows the request header is finished.

### The Response

Your C program will need to string-format a response that looks exactly like this to send back:

```http
HTTP/1.1 200 OK
Content-Type: text/html
Content-Length: 46

<html><body><h1>Hello, World!</h1></body></html>

```

* **`HTTP/1.1 200 OK`**: The status line. `200 OK` means everything went great. If a file isn't found, you'd send `404 Not Found`.
* **`Content-Type`**: Tells the browser what kind of data is coming (e.g., `text/html`, `image/jpeg`, `application/json`).
* **`Content-Length`**: The exact size of the body in bytes.
* **The Blank Line:** Again, a blank line (`\r\n\r\n`) separates the headers from the actual content.
* **The Body:** The actual HTML, text, or file data.

---

## 3. Map to C: What You Will Need to Code

To build this in C, you will be using **Berkeley Sockets** (the standard networking API in Unix/Linux). Here are the exact system functions you will use in your `main()` loop, in order:

| Step | C Socket Function | What it does |
| --- | --- | --- |
| **1** | `socket()` | Creates a network endpoint (like getting a telephone handset). |
| **2** | `bind()` | Assigns an IP address and port number to your socket (assigning a phone number). |
| **3** | `listen()` | Tells the OS to start listening for incoming calls. |
| **4** | `accept()` | Grabs an incoming connection from the queue (answering the phone). This gives you a *new* socket just for this client. |
| **5** | `read()` / `recv()` | Reads the raw HTTP request text from the client socket into a `char` buffer. |
| **6** | `write()` / `send()` | Sends your string-formatted HTTP response text back to the client socket. |
| **7** | `close()` | Hangs up the connection. |

### A Sneak Peek at the Architecture

Your basic C program logic will look like this pseudo-code:

```c
int server_fd = socket(...);
bind(server_fd, ...);
listen(server_fd, 10);

while(1) {
    int client_fd = accept(server_fd, ...);
    
    char buffer[30000] = {0};
    read(client_fd, buffer, 30000); // Read the HTTP Request
    
    // Simple parsing logic: see if buffer contains "GET / "
    
    char *response = "HTTP/1.1 200 OK\nContent-Type: text/plain\nContent-Length: 12\n\nHello World";
    write(client_fd, response, strlen(response)); // Send HTTP Response
    
    close(client_fd);
}

```