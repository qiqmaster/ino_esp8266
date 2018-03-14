/**************************************************************************************
  Arduino ESP8266 WWW Main
 **************************************************************************************/
#include <ESP8266WiFi.h>
#include <map>
#include <vector>
#include <functional>
#define string String
#include <FS.h>
const int VERSION_MAJOR = 6,
          VERSION_MINOR = 3,
          VERSION_EXTRA = 0;
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
const string HTTP_GET                                 = "GET",
             HTTP_POST                                = "POST";

const int    HTTP_MAX_DATA_WAIT                       = 5000,
             HTTP_MAX_POST_WAIT                       = 5000,
             HTTP_MAX_SEND_WAIT                       = 5000,
             HTTP_MAX_CLOSE_WAIT                      = 2000,
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
             HTTP_526_INVALID_SSL_CERTIFICATE         = 526;

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

typedef enum CheckType
{
  HEADERS_IN, HEADERS_OUT, PARAMETERS, COOKIES_IN, COOKIES_OUT
};

typedef enum PairType
{
  QUERY, COOKIE
};

class HTTPConnection
{
  private:
    WiFiClient _client;
    int _status;
    string _method, _uri, _path, _sessionId;
    std::map<string, string> _headers_in, _headers_out, _cookies_in;
    std::map<string, KeyPair> _params;
    std::map<string, Cookie> _cookies_out;
    bool _sendHeaders;

    const bool check(const string& _name, const CheckType& _type)
    {
      switch (_type)
      {
        case COOKIES_OUT:
          for (std::map<string, Cookie>::iterator cur = _cookies_out.begin(); cur != _cookies_out.end(); cur++)
            if ((*cur).first == _name)
              return true;
          break;
        case PARAMETERS:
          for (std::map<string, KeyPair>::iterator cur = _params.begin(); cur != _params.end(); cur++)
            if ((*cur).first == _name)
              return true;
          break;
        default:
          for (std::map<string, string>::iterator cur = (
                 _type == HEADERS_IN ? _headers_in.begin() :
                 _type == HEADERS_OUT ? _headers_out.begin() :
                 _cookies_in.begin()); cur != (
                 _type == HEADERS_IN ? _headers_in.end() :
                 _type == HEADERS_OUT ? _headers_out.end() :
                 _cookies_in.end()); cur++)
            if ((*cur).first == _name)
              return true;
          break;
      }
      return false;
    }
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
          fmt.add(_type == QUERY ? "QUERY" : "COOKIE");
          fmt.add(k);
          fmt.add(v);
          Serial.println(fmt.format("[HTTP][REQUEST][[0]] [1]=[2]"));
          bool detect = check(k, _type == QUERY ? PARAMETERS : COOKIES_IN);
          if (_type == QUERY)
          {
            KeyPair kp = detect ? _params[k] : KeyPair();
            kp.add(v);
            if (detect)
              _params[k] = kp;
            else
              _params.insert(std::pair<string, KeyPair>(k, kp));
          }
          else
          {
            if (detect)
              _cookies_in[k] = v;
            else
              _cookies_in.insert(std::pair<string, string>(k, v));
          }
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
    void close()
    {
      sendHeaders();
      _client.flush();
      _client.stop();
    }
  public:
    const int status()
    {
      return _status;
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
      return check(_name, PARAMETERS) ? _params[_name] : KeyPair();
    }
    const string header(const string& _name)
    {
      return check(_name, HEADERS_IN) ? _headers_in[_name] : "";
    }
    void header(const string& _name, const string& _value)
    {
      _headers_out.insert(std::pair<string, string>(_name, _value));
    }
    const string cookie(const string& _name)
    {
      return check(_name, COOKIES_IN) ? _cookies_in[_name] : string();
    }
    void cookie(const string& _name, const Cookie& _cookie)
    {
      if (!check(_name, COOKIES_OUT))
      {
        _cookies_out.insert(std::pair<string, Cookie>(_name, _cookie));
      }
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
    void send(const int& _status, const string& _ctype, const string& _content)
    {
      if (!_sendHeaders)
      {
        this->_status = _status;
        if (_status >= HTTP_400_BAD_REQUEST)
        {
          Formatter fmt;
          fmt.add(_status);
          fmt.add(VERSION_MAJOR);
          fmt.add(VERSION_MINOR);
          fmt.add(VERSION_EXTRA);
          send(HTTP_200_OK, "text/html", fmt.format("<html><head><title>Error [0]</title></head><body><h1>Error [0]</h1><hr>Arduino HTTPServer v[1].[2].[3]</body></html>"));
          return;
        }
        header("Content-Type", _ctype);
        header("Content-Length", utils.int2str(_content.length()));
        sendHeaders();
        _client.print(_content);
        close();
      }
    }
    void send(const int& _status, const string& _content)
    {
      send(_status, "text/plain", _content);
    }
    void send(const string& _ctype, const string& _content)
    {
      send(_status, _ctype, _content);
    }
    void send(const string& _content)
    {
      send(_status, _content);
    }
    void send(const int& _status)
    {
      send(_status, string());
    }
    HTTPConnection(WiFiClient& _client)
    {
      this->_client = _client;
      _sendHeaders = false;
      header("Connection", "close");
      Formatter fmt;
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

class HTTPServer
{
  private:
    WiFiServer _server;
    WiFiClient _current;
    bool _begin;
    long _change;
    int _mode;
    std::map<string, std::function<void(HTTPConnection&)>> _handles;
    const bool check(const string& _path)
    {
      if (_handles.size() < 1) return false;
      for (std::map<string, std::function<void(HTTPConnection&)>>::iterator cur = _handles.begin(); cur != _handles.end(); cur++)
        if ((*cur).first == _path) return true;
      return false;
    }
  public:
    void begin()
    {
      if (_begin) return;
      _server.begin();
      _begin = true;
      Serial.println("[HTTP] server started");
    }
    void on(const string& _path, const std::function<void(HTTPConnection&)>& _func)
    {
      if (check(_path)) return;
      _handles.insert(std::pair<string, std::function<void(HTTPConnection&)>>(_path, _func));
      Formatter fmt;
      fmt.add(_path);
      Serial.println(fmt.format("[HTTP] Registered handle on path '[0]'"));
    }
    void waitFor()
    {
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
                Formatter fmt;
                if (_handles.size() > 0)
                {
                  fmt.add(con.path());
                  if (check(con.path()))
                  {
                    Serial.println(fmt.format("[HTTP] Calling handle on path '[0]'"));
                    _handles[con.path()](con);
                    break;
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
  Sketch
 ********/
static const string wlan_ap_ssid = "ESP8266",
                    wlan_ap_pass = "changeme",
                    wlan_sta_ssid = "<sta_ssid>",
                    wlan_sta_pass = "<sta_password>";
static const bool   wlan_ap_secure = true,
                    wlan_sta_secure = true,
                    wlan_sta_reconnect = true;

static const int    wlan_mode = 0,
                    wlan_sta_reconnect_cnt = 2;


HTTPServer _server(80);

void page_root_index(HTTPConnection& con)
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
    _server.on("/", page_root_index);
    _server.on("/index.html", page_root_index);
    _server.begin();
  }
}

void loop() {
  _server.waitFor();
}
