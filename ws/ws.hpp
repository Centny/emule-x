//
//  ws.hpp
//  emule-x
//
//  Created by Centny on 2/3/17.
//
//

#ifndef ws_hpp
#define ws_hpp

#include <iostream>
#include <map>
#include <vector>
#include <websocketpp/config/asio_no_tls.hpp>
#include <websocketpp/server.hpp>

namespace emulex {
namespace ws {


typedef websocketpp::server<websocketpp::config::asio> server;
typedef server::message_ptr message_ptr;
typedef std::map<std::string, std::string> Args;
typedef server::connection_ptr Con;
typedef websocketpp::connection_hdl HDL;
typedef std::pair<websocketpp::http::status_code::value, std::string> Reply;
typedef websocketpp::lib::function<Reply(Con, Args)> http_json_handler;
typedef websocketpp::lib::function<std::string(HDL, std::string&, std::string&)> http_msg_handler;
#define OkReply(data) Reply(websocketpp::http::status_code::ok, data)
class WS_ {
   public:
    boost::asio::io_service& ios;
    server srv;
    std::string docroot;
    std::map<std::string, http_json_handler> hs;
    std::map<std::string, http_msg_handler> ms;

   public:
    WS_(boost::asio::io_service& ios, short port, std::string docroot);
    virtual void regh(std::string path, http_json_handler h);
    virtual void regm(std::string cmd, http_msg_handler h);

   protected:
    virtual void onmsg(websocketpp::connection_hdl hdl, message_ptr msg);
    virtual void onhttp(websocketpp::connection_hdl hdl);
};
typedef boost::shared_ptr<WS_> WS;

Args parseQuery(std::string query);
}
}

#endif /* ws_hpp */
