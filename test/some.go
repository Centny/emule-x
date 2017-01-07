package main

import "fmt"
import "encoding/binary"

func main() {
	fmt.Printf("%x\n", "splayer.exe")
	fmt.Printf("%v\n", len("splayer.exe"))
	fmt.Printf("%v\n", len("splayer.exe.reg"))
	fmt.Printf("%x\n", "splayer.cht.dll")
	var x uint32 = 77824
	fmt.Printf("%x\n", x)
	buf := make([]byte, 4)
	binary.LittleEndian.PutUint32(buf, 77824)
	fmt.Printf("a-%x\n", buf)
	binary.LittleEndian.PutUint32(buf, 12841768)
	fmt.Printf("a-%x\n", buf)
	binary.LittleEndian.PutUint32(buf, 60736)
	fmt.Printf("a-%x\n", buf)
	buf[0] = 0x9b
	fmt.Printf("%d\n", uint8(buf[0]))
	fmt.Printf("%s\n", string([]byte{0x78, 0x65, 0x83}))
	fmt.Printf("%s\n", string([]byte{0x9b}))
	fmt.Printf("%d\n", 0x98)
	fmt.Printf("%d\n", 0x98)
	//60736
	//12841768
	fmt.Printf("%v\n", binary.LittleEndian.Uint32([]byte{0x40, 0xed, 0x89, 0x15}))
	//
	fmt.Printf("%v\n", 0x9f-0x9b)
	fmt.Printf("%v\n", 0x9f-15)
	fmt.Printf("%v\n", 0x9b-11)
	fmt.Printf("%v\n", 0x82)
	//
	var val uint8
	fmt.Println()
	val = 0x98
	fmt.Printf("b-%x\n", val)
	fmt.Printf("b-%v\n", val&0x80)
	fmt.Printf("b-%v\n", val&0x7F)
	fmt.Printf("b-%x\n", val&0x7F)
	fmt.Println()
	val = 0x9f
	fmt.Printf("c-%x\n", val)
	fmt.Printf("c-%v\n", val&0x80)
	fmt.Printf("c-%v\n", val&0x7F)
	fmt.Printf("c-%x\n", val&0x7F)

	//
	fmt.Printf("%v\n", binary.LittleEndian.Uint32([]byte{0x78, 0xda, 0xe3, 0x66}))
	fmt.Printf("%v\n", binary.LittleEndian.Uint16([]byte{0x78, 0xda}))
}
