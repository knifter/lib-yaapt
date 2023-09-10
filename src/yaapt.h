#ifndef __YAAPT_H
#define __YAAPT_H

#include <Arduino.h>
#include <vector>
#include <initializer_list>

#include "yaapt_config.h"

// using namespace std;


/* YLog Ratatouillie */
typedef enum loglevel_t : uint8_t 
{
	LOGLEVEL_DEBUG = 'D',
	LOGLEVEL_INFO = 'I',
	LOGLEVEL_WARNING = 'W',
	LOGLEVEL_ERROR = 'E'
} yaapt_loglevel_t;

#define DBG(msg, ...)      YAAPT_LOG_NAME.log(LOGLEVEL_DEBUG,   __LINE__, __FILE__, __FUNCTION__, msg, ##__VA_ARGS__)
#define INFO(msg, ...)     YAAPT_LOG_NAME.log(LOGLEVEL_INFO,    __LINE__, __FILE__, __FUNCTION__, msg, ##__VA_ARGS__)
#define WARNING(msg, ...)  YAAPT_LOG_NAME.log(LOGLEVEL_WARNING, __LINE__, __FILE__, __FUNCTION__, msg, ##__VA_ARGS__)
#define ERROR(msg, ...)    YAAPT_LOG_NAME.log(LOGLEVEL_ERROR,   __LINE__, __FILE__, __FUNCTION__, msg, ##__VA_ARGS__)

/* Meta Data */

enum ChannelType_t : uint8_t;

class YaaptChannel;

class Yaapt
{
    public:
        Yaapt(Stream&);

        bool begin();

        // Channels only:
        uint8_t registerChannel(YaaptChannel* c);

        // exposed from serial
        size_t write(const uint8_t *buf, size_t len);
        size_t write(const char *);
        size_t write(const uint8_t);
        size_t write(const uint32_t);
        size_t write(const float);
        size_t write(const double);

    private:
        std::vector<YaaptChannel*> _channels;
        Stream& _stream;

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

    protected:
        virtual ChannelType_t channel_type() = 0;
        // virtual const char* channel_descriptor() = 0;

        size_t  write(const uint8_t *buffer, size_t size);
        size_t  write(const char* str);

        Yaapt& _y;
        uint8_t _channel_id = 0;
        std::string _name {"(noname)" };

    private:
        // Disable copy-constructor, assignment operator
        YaaptChannel(const YaaptChannel&);
        YaaptChannel& operator=(const YaaptChannel&);

        friend class Yaapt;
};

class YLogChannel : public YaaptChannel
{
    public:
        YLogChannel(Yaapt& y, const char* name = "log") : YaaptChannel(y, name) {};
        ChannelType_t channel_type();

        void log(const yaapt_loglevel_t, const uint32_t linenr, const char* file, const char* function, const char* format, ...) __attribute__ ((format (printf, 6, 7)));
};

class YDataChannel : public YaaptChannel
{
    public:
        YDataChannel(Yaapt& y, const char* name) : YaaptChannel(y, name) {};
        size_t  write(const uint8_t *buffer, size_t size);
        ChannelType_t channel_type();
};

class YPrintChannel : public YaaptChannel, public Print
{
    public:
        YPrintChannel(Yaapt& y, const char* name) : YaaptChannel(y, name) {};
        ChannelType_t channel_type();
    
    private:
        size_t write(const uint8_t* buffer, size_t size);
        size_t write(uint8_t c);
};

class YPlotChannel : public YaaptChannel
{
    public:
        YPlotChannel(Yaapt& y, const char* name) : YaaptChannel(y, name) {};
        ChannelType_t channel_type();

        void send(std::initializer_list<int32_t>);
        void send(std::initializer_list<uint32_t>);
        void send(std::initializer_list<float>);
        void send(std::initializer_list<double>);
        // void send(float);
        // void send(float, float);
        // void send(double);
        // void send(double, double);
};

extern Yaapt YAAPT_NAME;
extern YLogChannel YAAPT_LOG_NAME;

#endif // __YAAPT.H