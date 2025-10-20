#include "data.h"

const char *data_getIndexHTML()
{
    int _size = sizeof(data_indexHTML);
    for (int i = 0; i < sizeof(data_websiteBuffer); i++)
    {
        if (i < _size)
            data_websiteBuffer[i] = pgm_read_byte_near(data_indexHTML + i);
        else
            data_websiteBuffer[i] = 0x00;
    }
    return data_websiteBuffer;
}
const char *data_get404()
{
    int _size = sizeof(data_error404);
    for (int i = 0; i < sizeof(data_websiteBuffer); i++)
    {
        if (i < _size)
            data_websiteBuffer[i] = pgm_read_byte_near(data_error404 + i);
        else
            data_websiteBuffer[i] = 0x00;
    }
    return data_websiteBuffer;
}

const char *data_getFunctionsJS()
{
    int _size = sizeof(data_functionsJS);
    for (int i = 0; i < sizeof(data_websiteBuffer); i++)
    {
        if (i < _size)
            data_websiteBuffer[i] = pgm_read_byte_near(data_functionsJS + i);
        else
            data_websiteBuffer[i] = 0x00;
    }
    return data_websiteBuffer;
}
