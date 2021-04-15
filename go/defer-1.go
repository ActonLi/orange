package main

import "fmt"

func main() {
	fmt.Println("defer begin")

	defer fmt.Println(1)

	defer fmt.Println(2)

	fmt.Println("defer end")
}
