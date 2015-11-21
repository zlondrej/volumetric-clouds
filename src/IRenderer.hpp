#pragma once

#include "IRegisterable.hpp"

namespace pgp {

    class IRenderer : public IRegisterable {
    public:
        virtual void render() = 0;
    };
}
