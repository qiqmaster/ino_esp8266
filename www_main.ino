/**************************************************************************************
  Arduino ESP8266 WWW Main
 **************************************************************************************/
#include <ESP8266WiFi.h>
#include <map>
#include <vector>
#define string String
#include <FS.h>
const double VERSION_MAIN   = 6.5,
             VERSION_CODE   = 6.2,
             VERSION_EXTRA  = 180316;
/**************************************************************************************
  Utils
 *******/
class Utils
{
  public:
    /**********************************************
      String
     **********************************************/
    const int indexOf(const string& _s, const int& _start, const char& _c)
    {
      for (int i = _start; i < _s.length(); i++)
        if (_s[i] == _c) return i;
      return -1;
    }
    const int indexOf(const string& _s, const char& _c)
    {
      return indexOf(_s, 0, _c);
    }
    const string substring(const string& _s, const int& _start, const int& _end)
    {
      string out = "";
      if (_start >= 0 && _start < _end && _end <= _s.length())
        for (int i = _start; i < _end; i++)
          out += _s[i];
      return out;
    }
    const string substring(const string& _s, const int& _end)
    {
      return substring(_s, 0, _end);
    }
    const std::vector<string> split(const string& _s, const char& _c)
    {
      std::vector<string> out;
      string cur = "";
      for (int i = 0; i < _s.length() + 1; i++)
      {
        if (_s[i] == _c || i == _s.length())
        {
          out.insert(out.end(), cur);
          cur = "";
          continue;
        }
        cur += _s[i];
      }
      return out;
    }
    const string trim(const string& _s)
    {
      string out = _s;
      if (out[0] == ' ')
        out = substring(out, 1, out.length());
      if (out[out.length() - 1] == ' ')
        out = substring(out, 0, out.length() - 1);
      return out;
    }
    /**********************************************
      Number
     **********************************************/
    const int pow(const int& _in, const int& _pow)
    {
      if (_pow < 1) return 1;
      if (_pow == 1) return _in;
      int out = _in;
      for (int i = 1; i < _pow; i++) out *= _in;
      return out;
    }
    const int str2int(const string& _s)
    {
      int out = 0, _pow = 0;
      for (int i = _s.length() - 1; i >= (_s[0] == '-' ? 1 : 0); i--)
      {
        if (_s[i] < '0' || _s[i] > '9') return -1;
        out += pow(10, _pow) * (_s[i] - '0');
        _pow++;
      }
      if (_s[0] == '-') out *= -1;
      return out;
    }
    const double str2double(const String& _s)
    {
      std::vector<string> nums = split(_s, '.');
      int i0 = str2int(nums[0]),
          i1 = str2int(nums.size() > 1 ? nums[1] : "0");
      if (i0 < 0) return -1;
      if (i1 < 0) return -0.1;
      double out = 1;
      out /= pow(10, nums[0].length());
      out *= i1;
      out += i0;
      return out;
    }
    const string int2str(const int& _i)
    {
      return string(_i, DEC);
    }
    const string double2str(const double& _d)
    {
      return string(_d, DEC);
    }
    /**********************************************
      URL
     **********************************************/
    const string encodeURL(const string& _s)
    {
      string out = "";
      for (int i = 0; i < _s.length(); i++)
      {
        int z = _s[i];
        if (z < 0)
          z += 256;
        if (z == ' ')
        {
          out += '+';
          continue;
        }
        out += '%';
        int c1 = z % 16;
        int c2 = (z - c1) / 16;
        if (c2 >= 0 && c2 <= 9)
          out += '0' + c2;
        if (c2 >= 10 && c2 <= 15)
          out += 'a' - 10 + c2;
        if (c1 >= 0 && c1 <= 9)
          out += '0' + c1;
        if (c1 >= 10 && c1 <= 15)
          out += 'a' - 10 + c1;
      }
      return out;
    }
    const string decodeURL(const string& _s)
    {
      string out = "";
      for (int i = 0; i < _s.length(); i++)
      {
        if (_s[i] == '%')
        {
          if (i + 1 > _s.length())
            return string();
          int tmp = 0;
          for (int j = 0; j < 2; j++)
          {
            if (_s[i + j + 1] >= '0' && _s[i + j + 1] <= '9')
              tmp += (_s[i + j + 1] - '0') * (j == 0 ? 16 : 1);
            else if (_s[i + j + 1] >= 'a' && _s[i + j + 1] <= 'f')
              tmp += (_s[i + j + 1] - 'a' + 10) * (j == 0 ? 16 : 1);
            else if (_s[i + j + 1] >= 'A' && _s[i + j + 1] <= 'F')
              tmp += (_s[i + j + 1] - 'A' + 10) * (j == 0 ? 16 : 1);
            else
              return string();
          }
          out += (char)tmp;
          i += 2;
        }
        else if (_s[i] == '+')
          out += ' ';
        else
          out += _s[i];
      }
      return out;
    }
    Utils() {}
};
Utils utils;
/**************************************************************************************
  Formatter
 ***********/
class Formatter
{
  private:
    std::vector<string> _values;
  public:
    template<typename T> void add(const T& _t)
    {
      _values.insert(_values.end(), string(_t));
    }
    void reset()
    {
      if (_values.size() > 0)
        _values.clear();
    }
    const string format(const string& _s)
    {
      string out = "", num = "";
      int mode = 0;
      for (int i = 0; i < _s.length(); i++)
      {
        switch (mode)
        {
          case 0:
            if (_s[i] == '[')
            {
              mode = 1;
              continue;
            }
            out += _s[i];
            break;
          case 1:
            if (_s[i] == ']')
            {
              int index = utils.str2int(num);
              if (index < 0 || index > _values.size())
                out += "[#]";
              else
                out += _values[index];
              mode = 0;
              num = "";
              continue;
            }
            if (_s[i] >= '0' && _s[i] <= '9')
            {
              num += _s[i];
              continue;
            }
            mode = 0;
            out += "[";
            out += num;
            i--;
            break;
        }
      }
      return out;
    }
    Formatter() {}
};
/**************************************************************************************
  NanoFS
 ********/
