#pragma once

#include "IRegisterable.hpp"

namespace pgp {

    class IRenderer : virtual public IRegisterable {
    public:
        virtual void render() = 0;
    };
}
