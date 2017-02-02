//
//  console_test.cpp
//  boost-utils
//
//  Created by Centny on 1/3/17.
//
//

#include <errno.h>
#include <stdio.h>
int edk_console(int argc, char **argv);
int ed2k_md4(int argc, char **argv);
int runner_c(int argc, char **argv);
int main(int argc, char **argv) {
    //    edk_console(argc, argv);
    //    ed2k_md4(argc, argv);
    runner_c(argc, argv);
    return 0;
}
