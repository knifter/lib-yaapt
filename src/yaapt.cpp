
#include "yaapt.h"
#include "yaapt_config.h"

#include <vector>


Yaapt::Yaapt(Print& port) : _port(port)
{
};

bool Yaapt::begin()
{
    _port.printf("Yaapt.begin()\n");
    _port.write(FIELD_INIT_START);
    _port.write(FIELD_VERSION);
    _port.write(YAAPT_VERSION);
    
    for(YaaptChannel* c : _channels)
    {
        _port.write(FIELD_CHANNEL_DESC_START);
        _port.write(FIELD_CHANNEL_TYPE);
        _port.write(c->channel_type);
        _port.write(FIELD_CHANNEL_NAME);
        _port.write(strlen(c->name()));                           // Max 255
        _port.write(c->name(), strlen(c->name()));
        _port.write(FIELD_CHANNEL_CONTENTS);
        _port.write(strlen(c->channel_content_descriptor));       // Max 255
        _port.write(c->channel_content_descriptor, strlen(c->channel_content_descriptor));        
        _port.write(FIELD_CHANNEL_DESC_END);

        _port.printf("\tChannel %p: %s\n", c, c->name());
    }

    return true;
};

size_t Yaapt::write(const uint8_t *buffer, size_t size)
{
    return _port.write(buffer, size);    
};

uint8_t Yaapt::registerChannel(YaaptChannel* c)
{
    // Check if channel already registered
    if(c->channel_id() > 0)
        return c->channel_id();
    
    // Add this channel to our vector
    _channels.push_back(c);
    return _channels.size();
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
    if(size > 254)
    {
        return -1;
    };

    // write channel descriptor
    _y.write(&_channel_id, 1);

    // write length
    _y.write((uint8_t*)&size, 1);

    // write data
    return _y.write(buffer, size) + 2;
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
void YLogChannel::debug(const char* fmt, ...)
{
    char buf[256];
    size_t len = snprintf(buf, 256, fmt, 5);
    write((uint8_t*) buf, len);
}

void YPlotChannel::write(const uint16_t x0, const uint16_t x1)
{
    struct
    {
        uint16_t x0;
        uint16_t x1;
    } tmp;
    tmp.x0 = x0;
    tmp.x1 = x1;
    YaaptChannel::write((uint8_t*) &tmp, sizeof(tmp));
}
