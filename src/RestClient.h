#ifndef __REST_CLIENT_H__
#define __REST_CLIENT_H__

#include <list>

#ifdef DEBUG_REST
#define REST_DEBUGP(...) Serial.print(__VA_ARGS__)
#define REST_DEBUGF(...) Serial.printf(__VA_ARGS__)
#define REST_DEBUGLN(...) Serial.println(__VA_ARGS__)
#else
#define REST_DEBUGP(...)
#define REST_DEBUGF(...)
#define REST_DEBUGLN(...)
#endif

namespace Palatis {

template < typename CLIENT_T, int DEFAULT_PORT = 80 >
class RestClient {
  public:
    /**
     * construct a RestClient object with default values to the client
     *
     * @param host remote host
     * @param port remote port
     */
    RestClient(const char * const host, uint16_t const port = DEFAULT_PORT):
        _pclient(new CLIENT_T()),
        _client(*_pclient),
        _host(host), _port(port),
        _headers(0),
        _contentType("application/x-www-form-urlencoded")
    {
    }

    /**
     * construct a RestClient object with a reference to a specific client
     *
     * @param client the client to be used, this object holds a reference to
     *               it through out it's life.
     * @param host remote host
     * @param port remote port
     */    
    RestClient(CLIENT_T & client, const char * const host, uint16_t const port = DEFAULT_PORT):
        _pclient(nullptr),
        _client(client),
        _host(host), _port(port),
        _headers(0),
        _contentType("application/x-www-form-urlencoded")
    {
    }
    
    ~RestClient() {
        delete _pclient;
    }
    
    

    /**
     * add a new header to the back of the headers. O(1)
     *
     * @param key the key of the header
     * @param value the value of the header
     */
    void addHeader(String const key, String value) {
        _headers.push_back(std::make_pair(key, value));
    }
    
    /**
     * remove the first occurance of a specific header with key. O(n)
     *
     * @return if we removed something or not
     * @param key the key to remove
     */
    bool removeHeader(String const key) {
        for (auto i = _headers.begin();i != _headers.end(); ++i) {
            if (key == i->first) {
                _headers.erase(i);
                return true;
            }
        }
        return false;
    }
    
    /**
     * get the number of headers set. C++98: up to O(n), C++11: O(1)
     *
     * @return number of headers
     */
    std::list<std::pair<String, String>>::size_type numHeaders() {
        return _headers.size();
    }
    
    /**
     * get the header at position. O(n)
     * note: this does not guard against pass-the-end iterators.
     *
     * @return the iterator to the item at specific position.
     *         this iterator will hold valid even if you add/remove another item.
     */
    std::list<std::pair<String, String>>::iterator 
    getHeader(std::list<std::pair<String, String>>::size_type position) {
        auto i = _headers.begin();
        std::advance(i, position);
        return i;
    }
    
    /**
     * get the first occurance of header with specific key.
     * if you have multiple headers with same key, you should iterate through the list and
     * retrieve them with {@link getHeader(std::list<std::pair<String, String>>::size_type)}
     * by yourself.
     *
     * @return the iterator to the first occurance of item with specific key, or
     *         {@link getHeadersEnd()} if not found.
     */
    std::list<std::pair<String, String>>::iterator getHeader(String key) {
        auto i = _headers.begin();
        for (;i != _headers.end();++i)
            if (i->first == key)
                break;
        return i;
    }
    
    /**
     * get the iterator to the begin of the headers list
     *
     * @return the iterator to the begin of the headers list
     */
    std::list<std::pair<String, String>>::iterator getHeadersBegin() {
        return _headers.begin();
    }
    
    /**
     * get the iterator to the end of the headers list
     *
     * @return the iterator to the end of the headers list
     */
    std::list<std::pair<String, String>>::iterator getHeadersEnd() {
        return _headers.end();
    }
    
    /**
     * Set Content-Type Header
     *
     * @param contentType the new Content-Type
     */
    void setContentType(const char * const contentType) {
        _contentType = contentType;
    }

    /**
     * GET path
     *
     * @param path the path
     */
    String get(const char * const path) {
        return request("GET", path, nullptr);
    }

    /**
     * POST path and body
     *
     * @param path the path
     * @param body the body
     */
    String post(const char * const path, const char * const body) {
        return request("POST", path, body);
    }

    /**
     * PUT path and body
     *
     * @param path the path
     * @param body the body
     */
    String put(const char * const path, const char * const body) {
        return request("PUT", path, body);
    }

    /**
     * DELETE path and body
     *
     * @param path the path
     * @param body the body, default to NULL.
     */
    String del(const char * const path, const char * const body = nullptr) {
        return request("DELETE", path, body);
    }

    /**
     * Generic HTTP Request
     *
     * @return response from remote, or String("FAILED") if the connection failed.
     * @param method the method to be used
     * @param path the path
     * @param body the body
     */
    String request(const char * const method, const char * const path, const char * const body) {
        REST_DEBUGF("query(): free heap = %d\n", ESP.getFreeHeap());
        if (!_client.connect(_host, _port)) {
            REST_DEBUGP("Connection Failed\n");
            _client.stop();
            return "FAILED";
        }

        // write the request
        _print(method)._print(' ')._print(path)._print(" HTTP/1.1\r\n");
        _print("Host: ")._print(_host)._print(':')._print(_port)._print("\r\n");
        _print("Connection: close\r\n");

        for (auto i = _headers.begin();i != _headers.end();++i)
            _print(i->first)._print(": ")._print(i->second)._print("\r\n");

        if (body == NULL) {
            _print("Content-Length: 0\r\n");
            _print("Content-Type: ")._print(_contentType)._print("\r\n");
        } else {
            _print("Content-Length: ")._print(strlen(body))._print("\r\n");
            _print("Content-Type: ")._print(_contentType)._print("\r\n");
            _print("\r\n");
            _print(body);
            _print("\r\n");
        }

        _print("\r\n");

        size_t start_millis = millis();
        while (_client && !_client.available() && millis() - start_millis < 5000)
            delay(0);
        if (millis() - start_millis > 5000) {
            REST_DEBUGP("Connection Timeout\n");
            _client.stop();
            return "TIMEOUT";
        }

        // read back the response
        String response;
        while (_client.available()) {
            response += (char)_client.read();
            delay(0);
        }
        _client.stop();
        
        REST_DEBUGLN(response);
        return response;
    }

    const char * const getHost() const {
        return _host;
    }
    
    uint16_t const getPort() const {
        return _port;
    }

  private:
    CLIENT_T * const _pclient;
    CLIENT_T & _client;

    template<typename T>
    inline RestClient<CLIENT_T, DEFAULT_PORT> & _print(T const data) {
        REST_DEBUGP(data);
        _client.print(data);
        delay(0);
        return *this;
    }
    
    const char * const _host;
    uint16_t const _port;
    std::list<std::pair<String, String>> _headers;
	const char * _contentType;
};

} // namespace Palatis

#undef REST_DEBUGP
#undef REST_DEBUGF
#undef REST_DEBUGLN

#endif