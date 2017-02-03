//
//  ws.cpp
//  emule-x
//
//  Created by Centny on 2/3/17.
//
//

#include "ws.hpp"
#include <boost-utils/log/log.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/foreach.hpp>
#include <fstream>

namespace emulex {
namespace ws {

WS_::WS_(boost::asio::io_service& ios, short port, std::string docroot) : ios(ios), docroot(docroot) {
    // Set logging settings
    srv.set_access_channels(websocketpp::log::alevel::all);
    srv.clear_access_channels(websocketpp::log::alevel::frame_payload);
    // Initialize Asio
    srv.init_asio(&ios);
    // Register our message handler
    srv.set_http_handler(bind(&WS_::onhttp, this, ::_1));
    srv.set_message_handler(bind(&WS_::onmsg, this, ::_1, ::_2));
    // Listen on port
    srv.listen(port);
    srv.get_io_service();
    // Start the server accept loop
    srv.start_accept();
}

void WS_::reg(std::string path, http_json_handler h) { hs[path] = h; }

void WS_::onmsg(websocketpp::connection_hdl hdl, message_ptr msg) {
    std::string mdata = msg->get_payload();
    V_LOG_D("WS_ onmsg called with message %s", mdata.c_str());
    if (mdata == "stop-listening") {
        srv.stop_listening();
        return;
    }
}

void WS_::onhttp(websocketpp::connection_hdl hdl) {
    // Upgrade our connection handle to a full connection_ptr
    server::connection_ptr con = srv.get_con_from_hdl(hdl);
    std::string query = con->get_resource();
    std::vector<std::string> qs;
    boost::split(qs, query, boost::is_any_of("?"));
    std::string path = qs[0];
    std::string response;
    auto code = websocketpp::http::status_code::not_found;
    if (hs.find(path) == hs.end()) {
        std::string filename = path;
        std::ifstream file;
        if (filename == "/") {
            filename = docroot + "index.html";
        } else {
            filename = docroot + filename.substr(1);
        }
        V_LOG_D("WS_ onhttp try to get file(%s)", filename.c_str());
        file.open(filename.c_str(), std::ios::in);
        if (!file) {
            // 404 error
            std::stringstream ss;
            ss << "<!doctype html><html><head>"
               << "<title>Error 404 (Resource not found)</title><body>"
               << "<h1>Error 404</h1>"
               << "<p>The requested URL " << filename << " was not found on this server.</p>"
               << "</body></head></html>";
            response = ss.str();
            code = websocketpp::http::status_code::not_found;
        } else {
            file.seekg(0, std::ios::end);
            response.reserve(file.tellg());
            file.seekg(0, std::ios::beg);
            response.assign((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
            file.close();
            code = websocketpp::http::status_code::ok;
        }
    } else {
        Args args;
        if (qs.size() > 1) {
            args = parseQuery(qs[1]);
        }
        auto reply = hs[path](con, args);
        code = reply.first;
        response = reply.second;
    }
    con->set_body(response);
    con->set_status(code);
}
Args parseQuery(std::string query) {
    Args args;
    std::vector<std::string> qs;
    boost::split(qs, query, boost::is_any_of("&"));
    BOOST_FOREACH (const std::string& q, qs) {
        std::vector<std::string> val;
        boost::split(val, q, boost::is_any_of("="));
        if (val.size() > 1) {
            args[val[0]] = val[1];
        }
    }
    return args;
}
///
}
}
