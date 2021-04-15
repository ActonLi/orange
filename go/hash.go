package main

import "crypto/md5"
import "crypto/sha1"
import "fmt"

func main() {
	TestString := "http://c.biancheng.net/golang/"

	Md5Inst := md5.New()

	Md5Inst.Write([]byte(TestString))

	Result := Md5Inst.Sum([]byte(""))

	fmt.Printf("%x\n\n", Result)

	Sha1Inst := sha1.New()

	Sha1Inst.Write([]byte(TestString))

	Result = Sha1Inst.Sum([]byte(""))

	fmt.Printf("%x\n\n", Result)
}
