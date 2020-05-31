.PHONY: serial
serial:
	platformio run -t upload -e serial

.PHONY: ota
ota:
	platformio run -t upload -e ota
