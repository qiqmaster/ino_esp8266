/**************************************************************************************
  Arduino ESP8266 WWW Main
 **************************************************************************************/
#include <ESP8266WiFi.h>
#include <map>
#include <vector>
#define string String
#include <FS.h>
static const double VERSION_MAIN   = 6.6,
                    VERSION_CODE   = 7.31,
                    VERSION_EXTRA  = 180318;
static const string VERSION_PREFIX = "-perf";
static const string versionString()
{
  string out = "v";
  out += VERSION_MAIN;
  out += "m";
  out += VERSION_CODE;
  out += "c";
  out += VERSION_EXTRA;
  out += "e";
  out += VERSION_PREFIX;
  return out;
}
/**************************************************************************************
  Utils
 *******/
class Utils
{
  private:
    Utils() {}
  public:
    /**********************************************
      String
     **********************************************/
    static const int indexOf(const string& _s, const int& _start, const char& _c)
    {
      for (int i = _start; i < _s.length(); i++)
        if (_s[i] == _c) return i;
      return -1;
    }
    static const int indexOf(const string& _s, const char& _c)
    {
      return indexOf(_s, 0, _c);
    }
    static const string substring(const string& _s, const int& _start, const int& _end)
    {
      string out;
      if (_start >= 0 && _start < _end && _end <= _s.length())
        for (int i = _start; i < _end; i++)
          out += _s[i];
      return out;
    }
    static const string substring(const string& _s, const int& _end)
    {
      return substring(_s, 0, _end);
    }
    static const std::vector<string> split(const string& _s, const char& _c)
    {
      std::vector<string> out;
      string cur;
      for (int i = 0; i < _s.length() + 1; i++)
      {
        if (_s[i] == _c || i == _s.length())
        {
          out.push_back(cur);
          cur = "";
          continue;
        }
        cur += _s[i];
      }
      return out;
    }
    static const string trim(const string& _s)
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
    static const int pow(const int& _in, const int& _pow)
    {
      if (_pow < 1) return 1;
      if (_pow == 1) return _in;
      int out = _in;
      for (int i = 1; i < _pow; i++) out *= _in;
      return out;
    }
    static const bool isNum(const string& _s)
    {
      bool out = true;
      for (int i = 0; i < _s.length(); i++)
      {
        if (i == 0 && i + 1 < _s.length() && _s[i] == '-')
          continue;
        if (_s[i] < '0' || _s[i] > '9')
          return false;
      }
      return out;
    }
    static const int str2int(const string& _s)
    {
      int out = 0, _pow = 0;
      if (!isNum(_s)) return -1;
      for (int i = _s.length() - 1; i >= (_s[0] == '-' ? 1 : 0); i--)
      {
        out += pow(10, _pow) * (_s[i] - '0');
        _pow++;
      }
      if (_s[0] == '-') out *= -1;
      return out;
    }
    static const double str2double(const String& _s)
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
    static const string int2str(const int& _i)
    {
      return string(_i, DEC);
    }
    static const string double2str(const double& _d)
    {
      return string(_d, DEC);
    }
    /**********************************************
      URL
     **********************************************/
    static const string encodeURL(const string& _s)
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
    static const string decodeURL(const string& _s)
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
};
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
      _values.push_back(string(_t));
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
              int index = Utils::str2int(num);
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
enum HTTPStatus
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
      if (_value.length() < 1) return false;
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
enum PairType
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
      std::vector<string> _pair = Utils::split(_pairs, _type == QUERY ? '&' : ';');
      for (int i = 0; i < _pair.size(); i++)
      {
        int idx = Utils::indexOf(_pair[i], '=');
        if (idx < 0)
          return false;
        string k = Utils::trim(Utils::substring(_pair[i], idx)),
               v = Utils::trim(Utils::substring(_pair[i], idx + 1, _pair[i].length()));
        if (k.length() > 0 && v.length() > 0)
        {
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
        fmt.add(_client.remoteIP()[0]);
        fmt.add(_client.remoteIP()[1]);
        fmt.add(_client.remoteIP()[2]);
        fmt.add(_client.remoteIP()[3]);
        _client.print(fmt.format("HTTP/1.1 [1]\r\n"));
        Serial.println(fmt.format("[HTTP][[3].[4].[5].[6]][[0]][[1]] [2]"));
        header("Content-Length", Utils::int2str(_clen));
        header("Content-Type", _ctype);
        if (_status != HTTP_444_CLOSED_WITHOUT_HEADERS)
        {
          for (std::map<string, string>::iterator cur = _headers_out.begin(); cur != _headers_out.end(); cur++)
          {
            fmt.reset();
            fmt.add((*cur).first);
            fmt.add((*cur).second);
            _client.print(fmt.format("[0]: [1]\r\n"));
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
              fmt.add((c.expire() > 1000 ? string(c.expire()) : ""));       //[8]
              fmt.add(c.secure() ? "; secure" : "");                        //[9]
              _client.print(fmt.format("[0]: [1]=[2][3][4][5][6][7][8][9]\r\n"));
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
      if (_ctype.length() > 0 && Utils::indexOf(_ctype, '/') > 0)
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
          fmt.add(versionString());
          send(HTTP_200_OK, "text/html", fmt.format("<html><head><title>Error [0]</title></head><body><h1>Error [0]</h1><hr><i>Arduino HTTPServer [1]</i></body></html>"));
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
      _status = HTTP_200_OK;
      _clen = 0;
      _ctype = "text/plain";
      _sendHeaders = false;
      header("Connection", "close");
      fmt.reset();
      string line = _client.readStringUntil('\r');
      _client.read();
      std::vector<string> hdr = Utils::split(line, ' ');
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
      _uri = Utils::decodeURL(hdr[1]);
      if (_uri.length() < 1)
      {
        _status = HTTP_400_BAD_REQUEST;
        close();
        return;
      }
      int idx = Utils::indexOf(_uri, '?');
      if (idx > 0)
      {
        if (!readPairs(Utils::substring(_uri, idx + 1, _uri.length()), QUERY))
        {
          _status = HTTP_400_BAD_REQUEST;
          close();
          return;
        }
        _path = Utils::substring(_uri, 0, idx);
      } else _path = _uri;
      std::vector<string> prot = Utils::split(hdr[2], '/');
      if (prot.size() != 2 || prot[0] != "HTTP")
      {
        _status = HTTP_444_CLOSED_WITHOUT_HEADERS;
        close();
        return;
      }
      if (Utils::str2double(prot[1]) < 1.1)
      {
        _status = HTTP_426_UPGRADE_REQUIRED;
        close();
        return;
      }
      while ((line = _client.readStringUntil('\r')) != "")
      {
        _client.read();
        int idx = Utils::indexOf(line, ':');
        if (idx < 0)
        {
          _status = HTTP_406_NOT_ACCEPTABLE;
          close();
          return;
        }
        string k = Utils::substring(line, idx),
               v = Utils::substring(line, idx + 2, line.length());
        _headers_in.insert(std::pair<string, string>(k, v));
      }
      _client.read();
      if (_method == HTTP_POST)
      {
        int len = Utils::str2int(header("Content-Length"));
        if (len > 0)
        {
          string post = "";
          for (int i = 0; i < len; i++)
          {
            post += _client.read();
          }
          post = Utils::decodeURL(post);
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
  private:
    virtual void service(HTTPConnection& con)
    {
      con.send(HTTP_501_NOT_IMPLEMENTED);
    }
  public:
    static void service(HTTPServlet& _servlet, HTTPConnection& _con)
    {
      _servlet.service(_con);
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

              HTTPConnection con(_current);
              if (con.status() == 200)
              {
                if (_servlets.size() > 0)
                {
                  std::map<string, HTTPServlet*>::iterator it = _servlets.find(con.path());
                  if (it != _servlets.end()) {
                    HTTPServlet* _servlet = (*it).second;
                    HTTPServlet::service(*_servlet, con);
                    (*it).second = _servlet;
                    con.close();
                    break;
                  }
                }
                con.send(HTTP_404_NOT_FOUND);
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
enum PinMode
{
  MODE_INPUT, MODE_OUTPUT, MODE_ANY
};
enum PinType
{
  ANALOG, COMMAND, DIGITAL, DIGITAL_VIRTUAL, VIRTUAL, UNKNOWN
};
enum BoardType
{
  NODEMCU_ESP_12E
};
class Pin
{
  private:
    PinMode _mode;
    PinType _type;
    int _value;
    bool _pwm;
  public:
    const PinMode mode()
    {
      return _mode;
    }
    void mode(const PinMode& _mode)
    {
      this->_mode = _mode;
    }
    const int value()
    {
      return _value;
    }
    void value(const int& _value)
    {
      this->_value = _value;
    }
    const bool pwm()
    {
      return _pwm;
    }
    void pwm(const bool& _pwm)
    {
      this->_pwm = _pwm;
    }
    const PinType type()
    {
      return _type;
    }
    Pin(const PinType& _type, const bool& _pwm, const PinMode& _mode)
    {
      this->_type = _type;
      this->_pwm = _pwm;
      this->_mode = _mode;
      _value = 0;
    }
};

enum ErrorId
{
  ERROR_OK,
  ERROR_INVALID_REQUEST,
  ERROR_DUPLICATED,
  ERROR_INCORRECT_PASSWORD,
  ERROR_UNKNOWN_PIN_TYPE,
  ERROR_INVALID_PIN_ID,
  ERROR_UNKNOWN_CMD
};

static const string VIRTUINO_PATH = "/virtuino";

class VirtuinoBoard: public HTTPServlet
{
  private:
    static const int COMMAND_PIN_CNT = 1,
                     VIRTUAL_PIN_CNT = 32;
    static const char CMD_START_CHAR = '!',
                      CMD_END_CHAR = '$',
                      CMD_REQ_CHAR = '?',
                      PIN_COMMAND_CHAR = 'C',
                      PIN_ANALOG_CHAR = 'A',
                      PIN_DIGITAL_CHAR = 'O',
                      PIN_DIGITAL_VIRTUAL_CHAR = 'D',
                      PIN_VIRTUAL_CHAR = 'V',
                      PIN_PWM_CHAR = 'Q';
    static std::vector<Pin> _pins;
    static int _cmd_idx, _a_idx, _d_idx, _dv_idx, _v_idx;
    static string _pass;
    static bool _begin;
    void send(const string& _s, HTTPConnection& con)
    {
      con.send(_s);
    }
    void sendError(const ErrorId& _id, HTTPConnection& con)
    {
      Formatter fmt;
      fmt.add(CMD_START_CHAR);
      fmt.add(_id);
      fmt.add(CMD_END_CHAR);
      send(fmt.format("[0]00=[1][2]"), con);
    }
    virtual void service(HTTPConnection& con)
    {
      KeyPair _secret = con.param("secret"),
              _cmd = con.param("cmd");
      if (_secret.size() < 1 || _cmd.size() < 1)
      {
        sendError(ERROR_INVALID_REQUEST, con);
        return;
      }
      if (_secret.size() > 1 || _cmd.size() > 1)
      {
        sendError(ERROR_DUPLICATED, con);
        return;
      }
      if (_secret.get(0) != _pass)
      {
        sendError(ERROR_INCORRECT_PASSWORD, con);
        return;
      }
      string req = _cmd.get(0);
      std::map<int, string> _tmp;
      std::map<int, char> _in, _out;
      ErrorId errId = ERROR_OK;
      while (true)
      {
        if (req[0] != CMD_START_CHAR || req[req.length() - 1] != CMD_END_CHAR)
        {
          errId = ERROR_INVALID_REQUEST;
          break;
        }
        int idx = Utils::indexOf(req, CMD_END_CHAR);
        std::vector<string> _pair = Utils::split(Utils::substring(req, 1, idx), '=');
        if (_pair.size() != 2 || _pair[0].length() < 3 || _pair[1].length() < 1)
        {
          errId = ERROR_INVALID_REQUEST;
          break;
        }
        string _pinStr = _pair[0],
               _valStr = _pair[1],
               _idStr = Utils::substring(_pinStr, 1, _pinStr.length());
        char _type = _pinStr[0];
        if (!Utils::isNum(_idStr) || (!Utils::isNum(_valStr) && _valStr.length() == 1 && _valStr[0] != CMD_REQ_CHAR))
        {
          errId = ERROR_INVALID_REQUEST;
          break;
        }
        int _id = Utils::str2int(_idStr);

        if (_type == PIN_COMMAND_CHAR)
        {
          int _pinId = pinId(COMMAND, _id);
          if (_pinId < 0)
          {
            errId = ERROR_INVALID_PIN_ID;
            break;
          }
          std::map<int, string>::iterator it = _tmp.find(_pinId);
          if (it != _tmp.end())
          {
            errId = ERROR_DUPLICATED;
            break;
          }
          int _cmd_id = Utils::str2int(_valStr);
          if (_cmd_id == 1) //CheckVersion
          {
            _tmp.insert(std::pair<int, string>(_pinId, versionString()));
            _out.insert(std::pair<int, char>(_pinId, _type));
          }
          else errId = ERROR_UNKNOWN_CMD;
          break;
        }
        if (_type == PIN_ANALOG_CHAR)
        {
          int _pinId = pinId(ANALOG, _id);
          if (_pinId < 0)
          {
            errId = ERROR_INVALID_PIN_ID;
            break;
          }
          std::map<int, string>::iterator it = _tmp.find(_pinId);
          if (it != _tmp.end())
          {
            errId = ERROR_DUPLICATED;
            break;
          }
          if (Utils::isNum(_valStr) && pinMode(_pinId) != MODE_OUTPUT) //! MODE_INPUT or MODE_ANY
          {
            _tmp.insert(std::pair<int, string>(_pinId, _valStr));
            _in.insert(std::pair<int, char>(_pinId, _type));
          }
          else
          {
            _tmp.insert(std::pair<int, string>(_pinId, Utils::int2str(_pins[_pinId].value())));
            _out.insert(std::pair<int, char>(_pinId, _type));
          }
        }
        else if (_type == PIN_DIGITAL_CHAR || _type == PIN_PWM_CHAR)
        {
          int _pinId = pinId(DIGITAL, _id);
          if (_pinId < 0)
          {
            errId = ERROR_INVALID_PIN_ID;
            break;
          }
          std::map<int, string>::iterator it = _tmp.find(_pinId);
          if (it != _tmp.end())
          {
            errId = ERROR_DUPLICATED;
            break;
          }
          if (Utils::isNum(_valStr) && pinMode(_pinId) != MODE_OUTPUT && (_type == PIN_PWM_CHAR ? isPWM(_pinId) : true)) //! MODE_INPUT or MODE_ANY
          {
            _tmp.insert(std::pair<int, string>(_pinId, _valStr));
            _in.insert(std::pair<int, char>(_pinId, _type));
          }
          else
          {
            _tmp.insert(std::pair<int, string>(_pinId, Utils::int2str(_pins[_pinId].value())));
            _out.insert(std::pair<int, char>(_pinId, _type));
          }
        }
        else if (_type == PIN_DIGITAL_VIRTUAL_CHAR)
        {
          int _pinId = pinId(DIGITAL_VIRTUAL, _id);
          if (_pinId < 0)
          {
            errId = ERROR_INVALID_PIN_ID;
            break;
          }
          std::map<int, string>::iterator it = _tmp.find(_pinId);
          if (it != _tmp.end())
          {
            errId = ERROR_DUPLICATED;
            break;
          }
          if (Utils::isNum(_valStr) && pinMode(_pinId) != MODE_OUTPUT) //! MODE_INPUT or MODE_ANY
          {
            _tmp.insert(std::pair<int, string>(_pinId, _valStr));
            _in.insert(std::pair<int, char>(_pinId, _type));
          }
          else
          {
            _tmp.insert(std::pair<int, string>(_pinId, Utils::int2str(_pins[_pinId].value())));
            _out.insert(std::pair<int, char>(_pinId, _type));
          }
        }
        else if (_type == PIN_VIRTUAL_CHAR)
        {
          int _pinId = pinId(VIRTUAL, _id);
          if (_pinId < 0)
          {
            errId = ERROR_INVALID_PIN_ID;
            break;
          }
          std::map<int, string>::iterator it = _tmp.find(_pinId);
          if (it != _tmp.end())
          {
            errId = ERROR_DUPLICATED;
            break;
          }
          if (Utils::isNum(_valStr) && pinMode(_pinId) != MODE_OUTPUT) //! MODE_INPUT or MODE_ANY
          {
            _tmp.insert(std::pair<int, string>(_pinId, _valStr));
            _in.insert(std::pair<int, char>(_pinId, _type));
          }
          else
          {
            _tmp.insert(std::pair<int, string>(_pinId, Utils::int2str(_pins[_pinId].value())));
            _out.insert(std::pair<int, char>(_pinId, _type));
          }
        }
        else
        {
          errId = ERROR_UNKNOWN_PIN_TYPE;
          break;
        }
        if (idx + 1 < req.length()) req = Utils::substring(req, idx + 1, req.length());
        else break;
      }
      if (errId != ERROR_OK)
      {
        _tmp.clear();
        _in.clear();
        _out.clear();
        sendError(errId, con);
        return;
      }
      for (std::map<int, char>::iterator cur = _in.begin(); cur != _in.end(); cur++)
      {
        writePin((*cur).first, Utils::str2int(_tmp[(*cur).first]));
        _out.insert(std::pair<int, char>((*cur).first, (*cur).second));
      }
      _in.clear();
      Formatter fmt;
      string out;
      for (std::map<int, char>::iterator cur = _out.begin(); cur != _out.end(); cur++)
      {
        fmt.reset();
        fmt.add(CMD_START_CHAR);
        fmt.add((*cur).second);
        int id = pinTypeId((*cur).first);
        fmt.add(id < 10 ? "0" : "");
        fmt.add(id);
        fmt.add(_tmp[(*cur).first]);
        fmt.add(CMD_END_CHAR);
        out += fmt.format("[0][1][2][3]=[4][5]");
      }
      fmt.reset();
      send(out, con);
    }
  public:
    static const int pinCound()
    {
      return _pins.size();
    }
    static const PinMode pinMode(const int& _pinId)
    {
      return _begin && _pinId >= 0 && _pinId < _pins.size() ? _pins[_pinId].mode() : MODE_ANY;
    }
    static void pinMode(const int& _pinId, const PinMode& _mode)
    {
      if (_begin && _pinId >= 0 && _pinId < _pins.size())
        _pins[_pinId].mode(_mode);
    }
    static const PinType pinType(const int& _pinId)
    {
      return _begin && _pinId >= 0 && _pinId < _pins.size() ? _pins[_pinId].type() : UNKNOWN;
    }
    static const bool isCommand(const int& _pinId)
    {
      return pinType(_pinId) == COMMAND;
    }
    static const bool isAnalog(const int& _pinId)
    {
      return pinType(_pinId) == ANALOG;
    }
    static const bool isDigital(const int& _pinId)
    {
      return pinType(_pinId) == DIGITAL;
    }
    static const bool isVirtualDigital(const int& _pinId)
    {
      return pinType(_pinId) == DIGITAL_VIRTUAL;
    }
    static const bool isVirtual(const int& _pinId)
    {
      return pinType(_pinId) == VIRTUAL;
    }
    static const bool isPWM(const int& _pinId)
    {
      return isDigital(_pinId) && _pins[_pinId].pwm();
    }
    static const int pinId(const PinType& _type, const int& _id)
    {
      switch (_type)
      {
        case COMMAND:
          if (isCommand(_cmd_idx + _id))
            return _cmd_idx + _id;
          break;
        case ANALOG:
          if (isAnalog(_a_idx + _id))
            return _a_idx + _id;
          break;
        case DIGITAL:
          if (isDigital(_d_idx + _id))
            return _d_idx + _id;
          break;
        case DIGITAL_VIRTUAL:
          if (isVirtualDigital(_dv_idx + _id))
            return _dv_idx + _id;
          break;
        case VIRTUAL:
          if (isVirtual(_v_idx + _id))
            return _v_idx + _id;
          break;
      }
      return -1;
    }
    static const int pinTypeId(const int& _pinId)
    {
      switch (pinType(_pinId))
      {
        case COMMAND:
          return _pinId - _cmd_idx;
        case ANALOG:
          return _pinId - _a_idx;
        case DIGITAL:
          return _pinId - _d_idx;
        case DIGITAL_VIRTUAL:
          return _pinId - _dv_idx;
        case VIRTUAL:
          return _pinId - _v_idx;
      }
      return -1;
    }
    static const int readPin(const int& _pinId)
    {
      return _begin && _pinId >= 0 && _pinId < _pins.size() && _pins[_pinId].mode() != MODE_OUTPUT ? _pins[_pinId].value() : -1;
    }
    static const void writePin(const int& _pinId, const int& _value)
    {
      if (_begin && _pinId >= 0 && _pinId < _pins.size() && _pins[_pinId].mode() != MODE_INPUT)
        _pins[_pinId].value(_value);
    }
    static const int readAnalog(const int& _id)
    {
      return isAnalog(_a_idx + _id) ? readPin(_a_idx + _id) : -1;
    }
    static void writeAnalog(const int& _id, const int& _value)
    {
      if (isAnalog(_a_idx + _id))
        writePin(_a_idx + _id, _value);
    }
    const int readDigital(const int& _id)
    {
      return isDigital(_d_idx + _id) ? readPin(_d_idx + _id) : -1;
    }
    static void writeDigital(const int& _id, const int& _value)
    {
      if (isDigital(_d_idx + _id))
        writePin(_d_idx + _id, _value);
    }
    static const int readVirtualDigital(const int& _id)
    {
      return isVirtualDigital(_dv_idx + _id) ? readPin(_dv_idx + _id) : -1;
    }
    static void writeVirtualDigital(const int& _id, const int& _value)
    {
      if (isVirtualDigital(_dv_idx + _id))
        writePin(_dv_idx + _id, _value);
    }
    static const int readVirtual(const int& _id)
    {
      return isVirtual(_v_idx + _id) ? readPin(_v_idx + _id) : -1;
    }
    static void writeVirtual(const int& _id, const int& _value)
    {
      if (isVirtual(_v_idx + _id))
        writePin(_v_idx + _id, _value);
    }
    static void begin(const BoardType& _board, const string& _pass)
    {
      if (!_begin && _pass.length() > 0)
      {
        VirtuinoBoard::_pass = _pass;
        int _a_cnt, _d_cnt;
        std::vector<int> _pwm_idx;
        switch (_board)
        {
          case NODEMCU_ESP_12E:
            _a_cnt = 1;
            _d_cnt = 11;
            for (int i = 0; i < _d_cnt; i++) //ENABLE PWM ON ALL DIGITAL PINS
              _pwm_idx.push_back(i);
            break;
        }
        _cmd_idx = 0;
        _a_idx = _cmd_idx + COMMAND_PIN_CNT;
        _d_idx = _a_idx + _a_cnt;
        _dv_idx = _d_idx + _d_cnt;
        _v_idx = _dv_idx + VIRTUAL_PIN_CNT;
        for (int i = 0; i < COMMAND_PIN_CNT; i++)
          _pins.push_back(Pin(COMMAND, false, MODE_INPUT));
        for (int i = 0; i < _a_cnt; i++)
          _pins.push_back(Pin(ANALOG, false, MODE_ANY));
        for (int i = 0; i < _d_cnt; i++)
          _pins.push_back(Pin(DIGITAL, false, MODE_ANY));
        for (int i = 0; i < VIRTUAL_PIN_CNT; i++)
          _pins.push_back(Pin(DIGITAL_VIRTUAL, false, MODE_ANY));
        for (int i = 0; i < VIRTUAL_PIN_CNT; i++)
          _pins.push_back(Pin(VIRTUAL, false, MODE_ANY));
        for (int i = 0; i < _pwm_idx.size(); i++)
          _pins[_d_idx + i].pwm(true);
        _begin = true;
        Serial.println("[VIRTUINO] begin successfully");
      }
    }
    static void end()
    {
      if (_begin)
      {
        _pins.clear();
        _begin = false;
      }
    }
    VirtuinoBoard()
    {
    }
};
std::vector<Pin> VirtuinoBoard::_pins;
bool VirtuinoBoard::_begin;
int VirtuinoBoard::_cmd_idx, VirtuinoBoard::_a_idx, VirtuinoBoard::_d_idx, VirtuinoBoard::_dv_idx, VirtuinoBoard::_v_idx;
string VirtuinoBoard::_pass;
/**************************************************************************************
  Sketch
 ********/
static const string wlan_ap_ssid            = "ESP8266",
                    wlan_ap_pass            = "changeme",
                    wlan_sta_ssid           = "<sta_ssid>",
                    wlan_sta_pass           = "<sta_pass>",
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
  private:
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
  private:
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

HTTPServer server(80);
PageRootIndex page_root_index;
FaviconICO favicon_ico;
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

    server.install("/favicon.ico", &favicon_ico);
    server.install("/", &page_root_index);
    server.install("/index.html", &page_root_index);
    /**********************************************
      Virtuino
     **********************************************/
    //1. Begin use board
    VirtuinoBoard::begin(NODEMCU_ESP_12E, virtuino_pass);
    //2.1 Set V00 mode as OUTPUT
    VirtuinoBoard::pinMode(VirtuinoBoard::pinId(VIRTUAL, 0), MODE_OUTPUT);
    //2.2 Set V01 mode as OUTPUT
    VirtuinoBoard::pinMode(VirtuinoBoard::pinId(VIRTUAL, 1), MODE_OUTPUT);
    //3. Install virtuino class as servlet
    server.install(VIRTUINO_PATH, &virtuino);

    server.begin();
  }
}

long rnd_next_update = millis(), rnd_delay = 5000,  //5 sec
     led_next_update = millis(), led_delay = 10000; //10 sec
bool blink = false;
void loop() {
  server.waitFor();
  /**********************************************
    Virtuino Random Example, CMD: !V00=?$
   **********************************************/
  if (millis() >= rnd_next_update)
  {
    rnd_next_update += rnd_delay;
    VirtuinoBoard::writePin(virtuino.pinId(VIRTUAL, 0), rand() % 255); //0...255
  }
  /************************************************
    Virtuino Board LED Blink Example, CMD: !V01=?$
   ************************************************/
  if (millis() >= led_next_update)
  {
    led_next_update += rnd_delay;
    VirtuinoBoard::writePin(virtuino.pinId(VIRTUAL, 1), !blink); //0...255
    blink != blink;
  }
}
