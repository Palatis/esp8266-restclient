# RestClient for Arduino ESP8266 WiFi modules

HTTP Request library for Arduino and the ESP8266 WiFi SOC modules
This library is derived almost entirely from the great work done here: https://github.com/DaKaZ/esp8266-restclient

# Install

Clone (or download and unzip) the repository to `~/Documents/Arduino/libraries`
where `~/Documents/Arduino` is your sketchbook directory.

    > cd ~/Documents/Arduino
    > mkdir libraries
    > cd libraries
    > git clone https://github.com/palatis/esp8266-restclient.git RestClient

# Usage

### Include

You need to have the `ESP8266` board support already included.

### instantiate a RestClient<CLIENT_T, DEFAULT_PORT>

RestClient is generic, to be able to support different types of clients.
so you may use a basic WiFiClient, or a ssl-enabled WiFiClientSecure, or even HardwareSerial with a wrapper class.
the CLIENT_T should support these member functions and constructors:
- CLIENT_T::CLIENT_T()
- CLIENT_T::print(const char * const)
- CLIENT_T::print(const char)
- CLIENT_T::print(int)
- CLIENT_T::available()
- CLIENT_T::bool()
- CLIENT_T::read()
- CLIENT_T::connect(const char * const host, uint16_t port)
- CLIENT_T::stop()

if you don't give it a client, it will make one for you (will call the default c-tor of CLIENT_T).
```c++
// defaults to 80 port
Palatis::RestClient<WiFiClient, 80> restClient("api.twitch.tv");
// to a specif port
Palatis::RestClient<WiFiClient, 80> restClient("api.twitch.tv", 8080);
```

or you can give it a client
```c++
WiFiClientSecure wifiClient;
Palatis::RestClient<WiFiClientSecure, 443> restClient(wifiClient, "api.twitch.tv");
```

## RESTful methods

All methods return the response string directly, or "FAILED" when connection fails to establish.

### get(const char * const path)

Start making requests!

```c++
String response = client.get("/");
```

### post(const char * const path, const char * const body)
### put(const char * const path, const char * const body)

```
String response;
response = client.post("/", "foo=bar");
response = client.put("/", "foo=bar");
```

### del(const char* path, const char * const body = nullptr)

```
String response;
response = client.del("/");
response = client.del("/blah", "foo=bar");
```

## Debug Mode

If you're having trouble, you can always open `RestClient.cpp` and throw at the top:

```c++
#define DEBUG_REST
```

Everything happening in the client will get printed to the Serial port.

# Thanks

[csquared](https://github.com/csquared) For the original arduino-restclient
[DaKaZ](https://github.com/DaKaZ) For porting arduino-restclient to esp8266