//
//  kadx_protocol.cpp
//  emule-x
//
//  Created by Centny on 2/6/17.
//
//

#include "kadx_protocol.hpp"

namespace emulex {
namespace protocol {
ModH BuildMod() {
    auto mod = ModH(new M1L4());
    M1L4 *m = (M1L4 *)mod.get();
    m->magic[0] = OPX_KADX_HEAD;
    m->magic[1] = OPX_KADX_HEAD_C;
    m->big = false;
    return mod;
}
}
}
