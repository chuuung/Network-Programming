#include <boost/asio.hpp>
#include <cstdlib>
#include <iostream>
#include <memory>
#include <utility>
#include <stdio.h>
#include <iomanip>
#include <sstream>
#include <string>
#include <string.h>
#include <array>
#include <fstream>
#include <vector>
#include <boost/asio.hpp>

using boost::asio::ip::tcp;
using namespace std;

boost::asio::io_context io_context;

struct sock4req{
    string s_ip;
    string s_port;
	string d_ip;
    string d_port;
    string domain;
	int command;/* 1 : CONNECT , 2 : BIND */
};

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

const vector<string> split(const string &str, const char &delimiter) {
    vector<std::string> result;
    stringstream ss(str);
    string tok;

    while (std::getline(ss, tok, delimiter)) {
        result.push_back(tok);
    }
    return result;
}

//--------------------------------------------------------------------------------------------

class server {
   public:
    tcp::acceptor acceptor_;
    tcp::acceptor bind_acceptor;

    tcp::socket source_sock{io_context};
    tcp::socket dest_sock{io_context};
    tcp::endpoint endpoint;
    
    enum { max_length = 1000000 };
    unsigned char source_buf[max_length];
    unsigned char dest_buf[max_length];
    unsigned short bind_port;

    sock4req sock4req_;

    server(boost::asio::io_context& io_context, short port)
        : acceptor_(io_context, tcp::endpoint(tcp::v4(), port)), bind_acceptor(io_context, tcp::endpoint(tcp::v4(), 0)) {
        do_accept();
    }

    void do_accept() {
        acceptor_.async_accept(
            [this](boost::system::error_code ec, tcp::socket socket) {
                if (!ec) {
                    io_context.notify_fork(boost::asio::io_context::fork_prepare);
                    int newpid = fork();
                    if (newpid < 0){
                        cerr << "fork error!!!" << endl;
                    }
                    else if (newpid == 0) { // child
                        io_context.notify_fork(boost::asio::io_context::fork_child);
                        acceptor_.close();
                        source_sock = move(socket);
                        do_parse();
                    }
                    else{ // parent
                        io_context.notify_fork(boost::asio::io_context::fork_parent);
                        socket.close();
                        do_accept();
                    }
                }
            }
        );
    }
    void do_parse() {
        source_sock.async_read_some(
            boost::asio::buffer(source_buf, max_length),
            [this](boost::system::error_code ec, size_t length) {
                if (!ec) {
                    sock4req_ = {};
                    sock4req_.command = source_buf[1]; //connect or bind
                    sock4req_.s_ip = source_sock.remote_endpoint().address().to_string();
                    sock4req_.s_port = to_string(source_sock.remote_endpoint().port());
                    sock4req_.d_port = to_string((int)source_buf[2] * 256 + (int)source_buf[3]);
                    string ip = to_string(abs(source_buf[4])) + "." + to_string(abs(source_buf[5])) + "." + to_string(abs(source_buf[6])) + "." + to_string(abs(source_buf[7]));
                    sock4req_.d_ip = ip;
                    sock4req_.domain = get_domain(length);
                    
                    do_operation();
                }
            }
        );
    }
    string get_domain(size_t length) {
        vector<int> null_posi;
        string domain = "";

        for(int i=8; i<length; i++) {
            if(source_buf[i] == 0x00){
                null_posi.push_back(i);
            }
        }
        if (null_posi.size() != 1){
            for(int i = null_posi[0]+1; i< null_posi[1]; i++){
                domain += source_buf[i];
            }
        }
        return domain;
    }

    void do_operation() {
        if(sock4req_.command == 1) {
            string host;
            if(sock4req_.domain != "")
                host = sock4req_.domain;
            else
                host = sock4req_.d_ip;
            tcp::resolver resolver(io_context);
            tcp::resolver::query query(host, sock4req_.d_port);
            tcp::resolver::iterator iter = resolver.resolve(query);
            endpoint = iter->endpoint();
            tcp::socket temp_sock(io_context);
            dest_sock = move(temp_sock);


            if(do_firewall()) {
                print_info(1);
                
                do_connect();
            }
            else {
                print_info(0);
                reply_reject();
            }
        }
        else if(sock4req_.command == 2) {
            if(do_firewall()) {
                print_info(1);
                
                bind_port_reply();
            }
            else {
                print_info(0);
                reply_reject();
            }
        }
    }

    bool do_firewall() {
        vector<string> rules = read_file("socks.conf");
        for(int i=0; i<rules.size(); i++) {
            vector<string> rule_split = split(rules[i], ' ');

            if((rule_split[1] == "c" && sock4req_.command == 1) || (rule_split[1] == "b" && sock4req_.command == 2)) {
                string client_ip = source_sock.remote_endpoint().address().to_string();
                string pattern = rule_split[2];
                vector <string> client_ip_pattern = split(client_ip, '.');
                vector<string> ip_pattern = split(pattern, '.');
                for(int i = 0; i < ip_pattern.size(); i++){
                    if (ip_pattern[i] == "*"){
                        if (i == ip_pattern.size() - 1){
                            return true;
                        }
                        else{
                            continue;
                        }
                    }
                    else{
                        if (client_ip_pattern[i] != ip_pattern[i]){
                            return false;
                            break;
                        }
                    } 
                }    
            }
        }
        return false;
    }

