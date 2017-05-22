# configure a specific network interface
# see below for examples of specific drivers
import network


# enable station interface and connect to WiFi access point
nic = network.WLAN(network.STA_IF)
nic.active(True)

for x in nic.scan():
    print(str(x))
nic.connect('RadiationG', 'polkalol')
# now use sockets as usual
print (nic.status())
print (nic.ifconfig())
nic.active(False)

