#pragma once

#include "entity.hpp"
namespace forge {
class registry {
   public:
    using entity = entity<std::uint64_t>;
    entity make() {
        // dont just return we must store this somewhere too remember dat.
        return entity{};
    };
};
};  // namespace forge