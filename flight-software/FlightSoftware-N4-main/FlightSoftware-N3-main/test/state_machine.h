#ifndef STATE_MACHINE_H
#define STATE_MACHINE_H

class State_machine{
    private:

    public:
        State_machine();
        int pre_flight();
        int powered_flight();
        int coasting();
        int apogee();
        int ballistic_descent();
        int parachute_deploy();
        int post_flight();
};

#endif