class NanoFS
{
  private:
    bool _mount_reason, _mount;
    Formatter _fmt;
  public:
    NanoFS()
    {
      _mount_reason = false;
      _mount = false;
    }
    const bool mount()
    {
      if (!_mount_reason)
      {
        _mount_reason = true;
        _mount = SPIFFS.begin();
        _fmt.add(_mount ? "ok" : "failed");
        Serial.println(_fmt.format("[SPIFFS] Mounting partition... [0]"));
        _fmt.reset();
      }
      return _mount;
    }
    void unmount() {
      if (_mount_reason && _mount)
      {
        SPIFFS.end();
        _mount_reason = false;
        _mount = false;
        Serial.println("[SPIFFS] Unmounted.");
      }
    }
};
NanoFS nanofs;
/**************************************************************************************
  HTTP
 ********/
static const string HTTP_GET                                 = "GET",
                    HTTP_POST                                = "POST";

static const int    HTTP_MAX_DATA_WAIT                       = 5000,
                    HTTP_MAX_POST_WAIT                       = 5000,
                    HTTP_MAX_SEND_WAIT                       = 5000,
                    HTTP_MAX_CLOSE_WAIT                      = 2000;

typedef enum HTTPStatus
{
  /**********************************************
    Information
   **********************************************/
  HTTP_100_CONTINUE                        = 100,
  HTTP_101_SWITCHING_PROTOCOLS             = 101,
  HTTP_102_PROCESSING                      = 102,
  /**********************************************
    Success
   **********************************************/
  HTTP_200_OK                              = 200,
  HTTP_201_CREATED                         = 201,
  HTTP_202_ACCEPTED                        = 202,
  HTTP_203_NON_AUTHORITATIVE_INFORMATION   = 203,
  HTTP_204_NO_CONTENT                      = 204,
  HTTP_205_RESET_CONTENT                   = 205,
  HTTP_206_PARTIAL_CONTENT                 = 206,
  HTTP_207_MULTISTATUS                     = 207,
  HTTP_208_ALREADY_REPORTED                = 208,
  HTTP_226_IM_USED                         = 226,
  /**********************************************
    Redirections
   **********************************************/
  HTTP_300_MULTIPLE_CHOICES                = 300,
  HTTP_301_MOVED_PERMANENTLY               = 301,
  HTTP_302_MOVED_TEMPORARILY               = 302,
  HTTP_302_FOUND                           = 302,
  HTTP_303_SEE_OTHER                       = 303,
  HTTP_304_NOT_MODIFIED                    = 304,
  HTTP_305_USE_PROXY                       = 305,
  HTTP_307_TEMPORARY_REDIRECT              = 307,
  HTTP_308_PERMANTED_REDIRECT              = 308,
  /**********************************************
    Client error
   **********************************************/
  HTTP_400_BAD_REQUEST                     = 400,
  HTTP_401_UNAUTHORIZED                    = 401,
  HTTP_402_PAYMENT_REQUIRED                = 402,
  HTTP_403_FORBIDDEN                       = 403,
  HTTP_404_NOT_FOUND                       = 404,
  HTTP_405_METHOD_NOT_ALLOWED              = 405,
  HTTP_406_NOT_ACCEPTABLE                  = 406,
  HTTP_407_PROXY_AUTHENTICATION_REQUIRED   = 407,
  HTTP_408_REQUIEST_TIMEOUT                = 408,
  HTTP_409_CONFLICT                        = 409,
  HTTP_410_GONE                            = 410,
  HTTP_411_LENGTH_REQUIRED                 = 411,
  HTTP_412_PRECONDITION_FAILED             = 412,
  HTTP_413_PAYLOAD_TOO_LARGE               = 413,
  HTTP_414_URI_TOO_LONG                    = 414,
  HTTP_415_UNSUPPORTED_MEDIA_TYPE          = 415,
  HTTP_416_RANGE_NOT_SATISFIABLE           = 416,
  HTTP_417_EXPECTATION_FAILED              = 417,
  HTTP_421_MISDIRECTED_REQUEST             = 421,
  HTTP_422_UNPROCESSABLE_ENTITY            = 422,
  HTTP_423_LOCKED                          = 423,
  HTTP_424_FAILED_DEPENDENCY               = 424,
  HTTP_426_UPGRADE_REQUIRED                = 426,
  HTTP_428_PRECONDITION_REQUIRED           = 428,
  HTTP_429_TOO_MANY_REQUESTS               = 429,
  HTTP_431_REQUEST_HEADER_FIELDS_TOO_LARGE = 431,
  HTTP_444_CLOSED_WITHOUT_HEADERS          = 444,
  HTTP_449_RETRY_WITH                      = 429,
  HTTP_451_UNAVAILABLE_FOR_LEGAL_REASONS   = 451,
  /**********************************************
     Server error
   **********************************************/
  HTTP_500_INTERNAL_SERVER_ERROR           = 500,
  HTTP_501_NOT_IMPLEMENTED                 = 501,
  HTTP_502_BAD_GATEWAY                     = 502,
  HTTP_503_SERVICE_UNAVAILABLE             = 503,
  HTTP_504_GATEWAY_TIMEOUT                 = 504,
  HTTP_505_HTTP_VERSION_NOT_SUPPORTED      = 505,
  HTTP_506_VARIANT_ALSO_NEGOTIATES         = 506,
  HTTP_507_INSUFFICIENT_STORAGE            = 507,
  HTTP_508_LOOP_DETECTED                   = 508,
  HTTP_509_BANDWITH_LIMIT_EXCEEDED         = 509,
  HTTP_510_NOT_EXTENDED                    = 510,
  HTTP_511_NETWORK_AUTHENTICATION_REQUIRED = 511,
  HTTP_520_UNKNOWN_ERROR                   = 520,
  HTTP_521_WEB_SERVER_IS_DOWN              = 521,
  HTTP_522_CONNECTION_TIMED_OUT            = 522,
  HTTP_523_ORIGIN_IS_UNREACHABLE           = 523,
  HTTP_524_A_TIMEOUT_OCCURRED              = 524,
  HTTP_525_SSL_HANDSHAKE_FAILED            = 525,
  HTTP_526_INVALID_SSL_CERTIFICATE         = 526
};

