# Send an HTTP GET request from an m5stack atom board

Initial setup:

1. install the platformio core CLI tools
2. plug an m5atom into your computer
3. run `platformio -t upload -e serial`
4. press and hold the screen, then tap the reset button—the screen will pulse red/white
5. connect to the m5atom's wifi network, which will be esp-xxxxxx. Take note of this name!
6. visit http://192.168.4.1
6. enter wifi credentials and a URL that the device should request
7. click 'save'
8. click 'Restart device'
9. press the screen to send the HTTP request

To configure your machine to send firmware updates over WLAN:

10. open platformio.ini
11. set `upload_port` to the name of the device's wifi network, with '.local' appended
12. set the `--auth` flag in `upload_flags` to the password for the device's wifi network

Now you can run `platformio -t upload -e ota` to update the device as long as the device is reachable from your computer's network.
