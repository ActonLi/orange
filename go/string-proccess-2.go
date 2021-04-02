package main

import "fmt"
import "strings"

func StringProccess(list []string, chain []func(string) string) {
	for index, str := range list {
		result := str

		for _, proc := range chain {
			result = proc(result)
		}

		list[index] = result
	}
}

func removePrefix(str string) string {
	return strings.TrimPrefix(str, "go")
}

func main() {
	list := []string{"go scanner", "go parser", "go compiler", "go printer", "go formater"}

	chain := []func(string) string{
		removePrefix,
		strings.TrimSpace,
		strings.ToUpper,
	}

	StringProccess(list, chain)

	for _, str := range list {
		fmt.Println(str)
	}

}
