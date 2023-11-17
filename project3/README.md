# Remote Batch System

# Part One
## Introduction
Here, you are asked to write a Remote Batch System, which consists of a simple HTTP server called http server and a CGI program console.cgi. We will use Boost.Asio library to accomplish this project.

## Specification
### http server
1. In this project, the URI of HTTP requests will always be in the form of /${cgi name}.cgi
(e.g., /panel.cgi, /console.cgi, /printenv.cgi), and we will only test for the HTTP GET method.
2. Your http server should parse the HTTP headers and follow the CGI procedure (fork, set environment variables, dup, exec) to execute the specified CGI program.
3. The following environment variables are required to set:
(a) REQUEST METHOD (b) REQUEST URI  
(c) QUERY STRING  
(d) SERVER PROTOCOL  
(e) HTTP HOST 
(f) SERVER ADDR  
(g) SERVER PORT  
(h) REMOTE ADDR  
(i) REMOTE PORT  
For instance, if the HTTP request looks like:

```
GET /console.cgi?h0=nplinux1.cs.nctu.edu.tw&p0= ... (too long, ignored)
Host: nplinux8.cs.nctu.edu.tw:7779
User-Agent: Mozilla/5.0
Accept: text/html,application/xhtml+xml,applica ... (too long, ignored)
Accept-Language: en-US,en;q=0.8,zh-TW;q=0.5,zh; ... (too long, ignored)
Accept-Encoding: gzip, deflate
DNT: 1
Connection: keep-alive
Upgrade-Insecure-Requests: 1
```

Then before executing console.cgi, you need to set the corresponding environment variables. In this case, REQUEST METHOD should be ”GET”, HTTP HOST should be ”nplinux8.cs.nctu.edu.tw:7779”, and so on and so forth.

### console.cgi
1. You are highly recommended to inspect and run the CGI samples before you start this section. For details about CGI (Common Gateway Interface), please refer to the course slides as well as the given CGI examples.
2. The console.cgi should parse the connection information (e.g. host, port, file) from the environment variable QUERY STRING, which is set by your HTTP server (see section 2.1).
For example, if QUERY STRING is:

```
h0=nplinux1.cs.nctu.edu.tw&p0=1234&f0=t1.txt&h1=nplinux2.cs.nctu.edu.tw&
p1=5678&f1=t2.txt&h2=&p2=&f2=&h3=&p3=&f3=&h4=&p4=&f4=
```
It should be understood as:

```
h0=nplinux1.cs.nctu.edu.tw    # the hostname of the 1st server
p0=1234                       # the port of the 1st server
f0=t1.txt                     # the file to open
h1=nplinux2.cs.nctu.edu.tw    # the hostname of the 2nd server
p1=5678                       # the port of the 2nd server
f1=t2.txt                     # the file to open
h2=                           # no 3rd server, so this field is empty
p2=                           # no 3rd server, so this field is empty
f2=                           # no 3rd server, so this field is empty
h3=                           # no 4th server, so this field is empty
p3=                           # no 4th server, so this field is empty
f3=                           # no 4th server, so this field is empty
h4=                           # no 5th server, so this field is empty
p4=                           # no 5th server, so this field is empty
f4=                           # no 5th server, so this field is empty
```

# Part two
## Introduction
For this part, you are asked to provides the same functionality as part 1, but with some rules slightly
differs:
1. Implement one program, cgi server.exe, which is a combination of http server, panel.cgi, and
console.cgi.
2. Your program should run on Windows 10.

## Specification
### 2.1 cgi server.exe
1. The cgi server.exe accepts TCP connections and parse the HTTP requests (as http server does),
and we will only test for the HTTP GET method.
2. You don’t need to fork() and exec() since it’s relatively hard to do it on Windows. Simply parse the
request and do the specific job within the same process. We guarantee that in this part the URI of
HTTP requests will be ”/panel.cgi” or ”/console.cgi” plus a query string:
(a) If it is /panel.cgi,
Display the panel form just like panel.cgi in part 1. This time, you can hard code the input
file menu (t1.txt ∼ t10.txt).
(b) If it is /console.cgi?h0=...,
Connect to remote servers specified by the query string. Note that the behaviors MUST be
the same as part 1 in the user’s point of view (though the procedure is different in this
part).
