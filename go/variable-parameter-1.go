package main

import "bytes"
import "fmt"

func joinStrings(slist ...string) string {
	var b bytes.Buffer

	for _, s := range slist {
		b.WriteString(s)
	}

	return b.String()
}

func main() {
	fmt.Println(joinStrings(" pig", " and", " rat"))

	fmt.Println(joinStrings(" bammer", " mom", " and", " hawk"))
}
