package main

import "fmt"
import "time"

func test() {
	start := time.Now()

	sum := 0

	for i := 0; i < 100000000; i++ {
		sum++
	}

	elapsed := time.Since(start)

	fmt.Println("该函数执行完成耗时：", elapsed)
}

func main() {
	test()
}
