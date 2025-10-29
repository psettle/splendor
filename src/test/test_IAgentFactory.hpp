#ifndef TEST_IAGENTFACTORY
#define TEST_IAGENTFACTORY

#include <memory>

#include "util_General.hpp"

namespace engine {
class IAgent;
}

namespace test {

class IAgentFactory {
 public:
  virtual ~IAgentFactory() = default;

  virtual std::unique_ptr<engine::IAgent> MakeAgent(
      util::Generator& aGenerator) const = 0;
};

}  // namespace test

#endif  // TEST_IAGENTFACTORY