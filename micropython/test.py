import machine
import time

servo = machine.PWM(machine.Pin(12), freq=50)
servo.duty(40)
servo.duty(115)
servo.duty(77)

for x in range(40, 115):
    time.sleep(0.08)
    servo.duty(x)


