# configure a specific network interface
# see below for examples of specific drivers
import network


# enable station interface and connect to WiFi access point
ap = network.WLAN(network.AP_IF)
ap.config(essid='nodeMCU', channel=11, authmode=network.AUTH_WPA_WPA2_PSK, password='polkalol')
ap.active(True)
