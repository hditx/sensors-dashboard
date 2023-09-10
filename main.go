package main

import (
	"database/sql"
	"fmt"
	"log"
	"os"
	"os/signal"
	"time"

	MQTT "github.com/eclipse/paho.mqtt.golang"
	_ "github.com/go-sql-driver/mysql"
)

var db *sql.DB

func initDB() {
	var err error
	db, err = sql.Open("mysql", "usuario:contraseña@tcp(localhost:3306)/dashboard")
	if err != nil {
		log.Fatalf("Error al conectar a la base de datos: %v\n", err)
	}

	err = db.Ping()
	if err != nil {
		log.Fatalf("Error al hacer ping a la base de datos: %v\n", err)
	}
}

func onMessageReceived(client MQTT.Client, msg MQTT.Message) {
	topic := msg.Topic()
	payload := string(msg.Payload())
	_, err := db.Exec("INSERT INTO sensors_info (topic, payload, timestamp) VALUES (?, ?, ?)", topic, payload, time.Now())
	if err != nil {
		log.Printf("Error al insertar en la base de datos: %v\n", err)
	}
}

func main() {
	initDB()
	defer db.Close()

	// Configurar el cliente MQTT
	opts := MQTT.NewClientOptions()
	opts.AddBroker("tcp://localhost:1883")
	opts.SetClientID("mqtt-to-db")
	opts.SetCleanSession(true)

	client := MQTT.NewClient(opts)
	if token := client.Connect(); token.Wait() && token.Error() != nil {
		log.Fatalf("Error al conectar al broker MQTT: %v\n", token.Error())
	}

	// Suscribirse a los temas "humedad" y "temperatura"
	if token := client.Subscribe("humedad", 0, onMessageReceived); token.Wait() && token.Error() != nil {
		log.Fatalf("Error al suscribirse al tema 'humedad': %v\n", token.Error())
	}
	if token := client.Subscribe("temperatura", 0, onMessageReceived); token.Wait() && token.Error() != nil {
		log.Fatalf("Error al suscribirse al tema 'temperatura': %v\n", token.Error())
	}

	fmt.Println("Escuchando los temas MQTT. Presiona Ctrl+C para salir.")

	// Esperar una señal para terminar el programa
	c := make(chan os.Signal, 1)
	signal.Notify(c, os.Interrupt)
	<-c

	// Desconectar el cliente MQTT
	client.Disconnect(250)
}
