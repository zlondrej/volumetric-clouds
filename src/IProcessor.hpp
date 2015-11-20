#pragma once

namespace pgp {

    class IProcessor {
    public:
        virtual void step(float dt) = 0;
    };
}