class Cookie
{
  private:
    string _value, _path, _domain;
    long _expire;
    bool _secure;
  public:
    const string value()
    {
      return _value;
    }
    const string path()
    {
      return _path;
    }
    void path(const string& _path)
    {
      this->_path = _path;
    }
    const string domain()
    {
      return _domain;
    }
    void domain(const string& _domain)
    {
      this->_domain = _domain;
    }
    const long expire()
    {
      return _expire;
    }
    void expire(const long& _expire)
    {
      this->_expire = _expire;
    }
    const bool secure()
    {
      return _secure;
    }
    void secure(const bool& _secure)
    {
      this->_secure = _secure;
    }
    Cookie(const string& _value)
    {
      _expire = 0;
      _secure = false;
      this->_value = _value;
    }
};

class KeyPair
{
  private:
    std::vector<string> _values;
  public:
    const bool check(const string& _value)
    {
      if (_value.length() < 1 || _values.size() < 1)
        return false;
      if (_values.size() == 1)
        return _values[0] == _value;
      for (int i = 0; i < _values.size(); i++)
        if (_values[i] == _value)
          return true;
      return false;
    }
    const int size()
    {
      return _values.size();
    }
    void add(const string& _value)
    {
      if (!check(_value))
        _values.insert(_values.end(), _value);
    }
    const string get(const int& _index)
    {
      return _index >= 0 && _index < _values.size() ? _values[_index] : string();
    }
    KeyPair() {}
};

typedef enum PairType
{
  QUERY, COOKIE
};

class HTTPConnection
{
  private:
    WiFiClient _client;
    int _clen;
    HTTPStatus _status;
    string _method, _uri, _path, _ctype, _sessionId;
    std::map<string, string> _headers_in, _headers_out, _cookies_in;
    std::map<string, KeyPair> _params;
    std::map<string, Cookie> _cookies_out;
    bool _sendHeaders;

    const bool readPairs(const string& _pairs, const PairType& _type)
    {
      std::vector<string> _pair = utils.split(_pairs, _type == QUERY ? '&' : ';');
      for (int i = 0; i < _pair.size(); i++)
      {
        int idx = utils.indexOf(_pair[i], '=');
        if (idx < 0)
          return false;
        string k = utils.trim(utils.substring(_pair[i], idx)),
               v = utils.trim(utils.substring(_pair[i], idx + 1, _pair[i].length()));
        if (k.length() > 0 && v.length() > 0)
        {
          Formatter fmt;
          fmt.add(_type == QUERY ? "PARAM" : "COOKIE");
          fmt.add(k);
          fmt.add(v);
          Serial.println(fmt.format("[HTTP][REQUEST][[0]] [1]=[2]"));
          if (_type == QUERY)
          {
            std::map<string, KeyPair>::iterator it = _params.find(k);
            KeyPair kp = it != _params.end() ? (*it).second : KeyPair();
            kp.add(v);
            if (it != _params.end())
              (*it).second = kp;
            else
              _params.insert(std::pair<string, KeyPair>(k, kp));
          }
          std::map<string, string>::iterator it = _cookies_in.find(k);
          if (it != _cookies_in.end())
            (*it).second = v;
          else
            _cookies_in.insert(std::pair<string, string>(k, v));
        }
      }
      return true;
    }
    void sendHeaders()
    {
      if (!_sendHeaders)
      {
        Formatter fmt;
        fmt.add(_method);
        fmt.add(_status);
        fmt.add(_uri);
        _client.print(fmt.format("HTTP/1.1 [1]\r\n"));
        Serial.println(fmt.format("[HTTP][[0]][[1]] [2]"));
        header("Content-Length", utils.int2str(_clen));
        header("Content-Type", _ctype);
        if (_status != HTTP_444_CLOSED_WITHOUT_HEADERS)
        {
          for (std::map<string, string>::iterator cur = _headers_out.begin(); cur != _headers_out.end(); cur++)
          {
            fmt.reset();
            fmt.add((*cur).first);
            fmt.add((*cur).second);
            _client.print(fmt.format("[0]: [1]\r\n"));
            Serial.println(fmt.format("[HTTP][RESPONSE][HEADER] [0]: [1]"));
          }
          if (_cookies_out.size() > 0)
            for (std::map<string, Cookie>::iterator cur = _cookies_out.begin(); cur != _cookies_out.end(); cur++)
            {
              Cookie c = (*cur).second;
              fmt.reset();
              fmt.add("Set-Cookie");                                        //[0]
              fmt.add((*cur).first);                                        //[1]
              fmt.add(c.value());                                           //[2]
              fmt.add(c.path().length() > 0 ? "; path=" : "");              //[3]
              fmt.add(c.path());                                            //[4]
              fmt.add(c.domain().length() > 0 ? "; domain=" : "");          //[5]
              fmt.add(c.domain());                                          //[6]
              fmt.add(c.expire() > 1000 ? "; expire=" : "");                //[7]
              fmt.add((c.expire() > 1000 ? string(c.expire()) : ""));                 //[8]
              fmt.add(c.secure() ? "; secure" : "");                        //[9]
              _client.print(fmt.format("[0]: [1]=[2][3][4][5][6][7][8][9]\r\n"));
              Serial.println(fmt.format("[HTTP][RESPONSE][HEADER] [0]: [1]=[2][3][4][5][6][7][8][9]"));
            }
        }
        _client.print("\r\n");
        _sendHeaders = true;
      }
    }
    void header(const string& _name, const string& _value)
    {
      std::map<string, string>::iterator it = _headers_out.find(_name);
      if (it != _headers_out.end())
        (*it).second = _value;
      else
        _headers_out.insert(std::pair<string, string>(_name, _value));
    }
  public:
    const int status()
    {
      return _status;
    }
    void status(const HTTPStatus& _status)
    {
      this->_status = _status;
    }
    const string method()
    {
      return _method;
    }
    const string uri()
    {
      return _uri;
    }
    const string path()
    {
      return _path;
    }
    const KeyPair param(const string& _name)
    {
      std::map<string, KeyPair>::iterator it = _params.find(_name);
      return it != _params.end() ? (*it).second : KeyPair();
    }
    const string header(const string& _name)
    {
      std::map<string, string>::iterator it = _headers_in.find(_name);
      return it != _headers_in.end() ? (*it).second : string();
    }
    const string cookie(const string& _name)
    {
      std::map<string, string>::iterator it = _cookies_in.find(_name);
      return it != _cookies_in.end() ? (*it).second : string();
    }
    void cookie(const string& _name, const Cookie& _cookie)
    {
      if (_name.length() < 1) return;
      std::map<string, Cookie>::iterator it = _cookies_out.find(_name);
      if (it != _cookies_out.end())
        (*it).second = _cookie;
      else
        _cookies_out.insert(std::pair<string, Cookie>(_name, _cookie));
    }
    void redirect(const string& _uri)
    {
      header("Location", _uri);
      send(HTTP_308_PERMANTED_REDIRECT);
    }
    const string sessionId()
    {
      return _sessionId;
    }
    const string generateSessionId()
    {
      string id = "";
      for (int i = 0; i < 16; i++)
      {
        int rnd = rand() % 15;
        id += (char)(rnd >= 0 && rnd <= 9 ? '0' + rnd : 'a' - 10 + rnd);
      }
      return id;
    }
    void contentLength(const int& _clen)
    {
      if (_clen >= 0)
        this->_clen = _clen;
    }
    void contentType(const string& _ctype)
    {
      if (_ctype.length() > 0 && utils.indexOf(_ctype, '/') > 0)
        this->_ctype = _ctype;
    }
    const IPAddress remoteIP()
    {
      return _client.remoteIP();
    }

