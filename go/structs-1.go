package main

import "fmt"

func printMsgType(msg *struct {
	id   int
	data string
}) {
	fmt.Printf("%T\n", msg)
}

func main() {
	msg := &struct {
		id   int
		data string
	}{
		1024,
		"hello",
	}

	printMsgType(msg)
}
