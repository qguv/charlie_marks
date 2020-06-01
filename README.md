# Display text on a huzzah32 + featherwing charlieplexed LED matrix

Initial setup:

1. install the platformio core CLI tools
2. plug a huzzah32 into your computer
3. run `platformio -t upload -e serial`
4. connect to the device's wifi network, which will be esp-xxxxxx. Take note of this name!
5. visit http://192.168.4.1
6. enter wifi credentials and some messages that the device should request. separate messages with full stops.
7. click 'save'
8. click 'Restart device'

To configure your machine to send firmware updates over WLAN:

9. open platformio.ini
10. set `upload_port` to the name of the device's wifi network, with '.local' appended
11. set the `--auth` flag in `upload_flags` to the password for the device's wifi network

Now you can run `platformio -t upload -e ota` to update the device as long as the device is reachable from your computer's network.
