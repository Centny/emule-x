//
//  kadx_opcodes.h
//  emule-x
//
//  Created by Centny on 2/7/17.
//
//

#pragma once

#define OPX_KADX_HEAD 0xA1
#define OPX_KADX_HEAD_C 0xA2
#define OPX_LOGIN 0x10
#define OPX_LOGIN_BACK 0x11

#define OPX_SEARCH 0x14
#define OPX_SEARCH_BACK 0x15
#define OPX_SEARCH_BC 0x16

#define HASH_UID_L 20

#define OPX_SSRC 0x18
#define OPX_FSRC 0x19

#define OPX_CALLBACK 0x1B


#define OPX_HOLE 0x20
#define OPX_HOLE_CALLBACK 0x21
#define OPX_HOLE_PENDING 0x22
#define OPX_HOLING 0x23

#define OPX_FILE_STATUS 0x30
#define OPX_FILE_STATUS_BACK 0x31
#define OPX_FILE_PART 0x32
#define OPX_FILE_PROC 0x33

