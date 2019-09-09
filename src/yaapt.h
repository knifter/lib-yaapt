#ifndef __YAAPT_H
#define __YAAPT_H

#include <vector>

#include "yaapt_config.h"

#include <Arduino.h>

typedef enum : char {
    CHANNEL_NONE = 0x00,
    CHANNEL_LOG = 'L',
    CHANNEL_PLOT = 'P',
    CHANNEL_RAW = 'R',
    CHANNEL_COMMAND = 'C',
    CHANNEL_DATA = 'D'
} ChannelType_t;

typedef enum {
    FIELD_INIT_START = 0x00,
    FIELD_VERSION = 'V',
    FIELD_CHANNEL_DESC_START = 0x01,
        FIELD_CHANNEL_TYPE = 0x02,
        FIELD_CHANNEL_NAME = 0x03,
        FIELD_CHANNEL_CONTENTS = 0x04,
        FIELD_CHANNEL_DESC_END = 0x0E,
    FIELD_INIT_END = 0x0F,
    FIELD_STRING = 'S',
    FIELD_FLOAT = 'F',
    FIELD_DOUBLE = 'D',
    FIELD_UINT8_T = '8',
    FIELD_INT8_T = '7',
    FIELD_UINT32_T = 'U',
    FIELD_INT32_T = 'S'
} FieldType_t;

class YaaptChannel;

class Yaapt
{
    public:
        Yaapt(Print& port);
        ~Yaapt() {};

        bool    begin();
        uint8_t registerChannel(YaaptChannel* c);
        size_t  write(const uint8_t *buffer, size_t size);

    private:
        std::vector<YaaptChannel*> _channels;
        Print& _port;

        // Disable copy-constructor, assignment operator
        Yaapt(const Yaapt&);
        Yaapt& operator=(const Yaapt&);
};

class YaaptChannel
{
    public:
        YaaptChannel(Yaapt& y, const char* name);

        const char* name();
        uint8_t channel_id();
        const ChannelType_t channel_type = CHANNEL_NONE;
        const char* channel_content_descriptor = "";

    protected:
        size_t  write(const uint8_t *buffer, size_t size);

        Yaapt& _y;
        uint8_t _channel_id = 0;
        std::string _name {"(noname)" };

    private:
        // Disable copy-constructor, assignment operator
        YaaptChannel(const YaaptChannel&);
        YaaptChannel& operator=(const YaaptChannel&);
};

class YLogChannel : public YaaptChannel
{
    public:
        YLogChannel(Yaapt& y, const char* name = "log") : YaaptChannel(y, name) {};

        const ChannelType_t channel_type = CHANNEL_LOG;
        const char* channel_content_descriptor = "S"; //{FIELD_STRING, 0};

        void debug(const char* fmt, ...) __attribute__ ((format (printf, 2, 3)));

};

class YPlotChannel : public YaaptChannel
{
    public:
        YPlotChannel(Yaapt& y, const char* name) : YaaptChannel(y, name) {};

        const ChannelType_t channel_type = CHANNEL_PLOT;
        const char* channel_content_descriptor = "FF"; // {FIELD_FLOAT, FIELD_FLOAT, 0};

        void write(uint16_t, uint16_t);
};

#endif // __YAAPT.H