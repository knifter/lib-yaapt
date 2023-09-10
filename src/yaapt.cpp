
#include "yaapt.h"
#include "yaapt_config.h"

#include <vector>

// using namespace std;

enum FieldType_t : uint8_t 
{
    FIELD_VERSION =         'v',
    FIELD_CHANNEL_ID =      'C', // 0x01,
    FIELD_GROUP_START =     '{', // 0x02, // <STX>
    FIELD_GROUP_END =       '}', // 0x03, // <ETX>
    // FIELD_INIT_START =      '(',
    // FIELD_INIT_END =        ')',

    // Lists:
    FIELD_CHANNEL_DESC = 0x10,
         FIELD_CHANNEL_TYPE = 0x11,
         FIELD_CHANNEL_NAME = 0x12,
         FIELD_CHANNEL_CONTENTS = 0x13,
    // FIELD_CHANNEL_DESC_END = 0x1F,


    FIELD_LOG_LEVEL =       0x20,
    FIELD_LOG_LINENR =      0x21,
    FIELD_LOG_FILENAME =    0x22,
    FIELD_LOG_FUNCTION =    0x23,
    FIELD_LOG_MESSAGE =     0x24,
    // '0' = 0x30
    // 'A' = 0x41
    // 'a' = 0x61
    FIELD_STRING =          'S',
    FIELD_FLOAT =           'F',
    FIELD_DOUBLE =          'D',
    FIELD_UINT8 =           '8',
    FIELD_INT8 =            '7',
    FIELD_UINT32 =          'U',
    FIELD_INT32 =           'S',
    FIELD_UINT64 =          'V',
    FIELD_INT64 =           'L',
};

enum ChannelType_t : uint8_t 
{
    CHANNEL_TYPE_NONE = 0x00,
    CHANNEL_TYPE_LOG = 'L',
    CHANNEL_TYPE_PLOT = 'P',
    // CHANNEL_TYPE_COMMAND = 'C',
    CHANNEL_TYPE_DATA = 'D',
    CHANNEL_TYPE_TEXT = 'T'
};

/* 
* Print
*   Stream
*       HardwareSerial
*       SoftwareSerial
*/

Yaapt::Yaapt(Stream& s) : _stream(s)
{
};

bool Yaapt::begin()
{
    // _serial.begin(YAAPT_BAUDRATE, SERIAL_8N1);

    _stream.printf("Yaapt#%c%d", FIELD_VERSION, YAAPT_VERSION);

    // _stream.write(FIELD_CHANNEL_DESC);
    
    for(YaaptChannel* c : _channels)
    {
        _stream.write(FIELD_CHANNEL_DESC);
        _stream.write(FIELD_GROUP_START);
        _stream.write(FIELD_CHANNEL_TYPE);
        _stream.write(c->channel_type());
        _stream.write(FIELD_CHANNEL_NAME);
        _stream.write(c->name());
        // _port.write(FIELD_CHANNEL_CONTENTS);
        // _port.write(strlen(c->channel_content_descriptor));       // Max 255
        // _port.write(c->channel_content_descriptor, strlen(c->channel_content_descriptor));        
        _stream.write(FIELD_GROUP_END);
    };

    return true;
};

uint8_t Yaapt::registerChannel(YaaptChannel* c)
{
    // Check if channel already registered
    if(c->channel_id() > 0)
        return c->channel_id();
    
    // Add this channel to our vector
    _channels.push_back(c);
    return _channels.size() + '0';
};


size_t Yaapt::write(const uint8_t *buffer, size_t size)
{
    return _stream.write(buffer, size);    
};

size_t Yaapt::write(const char *str)
{
    return _stream.write(str, strlen(str));    
};

size_t Yaapt::write(const uint8_t b)
{
    return _stream.write(b);
};

size_t Yaapt::write(const uint32_t u)
{
    return _stream.write((uint8_t*)(&u), sizeof(uint32_t));
};

size_t Yaapt::write(const float f)
{
    return _stream.write((uint8_t*)(&f), sizeof(float));
};

size_t Yaapt::write(const double d)
{
    return _stream.write((uint8_t*)(&d), sizeof(double));
};

/*************************************************************************/
YaaptChannel::YaaptChannel(Yaapt& y, const char* name)  : _y(y)
{
    _channel_id = y.registerChannel(this);
    if(name != NULL)
        _name = name;//std::string(name);
};

size_t  YaaptChannel::write(const uint8_t* buffer, size_t size)
{
    _y.write(_channel_id);
    _y.write(size);
    return _y.write(buffer, size);
};

