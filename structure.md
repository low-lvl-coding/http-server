```text
my_http_server/
│
├── .gitignore               # Ignores compiled binaries and build artifacts
├── Makefile                 # Automates compilation
│
├── public/                  # The "Document Root" (Web files served to clients)
│   ├── index.html           # Your homepage
│   ├── about.html
│   └── favicon.ico
│
├── include/                 # Header files (.h)
│   ├── server.h             # Socket setup and loop declarations
│   ├── http.h               # HTTP request/response parsing structures
│   └── utils.h              # Helper functions (logging, file reading)
│
└── src/                     # Source files (.c)
    ├── main.c               # Entry point (parses CLI args, starts server loop)
    ├── server.c             # Socket, bind, listen, and accept logic
    ├── http.c               # Request parsing and response formatting
    └── utils.c              # Utility implementations

```

---

## Breakdown of Key Components

### 1. The `public/` Folder

This is your server’s sandboxed environment. If a user requests `GET /index.html`, your C code should look inside `public/index.html`.

> **Security Tip:** Never let requests look *outside* of this folder (e.g., handling a request like `GET /../../etc/passwd` is a classic security bug called a Directory Traversal attack).

### 2. The `include/` and `src/` Split

Separating your headers (`.h`) from your implementations (`.c`) keeps the codebase modular.

* **`server.c` / `server.h**`: Focuses purely on networking. It opens the port, listens, accepts the connection, and reads raw bytes into a buffer. It shouldn't care *what* the bytes say; it just passes them to the HTTP layer.
* **`http.c` / `http.h**`: Focuses purely on string manipulation. It takes the raw buffer from `server.c`, parses lines looking for `GET` or `POST`, maps the URL to a file path, and formats the `HTTP/1.1 200 OK` response string.
* **`utils.c` / `utils.h**`: A junk drawer for helper functions. In C, you don't get a free function to "read an entire file into a string buffer," so you'll have to write one yourself using `fopen`, `fseek`, and `fread`. Put that logic here.
* **`main.c`**: The glue. It reads configuration (like what port to run on, e.g., `./server --port 8080`), initializes the server, and starts the infinite loop.
