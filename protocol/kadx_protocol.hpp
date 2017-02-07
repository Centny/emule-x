//
//  kadx_protocol.hpp
//  emule-x
//
//  Created by Centny on 2/6/17.
//
//

#ifndef kadx_protocol_hpp
#define kadx_protocol_hpp
#include <lzma.h>
#include <boost-utils/boost-utils.hpp>
#include <boost/asio/streambuf.hpp>
#include <boost/endian/buffers.hpp>
#include <boost/foreach.hpp>
#include <iostream>
#include <list>
#include <map>
#include <sstream>
#include <string>
#include "../encoding/encoding.hpp"
#include "../fs/fs.hpp"
namespace emulex {
namespace protocol {
#include "kadx_opcodes.h"
using namespace boost;
using namespace butils::netw;
using namespace emulex::fs;
using namespace emulex::encoding;
ModH BuildMod();
}
}
#endif /* kadx_protocol_hpp */
