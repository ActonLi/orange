package main

import "fmt"
import "time"

func main() {
	result := 0
	start := time.Now()

	for i := 1; i <= 40; i++ {
		result = fibonacci(i)
		fmt.Printf("数列第 %d 位: %d\n", i, result)
	}

	end := time.Now()
	delta := end.Sub(start)
	fmt.Printf("程序执行时间为: %s\n", delta)
}

func fibonacci(n int) (res int) {
	if n <= 2 {
		res = 1
	} else {
		res = fibonacci(n-1) + fibonacci(n-2)
	}

	return
}
