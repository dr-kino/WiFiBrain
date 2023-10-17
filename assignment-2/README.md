# AP and Webserver with sensor’s reads
* Considering the reference scenario presented in the previous slide and two Arduinos:
	* Use one of the Arduinos to create an 802.11g access point in a user-selectable Wi-Fi channel (chosen by the user via serial console).

	* Use the other Arduino to build a simple webserver providing users with information about local humidity and temperature readings. The actual temperature and humidity values will be emulated, provided via the Arduino serial console (define the mechanisms to do so)
	
	* Test the setup using a cell phone or a PC connected to the same 802.11 networks and using a regular browser to get the temperature and humidity readings.

	* On the Access Point, provide via the serial console status indicators such as:
		* used channel and SSID 
		* connected devices
		* etc