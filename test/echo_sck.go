package main

import (
	"encoding/base64"
	"encoding/binary"
	"fmt"
	"io"
	"log"
	"net"
)

func main() {
	// Listen on TCP port 2000 on all interfaces.
	l, err := net.Listen("tcp", ":20000")
	if err != nil {
		log.Fatal(err)
	}
	// fmt.Printf("%v\n", 0xE3)
	defer l.Close()
	for {
		// Wait for a connection.
		conn, err := l.Accept()
		if err != nil {
			log.Fatal(err)
		}
		// Handle the connection in a new goroutine.
		// The loop then returns to accepting, so that
		// multiple connections may be served concurrently.
		go func(c net.Conn) {
			// Echo all incoming data.
			io.Copy(&xx{}, c)
			// Shut down the connection.
			c.Close()
		}(conn)
	}
}

type xx struct {
}

func (x *xx) Write(p []byte) (n int, err error) {
	buf := ""
	for _, b := range p {
		buf += fmt.Sprintf("%2x ", b)
	}
	fmt.Printf("D:%v\n", buf)
	fmt.Printf("1:%v\n", p[0])
	fmt.Printf("2:%v\n", binary.LittleEndian.Uint32(p[1:5]))
	fmt.Printf("3:%v\n", p[5])
	fmt.Printf("4:%v->%v\n", base64.StdEncoding.EncodeToString(p[6:22]), p[6:22])
	fmt.Printf("5:%v\n", binary.LittleEndian.Uint32(p[22:26]))
	fmt.Printf("6:%v\n", binary.LittleEndian.Uint16(p[26:28]))
	fmt.Printf("7:%v\n", binary.LittleEndian.Uint32(p[28:32]))

	fmt.Printf("E:%v\n", p[32:])
	return len(p), nil
}
