package main

import "errors"
import "fmt"

var errDivisionByZero = errors.New("division by zero")

func div(dividend, divisor int) (int, error) {
	if divisor == 0 {
		return 0, errDivisionByZero
	}

	return dividend / divisor, nil
}

func main() {
	fmt.Println(div(1, 0))
}
