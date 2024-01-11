#include<timer.h>
#include<systick.h>

void Timer_setup(Timer* t, uint32_t wait_time, bool auto_reset){
    t->wait_time = wait_time;
    t->auto_reset = auto_reset;
    t->end_time = systick_get_ticks() + wait_time;
    t->elapsed = false;
}

void Timer_reset(Timer* t){
    Timer_setup(t, t->wait_time, t->auto_reset);
}

bool Timer_has_elapsed(Timer* t){
    if(t->elapsed){
        return false;
    }

    uint32_t now = systick_get_ticks();
    bool has_elapsed = now >= t->end_time;
    
    if(has_elapsed){
        if(t->auto_reset){
            uint32_t drift = now - t->end_time;
            t->end_time = (now + t->wait_time) - drift;
        }else{
            t->elapsed = true;
        }
    }
    return has_elapsed;
}
