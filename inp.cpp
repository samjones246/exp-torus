// on gentoo, crossdev and
//    aarch64-unknown-linux-gnu-g++ -static inp.cpp -o inp
// just works

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <errno.h>

typedef uint32_t        __u32;
typedef uint16_t        __u16;
typedef __signed__ int  __s32;

/*
struct __attribute__((aligned(1),packed)) input_event {
    __u32 time_dummy_1;
    __u32 time_dummy_2;
    __u16 type;
    __u16 code;
    __s32 value;
};
*/
struct input_event {
    struct timeval time;
    __u16 type;
    __u16 code;
    __s32 value;
};

char input_device[] = "/dev/input/event0";
int input_fd;

#define S_ALL (S_IRUSR | S_IWUSR | S_IXUSR | S_IRGRP | S_IWGRP | S_IXGRP | S_IROTH | S_IWOTH | S_IXOTH)

// uapi/linux/input-event-codes.h
#define ABS_MT_SLOT		0x2f	/* MT slot being modified */
#define ABS_MT_TOUCH_MAJOR	0x30	/* Major axis of touching ellipse */
#define ABS_MT_TOUCH_MINOR	0x31	/* Minor axis (omit if circular) */
#define ABS_MT_WIDTH_MAJOR	0x32	/* Major axis of approaching ellipse */
#define ABS_MT_WIDTH_MINOR	0x33	/* Minor axis (omit if circular) */
#define ABS_MT_ORIENTATION	0x34	/* Ellipse orientation */
#define ABS_MT_POSITION_X	0x35	/* Center X touch position */
#define ABS_MT_POSITION_Y	0x36	/* Center Y touch position */
#define ABS_MT_TOOL_TYPE	0x37	/* Type of touching device */
#define ABS_MT_BLOB_ID		0x38	/* Group a set of packets as a blob */
#define ABS_MT_TRACKING_ID	0x39	/* Unique ID of initiated contact */
#define ABS_MT_PRESSURE		0x3a	/* Pressure on contact area */
#define ABS_MT_DISTANCE		0x3b	/* Contact hover distance */
#define ABS_MT_TOOL_X		0x3c	/* Center X tool position */
#define ABS_MT_TOOL_Y		0x3d	/* Center Y tool position */

#define SYN_REPORT		0

#define BTN_TOUCH		0x14a

#define EV_SYN			0x00
#define EV_KEY			0x01
#define EV_REL			0x02
#define EV_ABS			0x03

__s32 tracking_id = 0x0f000000;

void write_event(int type, int code, int value) {
    struct input_event event;
    int ret; 
    
    memset(&event, 0, sizeof(event));

    event.type = type;
    event.code = code;
    event.value = value;

    ret = write(input_fd, &event, sizeof(event));
    if(ret < sizeof(event)) {
        fprintf(stderr, "write event failed, %s\n", strerror(errno));
        exit(-1);
    }
    
    return;
}

void swipe(int x1, int y1, int x2, int y2, float sec) {

    int x, y;
    float t;
    int dt = (int) (sec / 128.0 * 1000000);

    write_event(EV_ABS, ABS_MT_TRACKING_ID, tracking_id++);
    write_event(EV_KEY, BTN_TOUCH, 1);

    for (t=0.0; t<=1.0; t+=1.0/128.0) {
        x = x1 * (1-t) + x2 * t;
        y = y1 * (1-t) + y2 * t;

        write_event(EV_ABS, ABS_MT_POSITION_X, x);
        write_event(EV_ABS, ABS_MT_POSITION_Y, y);
        write_event(EV_ABS, ABS_MT_TOUCH_MAJOR, 0x1);
        write_event(EV_ABS, ABS_MT_PRESSURE, 0x1);
        write_event(EV_SYN, SYN_REPORT, 0);

        usleep(dt);
    }
    
    write_event(EV_KEY, BTN_TOUCH, 0);
    write_event(EV_ABS, ABS_MT_TRACKING_ID, 0xffffffff);
    write_event(EV_SYN, SYN_REPORT, 0);
    
    return;
}

void tap(int x, int y) {

    write_event(EV_ABS, ABS_MT_TRACKING_ID, tracking_id++);
    write_event(EV_KEY, BTN_TOUCH, 1);
    write_event(EV_ABS, ABS_MT_POSITION_X, x);
    write_event(EV_ABS, ABS_MT_POSITION_Y, y);
    write_event(EV_ABS, ABS_MT_TOUCH_MAJOR, 0x1);
    write_event(EV_ABS, ABS_MT_PRESSURE, 0x1);
    write_event(EV_SYN, SYN_REPORT, 0);
    usleep(1000);
    write_event(EV_KEY, BTN_TOUCH, 0);
    write_event(EV_ABS, ABS_MT_TRACKING_ID, 0xffffffff);
    write_event(EV_SYN, SYN_REPORT, 0);
}

int main (int argc, char *argv[]) {
  
    int x1,y1,x2,y2;
    float sec = 0.25;

    input_fd = open(input_device, O_RDWR, S_ALL);
    if(input_fd < 0) {
        fprintf(stderr, "could not open %s, %s\n", input_device, strerror(errno));
        return 1;
    }

    if( ! ( (strcmp(argv[1], "swipe")==0 && argc >= 5)
          ||
            (strcmp(argv[1], "tap")==0   && argc >=3) ) ) {
        fprintf(stderr, "use: %s swipe x1 y1 x2 y2 [sec]\n", argv[0]);
        fprintf(stderr, "use: %s tap x y\n", argv[0]);
        return 1;
    }

    if (strcmp(argv[1], "swipe")==0) {

        x1 = atoi(argv[2]);
        y1 = atoi(argv[3]);
        x2 = atoi(argv[4]);
        y2 = atoi(argv[5]);
        
        if(argc > 5) {
            sec = atof(argv[6]);
        }

        printf("Swiping %i %i to %i %i in %f sec\n", x1, y1, x2, y2, sec);

        swipe(x1, y1, x2, y2, sec);

    } else if (strcmp(argv[1], "tap")==0) {
        x1 = atoi(argv[2]);
        y1 = atoi(argv[3]);
        
        //printf("Tapping %i %i \n", x1, y1);

        tap(x1, y1);
    }

    return 0;
}
