package main

import (
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
		tc, _ := net.Dial("tcp", "loc.w:4122")
		go func(c1, c2 net.Conn) { //write
			// Echo all incoming data.
			io.Copy(io.MultiWriter(c2, &W{N: "W"}), c1)
			// Shut down the connection.
			c1.Close()
			c2.Close()
		}(conn, tc)
		go func(c1, c2 net.Conn) { //read
			// Echo all incoming data.
			io.Copy(io.MultiWriter(c1, &W{N: "R"}), c2)
			// Shut down the connection.
			c1.Close()
			c2.Close()
		}(conn, tc)
	}
}

type W struct {
	N string
}

func (w *W) Write(p []byte) (n int, err error) {
	buf := ""
	for _, b := range p {
		buf += fmt.Sprintf("%2x ", b)
	}
	fmt.Printf("%s->%d->D:%v\n", w.N, len(p), buf)
	n = len(p)
	return
}
