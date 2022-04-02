#include <cstdlib>
#include <iostream>
#include <memory>
#include <fstream>
#include <vector>
#include <utility>
#include <boost/asio.hpp>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <boost/algorithm/string/replace.hpp>

using boost::asio::ip::tcp;
using namespace std;

boost::asio::io_context io_context;
tcp::endpoint proxy_endpoint;
bool use_proxy = false;

const vector<string> split(const string &str, const char &delimiter) {
    vector<string> result;
    stringstream ss(str);
    string tok;

    while (std::getline(ss, tok, delimiter)) {
        result.push_back(tok);
    }
    return result;
}

const vector<string> read_file(string filename) {
  vector<string> commands_temp;
  ifstream ifs(filename, std::ios::in);
  if (!ifs.is_open()) {
    cerr << "Failed to open file.\n";
  } 
  else {
    string temp;
    while (getline(ifs, temp)) {
      commands_temp.push_back(temp + "\n");
    }
    ifs.close();
  }
  return commands_temp;
}
string get_html(vector<string> servers_inf){
  string setting = R""""(
        <!DOCTYPE html>
        <html lang="en">
          <head>
            <meta charset="UTF-8" />
            <title>NP Project 3 Sample Console</title>
            <link
              rel="stylesheet"
              href="https://cdn.jsdelivr.net/npm/bootstrap@4.5.3/dist/css/bootstrap.min.css"
              integrity="sha384-TX8t27EcRE3e/ihU7zmQxVncDAy5uIKz4rEkgIXeMed4M0jlfIDPvg6uqKI2xXr2"
              crossorigin="anonymous"
            />
            <link
              href="https://fonts.googleapis.com/css?family=Source+Code+Pro"
              rel="stylesheet"
            />
            <link
              rel="icon"
              type="image/png"
              href="https://cdn0.iconfinder.com/data/icons/small-n-flat/24/678068-terminal-512.png"
            />
            <style>
              * {
                font-family: 'Source Code Pro', monospace;
                font-size: 1rem !important;
              }
              body {
                background-color: #212529;
              }
              pre {
                color: #cccccc;
              }
              b {
                color: #01b468;
              }
            </style>
          </head>
          <body>
            <table class="table table-dark table-bordered">
              <thead>
                <tr>
)"""";

    vector<string> html_col, html_id;
    int server_index = 0;
    for (int i = 0; i < servers_inf.size(); i += 3){
      if ( !servers_inf[i].substr(servers_inf[i].find("=")+1).empty()){
        string html_col_temp = "<th scope=\"col\">" + servers_inf[i].substr(servers_inf[i].find("=")+1) + ":" + servers_inf[i+1].substr(servers_inf[i+1].find("=")+1) + "</th>";
        string html_id_temp = "<td><pre id=\"s" + to_string(server_index) + "\" class=\"mb-0\"></pre></td>";
        if (i == servers_inf.size() - 3){
          html_col.push_back(html_col_temp);
          html_id.push_back(html_id_temp);
          server_index++;
        }
        else if (servers_inf[i+3].substr(servers_inf[i+3].find("=")+1).empty()){
          html_col.push_back(html_col_temp);
          html_id.push_back(html_id_temp);
          server_index++;
          break;
        }
        else{
          html_col_temp = html_col_temp + "\n";
          html_id_temp = html_id_temp + "\n";
          html_col.push_back(html_col_temp);
          html_id.push_back(html_id_temp);
          server_index++;
        }
      }
      else{
        break;
      }
    }

    string windows_second = R""""(
                </tr>
              </thead>
              <tbody>
                <tr>
)"""";
    string windows_last = R""""(
                </tr>
              </tbody>
            </table>
          </body>
        </html> 
)"""";
    string html = setting;

    for (int i = 0; i < server_index; i++){
      html = html + html_col[i];
    }
    html = html + windows_second;
    for (int i = 0; i < server_index; i++){
      html = html + html_id[i];
    }
    html = html + windows_last + "\n";

    return html;

}

string escape_html(string input) {
  string temp = input;
  boost::replace_all(temp, "&", "&amp;");
  boost::replace_all(temp, "\"", "&quot;");
  boost::replace_all(temp, "\'", "&apos;");
  boost::replace_all(temp, "<", "&lt;");
  boost::replace_all(temp, ">", "&gt;");
  boost::replace_all(temp, "\n", "&NewLine;");
  boost::replace_all(temp, "\r", "");
  return temp;
}

void proxy_operate(vector<string> servers_inf){

  string host_proxy, port_proxy;
  host_proxy = servers_inf.end()[-2].substr(3);
  port_proxy = servers_inf.back().substr(3);

  if(host_proxy != "" && port_proxy != "") {
      tcp::resolver resolver(io_context);
      tcp::resolver::query query(host_proxy, port_proxy);
      tcp::resolver::iterator iter = resolver.resolve(query);
      proxy_endpoint = iter->endpoint();
      use_proxy = true;
  }
}

