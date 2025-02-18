


#ifndef __CAMERA_SENSOR_PARA_H
#define __CAMERA_SENSOR_PARA_H

#define CAMERA_SENSOR_REG_DEFAULT_VALUE  \
    \
    {\
        \
        {\
             0x00000304,  0x00000000\
        },\
        \
        {\
             0x00000305,  0x0000000D\
        },\
        \
        {\
             0x00000306,  0x00000000\
        },\
        \
        {\
             0x00000307,  0x000000C0\
        },\
        \
        {\
             0x00000300,  0x00000000\
        },\
        \
        {\
             0x00000301,  0x00000004\
        },\
        \
        {\
             0x0000030A,  0x00000000\
        },\
        \
        {\
             0x0000030B,  0x00000002\
        },\
        \
        {\
             0x00000308,  0x00000000\
        },\
        \
        {\
             0x00000309,  0x00000008\
        },\
        \
        {\
             0xFFFFFFFF,  0x00000001\
        }\
}

#define CAMERA_SENSOR_CCT_DEFAULT_VALUE {{ 0xFFFFFFFF, 0x08 } ,{ 0x0205, 0x20 } ,{ 0x020e, 0x01 } ,{ 0x0210, 0x01 } ,{ 0x0212, 0x01 }}
#endif 