    void write(const byte& _c)
    {
      if (_client.connected())
      {
        sendHeaders();
        _client.write(_c);
      }
    }
    void print(const string& _s)
    {
      if (_client.connected())
      {
        sendHeaders();
        _client.print(_s);
      }
    }
    void close()
    {
      if (_client.connected())
      {
        sendHeaders();
        _client.flush();
        _client.stop();
        Serial.println("[HTTP] Disconnected");
      }
    }
    void send(const HTTPStatus& _status, const string& _ctype, const string& _content)
    {
      if (!_sendHeaders && _client.connected())
      {
        if (_status >= HTTP_400_BAD_REQUEST)
        {
          Formatter fmt;
          fmt.add(_status);
          fmt.add(VERSION_MAIN);
          fmt.add(VERSION_CODE);
          fmt.add(VERSION_EXTRA);
          send(HTTP_200_OK, "text/html", fmt.format("<html><head><title>Error [0]</title></head><body><h1>Error [0]</h1><hr><i>Arduino HTTPServer v[1]/[2] ([3])</i></body></html>"));
          return;
        }
        status(_status);
        contentType(_ctype);
        contentLength(_content.length());
        sendHeaders();
        _client.print(_content);
        close();
      }
    }
    void send(const HTTPStatus& _status, const string& _content)
    {
      send(_status, _ctype, _content);
    }
    void send(const string& _ctype, const string& _content)
    {
      send(_status, _ctype, _content);
    }
    void send(const string& _content)
    {
      send(_status, _content);
    }
    void send(const HTTPStatus& _status)
    {
      send(_status, string());
    }

    HTTPConnection(WiFiClient& _client)
    {
      this->_client = _client;
      Formatter fmt;
      fmt.add(_client.remoteIP()[0]);
      fmt.add(_client.remoteIP()[1]);
      fmt.add(_client.remoteIP()[2]);
      fmt.add(_client.remoteIP()[3]);
      Serial.println(fmt.format("[HTTP] Connected ([0].[1].[2].[3])"));
      _status = HTTP_200_OK;
      _clen = 0;
      _ctype = "text/plain";
      _sendHeaders = false;
      header("Connection", "close");
      fmt.reset();
      string line = _client.readStringUntil('\r');
      _client.read();
      std::vector<string> hdr = utils.split(line, ' ');
      if (hdr.size() != 3)
      {
        _status = HTTP_406_NOT_ACCEPTABLE;
        _client.stop();
        return;
      }
      if (hdr[0] != HTTP_GET && hdr[0] != HTTP_POST)
      {
        _status = HTTP_405_METHOD_NOT_ALLOWED;
        close();
        return;
      }
      _method = hdr[0];
      _uri = utils.decodeURL(hdr[1]);
      if (_uri.length() < 1)
      {
        _status = HTTP_400_BAD_REQUEST;
        close();
        return;
      }
      int idx = utils.indexOf(_uri, '?');
      if (idx > 0)
      {
        if (!readPairs(utils.substring(_uri, idx + 1, _uri.length()), QUERY))
        {
          _status = HTTP_400_BAD_REQUEST;
          close();
          return;
        }
        _path = utils.substring(_uri, 0, idx);
      } else _path = _uri;
      std::vector<string> prot = utils.split(hdr[2], '/');
      if (prot.size() != 2 || prot[0] != "HTTP")
      {
        _status = HTTP_444_CLOSED_WITHOUT_HEADERS;
        close();
        return;
      }
      if (utils.str2double(prot[1]) < 1.1)
      {
        _status = HTTP_426_UPGRADE_REQUIRED;
        close();
        return;
      }
      while ((line = _client.readStringUntil('\r')) != "")
      {
        _client.read();
        fmt.reset();
        fmt.add(line);
        Serial.println(fmt.format("[HTTP][REQUEST][HEADER] [0]"));
        int idx = utils.indexOf(line, ':');
        if (idx < 0)
        {
          _status = HTTP_406_NOT_ACCEPTABLE;
          close();
          return;
        }
        string k = utils.substring(line, idx),
               v = utils.substring(line, idx + 2, line.length());
        _headers_in.insert(std::pair<string, string>(k, v));
      }
      _client.read();
      if (_method == HTTP_POST)
      {
        int len = utils.str2int(header("Content-Length"));
        if (len > 0)
        {
          string post = "";
          for (int i = 0; i < len; i++)
          {
            post += _client.read();
          }
          post = utils.decodeURL(post);
          if (post.length() < 1 || !readPairs(post, QUERY))
          {
            _status = HTTP_400_BAD_REQUEST;
            close();
            return;
          }
        }
      }
      readPairs(header("Cookie"), COOKIE);
      _sessionId = cookie("sessionId");
      if (_sessionId.length() < 1)
      {
        _sessionId = generateSessionId();
        Cookie sid = Cookie(_sessionId);
        sid.path("/");
        cookie("sessionId", sid);
        _cookies_in.insert(std::pair<string, string>("sessionId", sid.value()));
      }
      _status = HTTP_200_OK;
    }
};

class HTTPServlet
{
  public:
    virtual void service(HTTPConnection& con)
    {
      con.send(HTTP_501_NOT_IMPLEMENTED);
    }
};