    void do_connect() {
        memset(source_buf, 0x00, max_length);
        memset(dest_buf, 0x00, max_length);
        dest_sock.async_connect(
            endpoint,
            [this](const boost::system::error_code& ec){
                if(!ec) {
                    conn_reply();
                }
                else{
                    cerr << "Connect error" << endl;
                }
            }
        );
    }

    void conn_reply() {
        unsigned char reply[8] = {0, 90, 0, 0, 0, 0, 0, 0};
        memcpy(source_buf, reply, 8);
        boost::asio::async_write(
            source_sock, boost::asio::buffer(source_buf, 8),
            [this](boost::system::error_code ec, size_t length) {
                if (!ec) {
                    read_from_source();
                    read_from_dest();
                }
            }
        );
    }

    void bind_port_reply() {
        memset(source_buf, 0x00, max_length);
        memset(dest_buf, 0x00, max_length);

        bind_port = bind_acceptor.local_endpoint().port();
        unsigned char reply[8] = {0, 90, bind_port / 256, bind_port % 256, 0, 0, 0, 0};
        memcpy(source_buf, reply, sizeof(reply));
        boost::asio::async_write(
            source_sock, boost::asio::buffer(source_buf, sizeof(reply)),
            [this](boost::system::error_code ec, size_t /*length*/) {
                if (!ec) {
                    bind_accept();
                }
                else{
                    cerr << "reply error !!!" << endl;
                }
            }
        );
    }

    void bind_accept() {
        bind_acceptor.async_accept(
            [this](boost::system::error_code ec, tcp::socket socket) {
                if (!ec) {
                    dest_sock = move(socket);
                    bind_accept_reply();
                    bind_acceptor.close();
                }
                else {
                    cout << "accept error !!!" << endl;
                }
            }
        );
    }
    void bind_accept_reply() {
        unsigned char reply[8] = {0, 90, bind_port / 256, bind_port % 256, 0, 0, 0, 0};
        memcpy(source_buf, reply, 8);
        boost::asio::async_write(
            source_sock, boost::asio::buffer(source_buf, 8),
            [this](boost::system::error_code ec, size_t /*length*/) {
                if (!ec) {
                    read_from_source();
                    read_from_dest();
                }
            }
        );
    }

    

    void read_from_source() {
        source_sock.async_read_some(
            boost::asio::buffer(source_buf, max_length),
            [this](boost::system::error_code ec, size_t length) {
                if (!ec) {
                    write_to_dest(length);
                }
                else{
                    source_sock.close();
                }
            }
        );
    }

    void write_to_dest(size_t length) {
        boost::asio::async_write(
            dest_sock, boost::asio::buffer(source_buf, length),
            [this](boost::system::error_code ec, size_t length) {
                if (!ec) {
                    read_from_source();
                }
                else{
                    source_sock.close();
                }
            }
        );
    }

    void read_from_dest() {
        dest_sock.async_read_some(
            boost::asio::buffer(dest_buf, max_length),
            [this](boost::system::error_code ec, size_t length) {
                if (!ec) {
                    write_to_source(length);
                }
                else{
                    dest_sock.close();
                }
            }
        );
    }

    void write_to_source(size_t length) {
        boost::asio::async_write(
            source_sock, boost::asio::buffer(dest_buf, length),
            [this](boost::system::error_code ec, size_t length) {
                if (!ec) {
                    read_from_dest();
                }
                else {
                    dest_sock.close();
                }
            }
        );
    }

    void reply_reject() {
        unsigned char reply[8] = {0, 91, 0, 0, 0, 0, 0, 0};
        memcpy(source_buf, reply, 8);
        boost::asio::async_write(
            source_sock, boost::asio::buffer(source_buf, 8),
            [this](boost::system::error_code ec, size_t /*length*/) {
                if (!ec) {
                    source_sock.close();
                }
            }
        );
    }

    void print_info(int valid){
        cout << "<S_IP>: " << source_sock.remote_endpoint().address().to_string() << endl;
        cout << "<S_PORT>: " << source_sock.remote_endpoint().port() << endl;
        cout << "<D_IP>: " << endpoint.address().to_string() << endl;
        cout << "<D_PORT>: " << sock4req_.d_port << endl;
        if (sock4req_.command == 1){
            cout << "<Command>: " << "CONNECT" << endl;
            if (valid == 1)
                cout << "<Reply>: " << "Accept" << endl ;
            else
                cout << "<Reply>: " << "Reject" << endl ;
        }
        else{
            cout << "<Command>: " << "BIND" << endl;
            if (valid == 1)
                cout << "<Reply>: " << "Accept" << endl ;
            else
                cout << "<Reply>: " << "Reject" << endl ;
        }
        cout << endl;

    }
};

int main(int argc, char* argv[]) {
    try {
        if (argc != 2) {
            cerr << "Usage: async_tcp_echo_server <port>\n";
            return 1;
        }

        server s(io_context, atoi(argv[1]));
        cout << "starting !" << endl;
        io_context.run();

    } catch (exception& e) {
        cerr << "Exception: " << e.what() << "\n";
    }

    return 0;
}