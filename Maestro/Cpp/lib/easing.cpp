
#include "easing.hpp"

#include <cmath>
#define abs(x) (x >= 0 ? x : -x)

// namespace easing = easing;

#ifndef PI
#define PI 3.1415926545
#endif

double easing::inSine(double t) {
	return sin(1.5707963 * t);
}

double easing::outSine(double t) {
	return 1 + sin(1.5707963 * (--t));
}

double easing::inOutSine(double t) {
	return 0.5 * (1 + sin(3.1415926 * (t - 0.5)));
}

double easing::inQuad(double t) {
    return t * t;
}

double easing::outQuad(double t) { 
    return t * (2 - t);
}

double easing::inOutQuad(double t) {
    return t < 0.5 ? 2 * t * t : t * (4 - 2 * t) - 1;
}

double easing::inCubic(double t) {
    return t * t * t;
}

double easing::outCubic(double t) {
    return 1 + (--t) * t * t;
}

double easing::inOutCubic(double t) {
    return t < 0.5 ? 4 * t * t * t : 1 + (--t) * (2 * (--t)) * (2 * t);
}

double easing::inQuart(double t) {
    t *= t;
    return t * t;
}

double easing::outQuart(double t) {
    t = (--t) * t;
    return 1 - t * t;
}

double easing::inOutQuart(double t) {
    if(t < 0.5) {
        t *= t;
        return 8 * t * t;
    } else {
        t = (--t) * t;
        return 1 - 8 * t * t;
    }
}

double easing::inQuint(double t) {
    auto t2 = t * t;
    return t * t2 * t2;
}

double easing::outQuint(double t) {
    auto t2 = (--t) * t;
    return 1 + t * t2 * t2;
}

double easing::inOutQuint(double t) {
    auto t2 = double { 0.0 };
    if(t < 0.5) {
        t2 = t * t;
        return 16 * t * t2 * t2;
    } else {
        t2 = (--t) * t;
        return 1 + 16 * t * t2 * t2;
    }
}

double easing::inExpo(double t) {
    return (pow(2, 8 * t) - 1) / 255;
}

double easing::outExpo(double t) {
    return 1 - pow(2, -8 * t);
}

double easing::inOutExpo(double t) {
    if(t < 0.5) {
        return (pow(2, 16 * t) - 1) / 510;
    } else {
        return 1 - 0.5 * pow(2, -16 * (t - 0.5));
    }
}

double easing::inCirc(double t) {
    return 1 - sqrt(1 - t);
}

double easing::outCirc(double t) {
    return sqrt(t);
}

double easing::inOutCirc(double t) {
    if(t < 0.5) {
        return (1 - sqrt(1 - 2 * t)) * 0.5;
    } else {
        return (1 + sqrt(2 * t - 1)) * 0.5;
    }
}

double easing::inBack(double t) {
    return t * t * (2.70158 * t - 1.70158);
}

double easing::outBack(double t) {
    return 1 + (--t) * t * (2.70158 * t + 1.70158);
}

double easing::inOutBack(double t) {
    if(t < 0.5) {
        return t * t * (7 * t - 2.5) * 2;
    } else {
        return 1 + (--t) * t * 2 * (7 * t + 2.5);
    }
}

double easing::inElastic(double t) {
    auto t2 = t * t;
    return t2 * t2 * sin(t * PI * 4.5);
}

double easing::outElastic(double t) {
    auto t2 = (t - 1) * (t - 1);
    return 1 - t2 * t2 * cos(t * PI * 4.5);
}

double easing::inOutElastic(double t) {
    auto t2 = double { 0.0 };
    if(t < 0.45) {
        t2 = t * t;
        return 8 * t2 * t2 * sin(t * PI * 9);
    } else if(t < 0.55) {
        return 0.5 + 0.75 * sin(t * PI * 4);
    } else {
        t2 = (t - 1) * (t - 1);
        return 1 - 8 * t2 * t2 * sin(t * PI * 9);
    }
}

double easing::inBounce(double t) {
    return pow(2, 6 * (t - 1)) * abs(sin(t * PI * 3.5));
}

double easing::outBounce(double t) {
    return 1 - pow(2, -6 * t) * abs(cos(t * PI * 3.5));
}

double easing::inOutBounce(double t) {
    if(t < 0.5) {
        return 8 * pow(2, 8 * (t - 1)) * abs(sin(t * PI * 7));
    } else {
        return 1 - 8 * pow(2, -8 * t) * abs(sin(t * PI * 7));
    }
}