class HTTPServer
{
  private:
    WiFiServer _server;
    WiFiClient _current;
    bool _begin;
    long _change;
    int _mode;
    std::map<string, HTTPServlet*> _servlets;
  public:
    void begin()
    {
      if (_begin) return;
      _server.begin();
      _begin = true;
      Serial.println("[HTTP] server started");
    }
    void end()
    {
      if (!_begin) return;
      _begin = false;
      if (_current.connected())
      {
        _current.flush();
        _current.stop();
        _current = WiFiClient();
      }
      _server.stop();
    }
    const bool check(const string& _path)
    {
      if (_servlets.size() < 1) return false;
      std::map<string, HTTPServlet*>::iterator it = _servlets.find(_path);
      return (*it).first == _path;
    }
    bool install(const string& _path, HTTPServlet* _servlet)
    {
      if (check(_path)) return false;
      _servlets.insert(std::pair<string, HTTPServlet*>(_path, _servlet));
      Formatter fmt;
      fmt.add(_path);
      Serial.println(fmt.format("[HTTP] Installed servlet on path '[0]'"));
      return true;
    }
    bool uninstall(const string& _path)
    {
      if (!check(_path)) return false;
      _servlets.erase(_path);
      Formatter fmt;
      fmt.add(_path);
      Serial.println(fmt.format("[HTTP] Uninstalled servlet on path '[0]'"));
      return true;
    }
    void waitFor()
    {
      if (!_begin) return;
      if (_mode == 0)
      {
        _current = _server.available();
        if (!_current) return;
        _mode = 1;
        _change = millis();
      }
      bool _keep = false, _yield = false;
      if (_current.connected())
      {
        switch (_mode)
        {
          case 1:
            if (_current.available())
            {
              _mode = 2;
              _current.setTimeout(HTTP_MAX_SEND_WAIT);
              Formatter fmt;
              HTTPConnection con(_current);
              if (con.status() == 200)
              {
                fmt.reset();
                if (_servlets.size() > 0)
                {
                  fmt.add(con.path());
                  std::map<string, HTTPServlet*>::iterator it = _servlets.find(con.path());
                  if (it != _servlets.end()) {
                    Serial.println(fmt.format("[HTTP] Calling servlet on path '[0]'"));
                    HTTPServlet* _servlet = (*it).second;
                    _servlet->service(con);
                    (*it).second = _servlet;
                    con.close();
                  }
                  con.send(HTTP_404_NOT_FOUND);
                }
              }
            }
            else
            {
              if (millis() - _change <= HTTP_MAX_DATA_WAIT)
                _keep = true;
              _yield = true;
            }
            break;
          case 2:
            if (millis() - _change <= HTTP_MAX_CLOSE_WAIT)
            {
              _keep = true;
              _yield = true;
            }
            break;
        }
      }
      if (!_keep)
      {
        _current = WiFiClient();
        _mode = 0;
      }
      if (_yield) yield();
    }
    HTTPServer(const int& port = 80): _server(port)
    {
      _begin = false;
      _mode = 0;
    }
};
/**************************************************************************************
  Virtuino
 **********/
typedef enum PinType
{
  ANALOG, COMMAND, DIGITAL, ERROR, VIRTUAL
};

typedef enum ErrorCode
{
  PASSWORD_PARAMETER_MUST_BE_ONLY_ONE = 0,
  INCORRECT_PASSWORD                  = 1,
  COMMAND_PARAMETER_MUST_BE_ONLY_ONE  = 2,
  INVALID_COMMAND_REQUEST             = 3,
  UNKNOWN_PIN_TYPE                    = 4,
  INVALID_PIN_ID                      = 5,
  UNKNOWN_PIN_VAL                     = 6,
  UNKNOWN_COMMAND                     = 7
};

typedef enum BoardType
{
  GENERIC_ESP8266         = 0,
  GENERIC_ESP8285         = 1,
  ESPDUINO                = 2,
  ADAFRUIT                = 3,
  ESPESSO_LITE_1_0        = 4,
  ESPESSO_LITE_2_0        = 5,
  PHONEIX_1_0             = 6,
  PHONEIX_2_0             = 7,
  NODEMCU_0_9             = 8,
  NODEMCU_1_0             = 9,
  OLIMEX_MOD_WIFI_ESP8266 = 10,
  SPARK_FUN_THING         = 11,
  SPARK_FUN_THING_DEV     = 12,
  SWEETPEA                = 13,
  WEMOS_D1_R2_MINI        = 14,
  WEMOS_D1                = 15,
  ESPINO                  = 16,
  THAIEASYELECS_ESPINO    = 17,
  WIFINFO                 = 18,
  CORE_DEVELOPMENT_MODULE = 19
};
static const string VIRTUINO_PATH = "/virtuino";
class VirtuinoBoard : public HTTPServlet
{
  private:
    static std::map<int, string> _analog, _digital, _digital_board, _virtual;
    static int _analog_cnt, _digital_cnt, _virtual_cnt;
    static bool _apply;
    bool _begin;
    string _pass;

