package main

import "fmt"

//import "time"

func main() {
	messages := make(chan string)
	signals := make(chan bool)

	/*
		go func() {
			messages <- "hi"
		}()

		time.Sleep(time.Second)
	*/

	select {
	case msg := <-messages:
		fmt.Println("received message", msg)
	default:
		fmt.Println("no message recevied")
	}

	msg := "hi"

	select {
	case messages <- msg:
		fmt.Println("send message", msg)
	default:
		fmt.Println("no message sent")
	}

	select {
	case msg := <-messages:
		fmt.Println("received message", msg)
	case sig := <-signals:
		fmt.Println("recevied signal", sig)
	default:
		fmt.Println("no activity")
	}
}
