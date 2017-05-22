import machine
import time
import os


print("Files on nodeMCU")
for x in os.listdir():
    print(x)

print("Starting PWM")

servo = machine.PWM(machine.Pin(12), freq=50)
servo.duty(40)
servo.duty(115)
servo.duty(77)

for x in range(40, 115):
    time.sleep(0.02)
    servo.duty(x)

print ("Finish")

