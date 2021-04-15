package main

import "crypto/md5"
import "crypto/sha1"
import "fmt"
import "io"
import "os"

func main() {
	TestFile := "123.txt"

	infile, inerr := os.Open(TestFile)

	if inerr == nil {
		md5h := md5.New()
		io.Copy(md5h, infile)
		fmt.Printf("%x %s\n", md5h.Sum([]byte("")), TestFile)

		sha1h := sha1.New()
		io.Copy(sha1h, infile)
		fmt.Printf("%x %s\n", sha1h.Sum([]byte("")), TestFile)
	} else {
		fmt.Println(inerr)
		os.Exit(1)
	}
}
