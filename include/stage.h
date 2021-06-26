#ifndef __STAGE_H__
#define __STAGE_H__


enum Stage {
    SETUP,
    WAIT,
    HALF_WAIT,
    READY,
    WORK,
    DANGER,
    SLOWING,
    SLOWING_TO_WAIT,
    SLOWING_TO_READY,
    STOP,
    VOLUME_CHANGE
};

Stage stage;


#endif