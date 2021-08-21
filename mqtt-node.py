import paho.mqtt.client as paho

broker="192.168.1.106"
port=1883

database = { "9A439680": (100, 30),"63BEB903":(20,50)}

def on_publish(client,userdata,result):
    print("data published \n")

def on_message(client, userdata, message):
    id = message.payload.decode('utf-8')
    client.publish("tempDepth",str(database[id][0])+"m"+str(database[id][1]))
    print(str(database[id][0])+" "+str(database[id][1]))

client= paho.Client("client")
client.on_message= on_message
client.on_publish = on_publish
client.username_pw_set(username='tara', password='1')
client.connect(broker,port=port,keepalive=60)
print("connected to the broker")
client.subscribe("rfid")
client.loop_forever()