size_t  YaaptChannel::write(const char* str)
{
    // include the \0 at the end for strings
    return write((uint8_t*) str, strlen(str) + 1);
};

const char* YaaptChannel::name()
{
    return _name.c_str(); //_name;
};

uint8_t YaaptChannel::channel_id()
{ 
    return _channel_id; 
};

/*************************************************************************/
ChannelType_t YLogChannel::channel_type() 
{ 
    return CHANNEL_TYPE_LOG; 
};

// <.., uint8_t:level, uint32_t linenr, str0:file, str0:functionname, str0:message>
void YLogChannel::log(const yaapt_loglevel_t level, const uint32_t linenr, const char* file, const char* function, const char* fmt, ...)
{
    _y.write((uint8_t) FIELD_CHANNEL_ID); // needed, it is not optional?
    _y.write(_channel_id);
    _y.write((uint8_t) FIELD_LOG_LEVEL); // needed, it is not optional?
    _y.write((uint8_t) level);
    _y.write((uint8_t) FIELD_LOG_LINENR);
    _y.write(linenr);
    _y.write((uint8_t) FIELD_LOG_FILENAME);
    _y.write(file);
    _y.write((uint8_t) FIELD_LOG_FUNCTION);
    _y.write(function);

#define BUFLEN 1024    
    char buf[BUFLEN];
    va_list arg;
    va_list copy;
    va_start(arg, fmt);
    va_copy(copy, arg);
    size_t len = vsnprintf(buf, BUFLEN, fmt, copy);
    va_end(copy);
    _y.write((uint8_t) FIELD_LOG_MESSAGE);
    _y.write(buf);
};

/*************************************************************************/
ChannelType_t YDataChannel::channel_type() 
{ 
    return CHANNEL_TYPE_DATA; 
};

size_t  YDataChannel::write(const uint8_t *buffer, size_t size)
{ 
    return YaaptChannel::write(buffer, size); 
};

/*************************************************************************/
ChannelType_t YPrintChannel::channel_type() 
{ 
    return CHANNEL_TYPE_TEXT; 
};

size_t YPrintChannel::write(const uint8_t* buffer, size_t size) 
{ 
    return YaaptChannel::write(buffer, size); 
};

size_t YPrintChannel::write(uint8_t c)
{ 
    return write(&c, 1); 
};


/*************************************************************************/
ChannelType_t YPlotChannel::channel_type() 
{ 
    return CHANNEL_TYPE_PLOT; 
};

void YPlotChannel::send(std::initializer_list<uint32_t> list)
{
    _y.write((uint8_t) FIELD_CHANNEL_ID); // needed, it is not optional?
    _y.write(_channel_id);

    _y.write((uint8_t) FIELD_GROUP_START);
    for(const uint32_t x : list)
    {
        _y.write((uint8_t) FIELD_UINT32);
        _y.write(x);
    };

    _y.write((uint8_t) FIELD_GROUP_END);
};

void YPlotChannel::send(std::initializer_list<int32_t> list)
{
    _y.write((uint8_t) FIELD_CHANNEL_ID); // needed, it is not optional?
    _y.write(_channel_id);

    _y.write((uint8_t) FIELD_GROUP_START);
    for(const int32_t x : list)
    {
        _y.write((uint8_t) FIELD_INT32);
        _y.write((uint32_t) x);
    };

    _y.write((uint8_t) FIELD_GROUP_END);
};

void YPlotChannel::send(std::initializer_list<float> list)
{
    _y.write((uint8_t) FIELD_CHANNEL_ID); // needed, it is not optional?
    _y.write(_channel_id);

    _y.write((uint8_t) FIELD_GROUP_START);
    for(const float f : list)
    {
        _y.write((uint8_t) FIELD_FLOAT);
        _y.write(f);
    };

    _y.write((uint8_t) FIELD_GROUP_END);
};

void YPlotChannel::send(std::initializer_list<double> list)
{
    _y.write((uint8_t) FIELD_CHANNEL_ID); // needed, it is not optional?
    _y.write(_channel_id);

    _y.write((uint8_t) FIELD_GROUP_START);
    for(const double f : list)
    {
        _y.write((uint8_t) FIELD_DOUBLE);
        _y.write(f);
    };

    _y.write((uint8_t) FIELD_GROUP_END);
};

/*************************************************************************/
Yaapt YAAPT_NAME(YAAPT_STREAM_DEVICE);
YLogChannel YAAPT_LOG_NAME(yaapt);

