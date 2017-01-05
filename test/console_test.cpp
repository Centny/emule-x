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
int main(int argc, char **argv) {
    edk_console(argc, argv);
    return 0;
}
