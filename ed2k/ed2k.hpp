//
//  ed2k.hpp
//  ed2k
//
//  Created by Centny on 1/1/17.
//
//

#ifndef ed2k_hpp
#define ed2k_hpp
#include <boost-utils/boost-utils.hpp>

namespace emulex {
namespace ed2k {
using namespace butils::netw;
class ED2K_;
typedef boost::shared_ptr<ED2K_> ED2K;

class ED2K_ : public CmdH_, public ConH_, public boost::enable_shared_from_this<ED2K_> {
   protected:
    asio::io_service &ios;
    Connector con;

   protected:
    void login();

   public:
    ED2K_(asio::io_service &ios);
    virtual void connect(const char *addr, unsigned short port, boost::system::error_code &err);
    virtual bool OnConn(TCP s, const boost::system::error_code &ec);
    // event on socket close.
    virtual void OnClose(TCP s, const boost::system::error_code &ec);
    virtual int OnCmd(Cmd c);
};
}
}

#endif /* ed2k_hpp */
