#ifndef __FUEL_H__
#define __FUEL_H__

double Ratio(double fuel, double air, double n2o) {
    if (fuel == 100.0) return 0;
    return fuel / (air * 0.21 + n2o * 0.356);
}

double Fuel(double x) {
    if (x > 0.12 && x <= 0.23) {
        return 90.91 * x - 0.91;
    } else if (x > 0.23 && x <= 0.3) {
        return 428.6 * x - 78.49;
    } else if (x > 0.3 && x <= 0.4) {
        return 100.0 * x + 20;
    } else if (x > 0.4 && x <= 0.95) {
        return -90.91 * x + 96.3645;
    }
    return 0;
}

double NitroCoef(double nitro) {
    return 1.0 + nitro / 100.0;
}

double BoostCoef(double boost) {
    return boost * 0.5;
}

double Power(double fuel, double air, double n2o, double boost) {
    double fuel_oxygen_ratio = Ratio(fuel, air, n2o);
    double res = Fuel(fuel_oxygen_ratio) * NitroCoef(n2o);
    if (res)
        res += BoostCoef(boost);
    return res;
}

double fuel, air, n2o, boost, power;

void ChangeFuel(double x) {
    fuel += x;
    if (air > 0) {
        air -= x;

    } else {
        n2o -= x;
    }
    if (fuel >= 100.0) {
        n2o = 0.0;
        air = 0.0;
        fuel = 100.0;
    }
}

void ChangeAir(double x) {
    air += x;
    if (n2o == 0) {
        fuel -= x;
    } else if (fuel == 0) {
        n2o -= x;
    } else {
        x = x / 2.0;
        fuel -= x;
        n2o -= x;
    }
    if (air >= 100.0) {
        n2o = 0.0;
        fuel = 0.0;
        air = 100.0;
    }
}

void ChangeN2O(double x) {
    if (x > 0 && air > 0) {
        n2o += x;
        air -= x;
    } else if (x < 0 && air < 100) {
        n2o += x;
        air -= x;
    }
    if (n2o >= 100.0) {
        n2o = 100.0;
        fuel = 0.0;
        air = 0.0;
    }
}

void ChangeBoost(double x) {
    boost += x;
    if (boost > 100)
        boost = 100;
    if (boost < 0)
        boost = 0;
}

void WorkChangeAnimationSpeed() {
    ChangeAnimationSpeed(0, 55 - map(fuel, 0, 100, 5, 40));
    ChangeAnimationSpeed(1, 55 - map(air, 0, 100, 5, 40));
    ChangeAnimationSpeed(2, 55 - map(n2o, 0, 100, 5, 40));
}

int prev_fuel = 0;
void EncoderReadFuel() {
    int f = encoders[0].read() / encoder_step_width;
    if (prev_fuel != f) {
        afk_time = millis();
        if (prev_fuel < f) {
            if (fuel > 0) {
                ChangeFuel(-1);
                WorkChangeAnimationSpeed();
            }
        } else if (prev_fuel > f) {
            if (fuel < 100) {
                ChangeFuel(1);
                WorkChangeAnimationSpeed();
            }
        }
        prev_fuel = f;
    }
}

int prev_air = 0;
void EncoderReadAir() {
    int f = encoders[1].read() / encoder_step_width;
    if (prev_air != f) {
        afk_time = millis();
        if (prev_air < f) {
            if (air > 0) {
                ChangeAir(-1);
                WorkChangeAnimationSpeed();
            }
        } else if (prev_air > f) {
            if (air < 100) {
                ChangeAir(1);
                WorkChangeAnimationSpeed();
            }
        }
        prev_air = f;
    }
}


int prev_N2O = 0;
void EncoderReadN2O() {
    int f = encoders[2].read() / encoder_step_width;
    if (prev_N2O != f) {
        afk_time = millis();
        if (prev_N2O < f) {
            if (n2o > 0) {
                ChangeN2O(-1);
                WorkChangeAnimationSpeed();
            }
        } else if (prev_N2O > f) {
            if (n2o < 100) {
                ChangeN2O(1);
                WorkChangeAnimationSpeed();
            }
        }
        prev_N2O = f;
    }
}

int prev_boost = 0;
void EncoderReadBoost() {
    int f = encoders[3].read() / encoder_step_width;
    if (prev_boost != f) {
        afk_time = millis();
        if (prev_boost < f) {
            if (boost > 0) {
                ChangeBoost(-1);
                ChangeAnimationSpeed(3, 55 - map(boost, 0, 100, 5, 40));
            }

        } else if (prev_boost > f) {
            if (boost < 100) {
                ChangeBoost(1);
                ChangeAnimationSpeed(3, 55 - map(boost, 0, 100, 5, 40));
            }
        }
        prev_boost = f;
    }
}

bool power_lock = false;
double prev_power = 0;
void UpdatePower() {
    if (power_lock) return;
    EncoderReadFuel();
    EncoderReadAir();
    EncoderReadN2O();
    EncoderReadBoost();

    DisplayNumber(0, fuel);
    DisplayNumber(1, air);
    DisplayNumber(2, n2o);
    DisplayNumber(3, boost);

    if (!fuel) pipe_enable[0] = false; else pipe_enable[0] = true;
    if (!air) pipe_enable[1] = false; else pipe_enable[1] = true;
    if (!n2o) pipe_enable[2] = false; else pipe_enable[2] = true;
    if (!boost) pipe_enable[3] = false; else pipe_enable[3] = true;
    prev_power = power;
    power = Power(fuel, air, n2o, boost);
}

#endif