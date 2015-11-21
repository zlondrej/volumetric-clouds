#pragma once

#include "IRegisterable.hpp"

namespace pgp {

    class IProcessor : public IRegisterable {
    public:
        virtual void step(float dt) = 0;
    };
}
