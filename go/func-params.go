package main

import "fmt"

type InnerData struct {
	a int
}

type Data struct {
	complax  []int
	instance InnerData
	ptr      *InnerData
}

func passByValue(inFunc Data) Data {
	fmt.Printf("inFunc value: %+v\n", inFunc)
	fmt.Printf("inFunc ptr: %p\n", &inFunc)

	return inFunc
}

func main() {
	in := Data{
		complax: []int{1, 2, 3},
		instance: InnerData{
			5,
		},
	}

	fmt.Printf("in value: %+v\n", in)
	fmt.Printf("in ptr: %p\n", &in)

	out := passByValue(in)

	fmt.Printf("out value: %+v\n", out)
	fmt.Printf("out ptr: %p\n", &out)
}
