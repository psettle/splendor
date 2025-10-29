#ifndef TEST_COLLECT_HPP
#define TEST_COLLECT_HPP

#include <vector>

namespace test {

class IAgentFactory;
class Episode;

std::vector<Episode> CollectEpisodes(IAgentFactory const& aLeft,
                                     IAgentFactory const& aRight,
                                     std::size_t aSampleCount,
                                     std::size_t aThreadCount);

}  // namespace test

#endif /* TEST_COLLECT_HPP */