    void send(const string& _s, HTTPConnection& con)
    {
      Formatter fmt;
      fmt.add(_s);
      con.send(_s);
      Serial.println(fmt.format("[VIRTUINO] Out: [0]"));
    }
    void sendError(const ErrorCode& _code, HTTPConnection& con)
    {
      Formatter fmt;
      fmt.add(_code);
      send(fmt.format("!E00=[0]$"),con);
    }
  public:
    static void reset()
    {
      _analog.clear();
      _digital.clear();
      _digital_board.clear();
      _virtual.clear();
    }
    static void apply(const BoardType& _type)
    {
      if(_apply) return;
      switch (_type)
      {
        case NODEMCU_1_0:
          _analog_cnt = 1;
          _digital_cnt = 10;
          break;
        default:
          _analog_cnt = 0;
          _digital_cnt = 0;
          break;
      }
      _apply = true;
    }
    template<typename T> static void virtualDigitalBoardWrite(const int& _pinId, const T& _t)
    {
      if (!_apply || _pinId < 0 || _pinId > _digital_cnt)
        return;
      std::map<int, string>::iterator it = _digital_board.find(_pinId);
      if (it != _digital_board.end())
        (*it).second = string(_t);
      else
        _digital_board.insert(std::pair<int, string>(_pinId, string(_t)));
    }
    static const string virtualDigitalBoardRead(const int& _pinId)
    {
      if (!_apply) return string();
      std::map<int, string>::iterator it = _digital_board.find(_pinId);
      return (*it).second;
    }
    template<typename T> static void virtualDigitalWrite(const int& _pinId, const T& _t)
    {
      if (!_apply || _pinId < 0 || _pinId > _virtual_cnt)
        return;
      std::map<int, string>::iterator it = _digital.find(_pinId);
      if (it != _digital.end())
        (*it).second = string(_t);
      else
        _digital.insert(std::pair<int, string>(_pinId, string(_t)));
    }
    static const string virtualDigitalRead(const int& _pinId)
    {
      if (!_apply) return string();
      std::map<int, string>::iterator it = _digital.find(_pinId);
      return (*it).second;
    }
    template<typename T> static void virtualAnalogWrite(const int& _pinId, const T& _t)
    {
      if (!_apply || _pinId < 0 || _pinId > _analog_cnt)
        return;
      std::map<int, string>::iterator it = _analog.find(_pinId);
      if (it != _analog.end())
        (*it).second = string(_t);
      else
        _analog.insert(std::pair<int, string>(_pinId, string(_t)));
    }
    static const string virtualAnalogRead(const int& _pinId)
    {
      if (!_apply) return string();
      std::map<int, string>::iterator it = _analog.find(_pinId);
      return (*it).second;
    }
    template<typename T> static void virtualWrite(const int& _pinId, const T& _t)
    {
      if (!_apply || _pinId < 0 || _pinId > _virtual_cnt)
        return;
      std::map<int, string>::iterator it = _virtual.find(_pinId);
      if (it != _virtual.end())
        (*it).second = string(_t);
      else
        _virtual.insert(std::pair<int, string>(_pinId, string(_t)));
    }
    static const string virtualRead(const int& _pinId)
    {
      if (!_apply) return string();
      std::map<int, string>::iterator it = _virtual.find(_pinId);
      return (*it).second;
    }
    static void clear() {
      if (!_apply) return;
      _analog.clear();
      _digital.clear();
      _digital_board.clear();
      _virtual.clear();
      Serial.println("[VIRTUINO] Cleared out");
    }
    void begin(HTTPServer& _server, const string& _pass)
    {
      string out;
      if (!_apply)
        out = "failed: apply board type first!";
      else {
        if (_begin) return;
        if (!_server.install(VIRTUINO_PATH, this)) out = "failed: path already busy!";
        else {
          out = "ok";
          _begin = true;
        }
      }
      Formatter fmt;
      fmt.add(out);
      Serial.println(fmt.format("[VIRTUINO] begin [0]"));
    }
    void stop(HTTPServer& _server)
    {
      string out;
      if (!_apply || !_begin) out = "failed: begin first!";
      else {
        if (!_server.uninstall(VIRTUINO_PATH)) out = "failed: path not installed!";
        else {
          out = "ok";
          _begin = false;
        }
      }
      Formatter fmt;
      fmt.add(out);
      Serial.println(fmt.format("[VIRTUINO] stop [0]"));
    }
    virtual void service(HTTPConnection& con)
    {
      if (!_apply || !_begin) return;
      Formatter fmt;
      KeyPair secret = con.param("secret");
      if (secret.size() > 1)
      {
        sendError(PASSWORD_PARAMETER_MUST_BE_ONLY_ONE, con);
        return;
      }
      if (secret.get(0) != _pass)
      {
        sendError(INCORRECT_PASSWORD, con);
        return;
      }
      KeyPair cmd = con.param("cmd");
      if (cmd.size() > 1)
      {
        sendError(COMMAND_PARAMETER_MUST_BE_ONLY_ONE, con);
        return;
      }
      string cmd_str = cmd.get(0);
      fmt.reset();
      fmt.add(cmd_str);
      Serial.println(fmt.format("[VIRTUINO] In: [0]"));
      if (cmd_str.length() < 1 || cmd_str[0] != '!' || cmd_str[cmd_str.length() - 1] != '$')
      {
        sendError(INVALID_COMMAND_REQUEST, con);
        return;
      }
      string out;
      while (cmd_str[0] == '!' && cmd_str[cmd_str.length() - 1] == '$')
      {
        std::vector<string> _pair = utils.split(utils.substring(cmd_str, 1, utils.indexOf(cmd_str, '$')), '=');
        string  pinType = utils.substring(_pair[0], 0, 1),
                pinIdStr = utils.substring(_pair[0], 1, _pair[0].length()),
                pinValStr = _pair[1];
        if (pinType.length() != 1)
        {
          sendError(UNKNOWN_PIN_TYPE, con);
          return;
        }
        if (pinValStr[0] == '?' && pinValStr.length() != 1)
        {
          sendError(UNKNOWN_PIN_VAL, con);
          return;
        }
        int pinIdNum = utils.str2int(pinIdStr),
            pinValNum = utils.str2int(pinValStr);
        //C00 - Command
        //A00 - Analog Board
        //D00..31 - Digital Virtual
        //O00..<_digital_cnt> - Digital Board
        //Q00..31 - Digital Board
        //V00..31 - Virtual
        //T00..31 - ???
        switch (pinType[0])
        {
          case 'C': //Command
            if (pinValNum == 1) //VersionRequest
            {
              fmt.reset();
              fmt.add(pinIdNum < 10 ? "0" : "");
              fmt.add(pinIdNum);
              fmt.add(VERSION_MAIN);
              fmt.add(VERSION_CODE);
              fmt.add(VERSION_EXTRA);
              out += fmt.format("!C[0][1]=[2]/[3] ([4])$");
              break;
            }
            sendError(UNKNOWN_COMMAND, con);
            break;
          case 'A': //Analog
            if (pinIdNum < 0 || pinIdNum > _analog_cnt)
            {
              sendError(INVALID_PIN_ID, con);
              return;
            }
            if (pinValNum >= 0){
              fmt.reset();
              fmt.add(pinType[0]);
              fmt.add(pinIdNum);
              fmt.add(pinValNum);
              Serial.println(fmt.format("[VIRTUINO] Update pin: [0][1]=[2]"));
              virtualAnalogWrite(pinIdNum, pinValNum);
            }
            fmt.reset();
            fmt.add(pinIdNum < 10 ? "0" : "");
            fmt.add(pinIdNum);
            fmt.add(virtualAnalogRead(pinIdNum));
            out += fmt.format("!A[0][1]=[2]$");
            break;
          case 'D': //Digital Virtual
            if (pinIdNum < 0 || pinIdNum > _virtual_cnt)
            {
              sendError(INVALID_PIN_ID, con);
              return;
            }
            if (pinValNum >= 0){
              fmt.reset();
              fmt.add(pinType[0]);
              fmt.add(pinIdNum);
              fmt.add(pinValNum);
              Serial.println(fmt.format("[VIRTUINO] Update pin: [0][1]=[2]"));
              virtualDigitalWrite(pinIdNum, pinValNum);
            }
            fmt.reset();
            fmt.add(pinIdNum < 10 ? "0" : "");
            fmt.add(pinIdNum);
            fmt.add(virtualDigitalRead(pinIdNum));
            out += fmt.format("!D[0][1]=[2]$");
            break;
          case 'O': //Digital Board
          case 'Q': //Digital Board
            if (pinIdNum < 0 || pinIdNum > _digital_cnt)
            {
              sendError(INVALID_PIN_ID, con);
              return;
            }
            if (pinValNum >= 0){
              fmt.reset();
              fmt.add(pinType[0]);
              fmt.add(pinIdNum);
              fmt.add(pinValNum);
              Serial.println(fmt.format("[VIRTUINO] Update pin: [0][1]=[2]"));
              virtualDigitalBoardWrite(pinIdNum,pinValNum);
            }
            fmt.reset();
            fmt.add(pinType[0]);
            fmt.add(pinIdNum < 10 ? "0" : "");
            fmt.add(pinIdNum);
            fmt.add(virtualDigitalBoardRead(pinIdNum));
            out += fmt.format("![0][1][2]=[3]$");
            break;
          case 'V': //Virtual
              if (pinIdNum < 0 || pinIdNum > _virtual_cnt)
            {
              sendError(INVALID_PIN_ID, con);
                return;
              }
            if (pinValNum >= 0) {
              fmt.reset();
              fmt.add(pinType[0]);
              fmt.add(pinIdNum);
              fmt.add(pinValNum);
              Serial.println(fmt.format("[VIRTUINO] Update pin: [0][1]=[2]"));
              virtualWrite(pinIdNum,pinValNum);
            }
            fmt.reset();
            fmt.add(pinIdNum < 10 ? "0" : "");
            fmt.add(pinIdNum);
            fmt.add(virtualRead(pinIdNum));
              out += fmt.format("!V[0][1]=[2]$");
              break;
          default: //Unknown
            sendError(UNKNOWN_PIN_TYPE, con);
            break;
        }
        int next = utils.indexOf(cmd_str, '$') + 1;
        if (next == cmd_str.length()) break;
        cmd_str = utils.substring(cmd_str, next, cmd_str.length());
      }
      fmt.reset();
      fmt.add(out);
      send(out,con);
    }
    VirtuinoBoard()
    {
      _begin = false;
      _pass = "1234";
    }
};
int VirtuinoBoard::_analog_cnt;
int VirtuinoBoard::_digital_cnt;
int VirtuinoBoard::_virtual_cnt;
std::map<int, string> VirtuinoBoard::_analog;
std::map<int, string> VirtuinoBoard::_digital;
std::map<int, string> VirtuinoBoard::_digital_board;
std::map<int, string> VirtuinoBoard::_virtual;
bool VirtuinoBoard::_apply;
/**************************************************************************************
  Sketch
 ********/
