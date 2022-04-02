
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <vector>
#include <cstdlib>
#include <iostream>
#include <memory>
#include <utility>
#include <boost/asio.hpp>

using boost::asio::ip::tcp;
using namespace std;

int count = 0;
const vector<string> split(const string &str, const char &delimiter) {
    vector<std::string> result;
    stringstream ss(str);
    string tok;

    while (std::getline(ss, tok, delimiter)) {
        result.push_back(tok);
    }
    return result;
}

class session : public std::enable_shared_from_this<session>
{
public:
  tcp::socket socket_;
  enum { max_length = 1024 };
  char data_[max_length];
  session(tcp::socket socket) : socket_(std::move(socket)){}
  void start()
  {
    do_read();
  }

private:
  void do_read()
  {
    auto self(shared_from_this());
    socket_.async_read_some(boost::asio::buffer(data_, max_length),
        [this, self](boost::system::error_code ec, std::size_t length)
        {
          if (!ec)
          {
            vector<string> lines, line_words, var_inf;
            vector<string> env_var = {"REQUEST_METHOD", "REQUEST_URI", "QUERY_STRING", "SERVER_PROTOCOL", "HTTP_HOST", "SERVER_ADDR", "SERVER_PORT", "REMOTE_ADDR", "REMOTE_PORT"};
            string http_request = data_;
            lines = split(http_request, '\r\n');
            line_words = split(lines[0], ' ');
            
            if (lines[0].find("favicon") == string::npos){
              var_inf.push_back(line_words[0]);
              var_inf.push_back(line_words[1]);
              var_inf.push_back("");
              var_inf.push_back(line_words[2]);
              var_inf.push_back(lines[1].substr(lines[1].find(":") + 2));
              var_inf.push_back(socket_.local_endpoint().address().to_string());
              var_inf.push_back(to_string(socket_.local_endpoint().port()));
              var_inf.push_back(socket_.remote_endpoint().address().to_string());
              var_inf.push_back(to_string(socket_.remote_endpoint().port()));

              string path = "";
              if (line_words[1].find("?") != string::npos){
                vector<string> temp = split(line_words[1], '?');
                var_inf[2] = temp[1];
                string before_question = temp[0];
                path = before_question.substr(1);
              }
              else{
                string temp = var_inf[1];
                path = temp.substr(1);
            }
            
              /*
              cout << "path: " << path << endl;
              cout << "QUERY STRING: " << var_inf[2] << endl;
              */
              cout << http_request << endl;
              
              do_fork_cgi(env_var, var_inf, path);
              cout << "do fork ~~~" << endl;
            }
          }
        });
  }
  
  void do_fork_cgi(vector<string> env_var, vector<string> var_inf, string path){
    auto self(shared_from_this());
    char status_code[100] = "HTTP/1.1 200 OK\r\n";
    boost::asio::async_write(socket_, boost::asio::buffer(status_code, strlen(status_code)),
        [this, self, path, env_var, var_inf](boost::system::error_code ec, std::size_t)
        {
          if (!ec)
          {
            int newpid;
            newpid = fork();
            if (newpid < 0){
              cerr << "fork error!!!" << endl;
            }
            else if (newpid == 0){
              clearenv();
              for(int i = 0; i < 9; i++){
                setenv(env_var[i].c_str(), var_inf[i].c_str(), 1);
              }
              close(0);
              dup(socket_.native_handle());
              close(1);
              dup(socket_.native_handle());
              close(2);
              dup(socket_.native_handle());
              if(execlp(path.c_str(), path.c_str(), NULL) == -1) {
                cerr << "execution error!!" << endl;
              }
            }
            else{
              socket_.close();
            }
          }
        });
  }
};

class server
{
public:
  server(boost::asio::io_context& io_context, short port)
    : acceptor_(io_context, tcp::endpoint(tcp::v4(), port))
  {
    do_accept();
  }

private:
  void do_accept()
  {
    acceptor_.async_accept(
        [this](boost::system::error_code ec, tcp::socket socket)
        {
          if (!ec)
          {
            cout << "get request!!" << endl;
            std::make_shared<session>(std::move(socket))->start();
          }
          do_accept();
        });

  }

  tcp::acceptor acceptor_;
};

int main(int argc, char* argv[])
{
  try
  {
    if (argc != 2)
    {
      std::cerr << "Usage: async_tcp_echo_server <port>\n";
      return 1;
    }
    boost::asio::io_context io_context;
    cout << "waiting for http request ..." << endl;
    server s(io_context, std::atoi(argv[1]));
    io_context.run();
  }
  catch (std::exception& e)
  {
    std::cerr << "Exception: " << e.what() << "\n";
  }
  return 0;
}