class session
  : public std::enable_shared_from_this<session>
{
public:
  tcp::socket socket_;
  tcp::endpoint endpoint_;
  enum { max_length = 1000000 };
  char data_[max_length];
  char input[max_length];
  vector<string> commands;
  int which_server_;

  session(tcp::socket socket, tcp::endpoint endpoint, string filename, int which_server)
    : socket_(std::move(socket))
  {
    commands = read_file(filename);
    endpoint_ = endpoint;
    which_server_ = which_server;
  }

  //連線
  void start()
  {
    auto self(shared_from_this());
    if(!use_proxy) {
        socket_.async_connect(
            endpoint_,
            [this, self](const boost::system::error_code& error){
                if(!error) {
                    do_read();
                }
                else{
                  cerr << "connect error!!!" << endl;
                }
            }
        );
    }
    else {
        socket_.async_connect(
            proxy_endpoint,
            [this, self](const boost::system::error_code& error){
                if(!error) {
                    reply_connection();
                }
                else{
                    cerr << "connect error!!!" << endl;
                }
            }
        );
    }
  }

  void reply_connection(){
    auto self(shared_from_this());
    string dstip = endpoint_.address().to_string();
    vector<string> dstip_split = split(dstip, '.');
    unsigned short dstport = endpoint_.port();
    unsigned char reply[9] = { 4, 1, dstport / 256, dstport % 256, stoi(dstip_split[0]), stoi(dstip_split[1]), stoi(dstip_split[2]), stoi(dstip_split[3]), 0};
    memcpy(data_, reply, 9);

    socket_.async_write_some(
        boost::asio::buffer(data_, 9),
        [this, self](boost::system::error_code ec, size_t length) {
            if(!ec) {
                do_read();
            }
            else{
                cerr<< "reply error!!!" << endl;
            }
        }
    );
  }

  void do_read()
  {
    auto self(shared_from_this());
    socket_.async_read_some(boost::asio::buffer(data_, max_length),
        [this, self](boost::system::error_code ec, std::size_t length)
        {
          if (!ec)
          {
            string server_output = string(data_);
            memset(data_, '\0', max_length);
            memset(input, '\0', max_length);
            cout << "<script>document.getElementById(\"s" + to_string(which_server_) + "\").innerHTML += \"" + escape_html(server_output) + "\";</script>" << endl;
            cout << flush;
            
            if(server_output.find("%") != string::npos) { // 接到server的%就船東西過去
                do_write();
            }
            else{
                do_read();
            } 
          }
        });
  }

  void do_write() {
    auto self(shared_from_this());
    string cmd = commands[0];
    commands.erase(commands.begin());
    strcpy(input, cmd.c_str());
    cout << "<script>document.getElementById(\"s" + to_string(which_server_) + "\").innerHTML += \"<b>" + escape_html(cmd) + "</b>\";</script>" << endl;
    cout << flush;
  
    boost::asio::async_write(
        socket_, boost::asio::buffer(input, strlen(input)),
        [this, self](boost::system::error_code ec, size_t /*length*/) {
            if (!ec) {
                do_read();
            }
        }
    );
  }
};


int main()
{
  try
  {
    
    cout << "Content-type: text/html\r\n\r\n";
    string query = string(getenv("QUERY_STRING"));
    vector<string> servers_inf;
    servers_inf = split(query, '&');

    string html = get_html(servers_inf);
    cout << html << flush;

    proxy_operate(servers_inf); //處理http 最後的proxy


    for(int i = 0; i < servers_inf.size() - 2; i += 3) {
      int which_server = i / 3;
      string host = servers_inf[i].substr(servers_inf[i].find("=")+1);
      string port = servers_inf[i+1].substr(servers_inf[i+1].find("=")+1);
      string file = "test_case/" + servers_inf[i+2].substr(servers_inf[i+2].find("=")+1);

      if(host != "" && port != "" && file != "") {
        tcp::resolver resolver(io_context); //用來設置和查詢 DNS
        tcp::resolver::query query(host, port); //將ip 主機名 port 轉化為socket的表示格式
        tcp::resolver::iterator iter = resolver.resolve(query); //endpoint 的指針
        tcp::endpoint endpoint = iter->endpoint(); //裝port and ip address
        tcp::socket socket(io_context);
        make_shared<session>(move(socket), endpoint, file, which_server)->start();
      }
    }

    io_context.run();
  }
  catch (std::exception& e)
  {
    std::cerr << "Exception: " << e.what() << "\n";
  }

  return 0;
}