static const string wlan_ap_ssid            = "ESP8266",
                    wlan_ap_pass            = "changeme",
                    wlan_sta_ssid           = "internet",
                    wlan_sta_pass           = "CAG3N3A4",
                    virtuino_pass           = "1234";
static const bool   wlan_ap_secure          = true,
                    wlan_sta_secure         = true,
                    wlan_sta_reconnect      = true;

static const int    wlan_mode               = 0,
                    wlan_sta_reconnect_cnt  = 2;

static const byte   http_favicon_ico[]      = {
  0x89, 0x50, 0x4e, 0x47, 0x0d, 0x0a, 0x1a, 0x0a, 0x00, 0x00, 0x00,
  0x0d, 0x49, 0x48, 0x44, 0x52, 0x00, 0x00, 0x00, 0x10, 0x00, 0x00, 0x00, 0x10, 0x08, 0x06, 0x00,
  0x00, 0x00, 0x1f, 0xf3, 0xff, 0x61, 0x00, 0x00, 0x00, 0x01, 0x73, 0x52, 0x47, 0x42, 0x00, 0xae,
  0xce, 0x1c, /********************************/  0xe9, 0x00, 0x00, 0x00, 0x04, 0x67, 0x41, 0x4d,
  0x41, 0x00, /*########***########***########*/  0x00, 0xb1, 0x8f, 0x0b, 0xfc, 0x61, 0x05, 0x00,
  0x00, 0x00, /*##*********##*********##****##*/  0x09, 0x70, 0x48, 0x59, 0x73, 0x00, 0x00, 0x0e,
  0xc2, 0x00, /*######*****########***########*/  0x00, 0x0e, 0xc2, 0x01, 0x15, 0x28, 0x4a, 0x80,
  0x00, 0x00, /*##***************##***##*******/  0x00, 0x91, 0x49, 0x44, 0x41, 0x54, 0x38, 0x4f,
  0xa5, 0x92, /*########################*******/  0x0b, 0x0e, 0x80, 0x20, 0x0c, 0x43, 0x99, 0xf7,
  0xbf, 0xb3, /********************************/  0xd2, 0x49, 0x97, 0x32, 0x3e, 0xf1, 0xf3, 0x12,
  0xe2, 0xda, /******######********######******/  0x61, 0x11, 0xd0, 0xce, 0x4a, 0xf9, 0x81, 0x07,
  0x98, 0x59, /****##******##****##******##****/  0x93, 0x37, 0xc8, 0x7c, 0xea, 0x1d, 0xad, 0x76,
  0xc1, 0x41, /**##**********##******##****##**/  0x76, 0x1e, 0x89, 0x00, 0xa4, 0xe7, 0x15, 0xe8,
  0xa9, 0x3f, /**##**######**####**######**##**/  0xe8, 0x9a, 0xe6, 0x5b, 0xd0, 0xd4, 0x0c, 0xfb,
  0xb3, 0x79, /**##************##****##****##**/  0xd3, 0x33, 0xd8, 0x31, 0x0d, 0x68, 0xf5, 0x27,
  0xba, 0x00, /****##******##****##******##****/  0xfd, 0x92, 0x61, 0xa5, 0x45, 0x2f, 0x02, 0xf2,
  0xfe, 0x54, /******######********######******/  0xef, 0x7a, 0x7e, 0x0b, 0x79, 0x02, 0x80, 0x86,
  0xcf, 0x1a, /********************************/  0xa8, 0x66, 0x1d, 0xd7, 0x08, 0x60, 0x72, 0x10,
  0xd6, 0xf9, 0x49, 0xba, 0x00, 0x24, 0x73, 0x50, 0x93, 0x99, 0x07, 0x3c, 0x00, 0x66, 0x4e, 0xa6,
  0xd6, 0x1e, 0x5f, 0x86, 0x8e, 0xba, 0x16, 0x11, 0xa9, 0x21, 0x62, 0x3b, 0xab, 0xde, 0xef, 0xff,
  0xa0, 0x3b, 0x83, 0xf7, 0x94, 0x72, 0x01, 0x54, 0x27, 0x92, 0xf1, 0x7a, 0xcd, 0x71, 0x0b, 0x00,
  0x00, 0x00, 0x00, 0x49, 0x45, 0x4e, 0x44, 0xae, 0x42, 0x60, 0x82
};

class FaviconICO: public HTTPServlet
{
  public:
    virtual void service(HTTPConnection& con)
    {
      con.contentType("image/png");
      con.contentLength(sizeof(http_favicon_ico));
      for (int i = 0; i < sizeof(http_favicon_ico); i++)
        con.write(http_favicon_ico[i]);
      con.close();
    }
};

class PageRootIndex: public HTTPServlet
{
  public:
    virtual void service(HTTPConnection& con)
    {
      if (con.path() != "/index.html")
      {
        con.redirect("/index.html");
        return;
      }
      Formatter fmt;
      fmt.add("It's work!");
      con.send("text/html", fmt.format("<html><head><title>[0]</title></head><body><h1>[0]</h1></body></html>"));
    }
};

HTTPServer _server(80);
PageRootIndex _page_root_index;
VirtuinoBoard virtuino;

void setup() {
  delay(1000);
  Serial.begin(115200);
  nanofs.mount();
  Formatter fmt;
  int _cnt = 0, _cnt_reconnect = 0, _mode = wlan_mode;
  bool _echo = false, _wait = true;

  while (WiFi.status() != WL_CONNECTED)
  {
    if (!_echo)
    {
      fmt.reset();
      fmt.add(_mode == 0 ? "STA" : "AP");
      fmt.add(_mode == 0 ? wlan_sta_ssid : wlan_ap_ssid);
      fmt.add(_cnt_reconnect + 1);
      Serial.print(fmt.format("[WLAN] Starting WIFI on '[0]' mode ('[1]') #[2] "));
      WiFi.mode(_mode == 0 ? WIFI_STA : WIFI_AP);
      switch (_mode)
      {
        case 0:
          if (wlan_sta_secure)
            WiFi.begin(wlan_sta_ssid.c_str(), wlan_sta_pass.c_str());
          else
            WiFi.begin(wlan_sta_ssid.c_str());
          break;
        case 1:
          if (wlan_ap_secure)
            WiFi.softAP(wlan_ap_ssid.c_str(), wlan_ap_pass.c_str());
          else
            WiFi.softAP(wlan_ap_ssid.c_str());
          break;
      }
      _echo = true;
    }
    switch (WiFi.status())
    {
      case WL_NO_SSID_AVAIL:
        Serial.println(" failed: no ssid available");
        _wait = false;
        break;
      case WL_CONNECT_FAILED:
        Serial.println(" failed: connection failed");
        _wait = false;
        break;
      case WL_CONNECTION_LOST:
        Serial.println(" failed: connection lost");
        _wait = false;
        break;
      case WL_NO_SHIELD:
        Serial.println(" failed: shield not found");
        _wait = false;
        break;
      case WL_DISCONNECTED:
      case WL_IDLE_STATUS:
        if (_mode == 1 && _cnt == 5) _wait = false;
        break;
    }
    if (!_wait)
    {
      if (_mode == 0)
      {
        if (_cnt_reconnect == wlan_sta_reconnect_cnt)
        {
          if (wlan_sta_reconnect)
          {
            Serial.println("[WLAN] WIFI will be restarted after 5 seconds on AP mode.");
            _mode = 1;
            _cnt = 0;
            _cnt_reconnect = 0;
            _echo = false;
            _wait = true;
            WiFi.disconnect();
            delay(5000);
            continue;
          }
          return;
        }
        Serial.println("[WLAN] Connecting to WIFI will be restarting after 2 seconds.");
        _cnt = 0;
        _echo = false;
        _wait = true;
        _cnt_reconnect++;
        WiFi.disconnect();
        delay(2000);
        continue;
      }
      break;
    }
    _cnt++;
    Serial.print(".");
    delay(1000);
  }
  fmt.reset();
  if (_wait || (!_wait && _mode == 1))
  {
    fmt.add(_mode == 0 ? WiFi.localIP()[0] : WiFi.softAPIP()[0]);
    fmt.add(_mode == 0 ? WiFi.localIP()[1] : WiFi.softAPIP()[1]);
    fmt.add(_mode == 0 ? WiFi.localIP()[2] : WiFi.softAPIP()[2]);
    fmt.add(_mode == 0 ? WiFi.localIP()[3] : WiFi.softAPIP()[3]);
    Serial.println(fmt.format(" ok ([0].[1].[2].[3])"));
    WiFi.printDiag(Serial);
    _server.install("/favicon.ico", new FaviconICO());
    _server.install("/", &_page_root_index);
    _server.install("/index.html", &_page_root_index);
    VirtuinoBoard::apply(NODEMCU_1_0);
    virtuino.begin(_server, virtuino_pass);
    _server.begin();
  }
}

long next_update = millis(), next_pause=5000; //5 sec
void loop() {
  _server.waitFor();
  /**********************************************
    Virtuino Random Example, CMD: !V00=?$
   **********************************************/
  if(millis()>=next_update)
  {
    next_update += next_pause;
    VirtuinoBoard::virtualWrite(0,rand() % 255); //0...255
  }
}
