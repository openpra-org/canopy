#include <cmath>

namespace Canopy::Sampler {

    template<typename float_type, typename size_type>
    class RunningStats {
    public:
        RunningStats() : m_n(0) {}

        void clear() {
            m_n = 0;
        }

        void push_back(float_type x) {
            m_n++;

            // See Knuth TAOCP vol 2, 3rd edition, page 232
            if (m_n == 1) {
                m_oldM = m_newM = x;
                m_oldS = 0.0;
            } else {
                m_newM = m_oldM + (x - m_oldM) / m_n;
                m_newS = m_oldS + (x - m_oldM) * (x - m_newM);

                // set up for next iteration
                m_oldM = m_newM;
                m_oldS = m_newS;
            }
        }

        size_type count() const {
            return m_n;
        }

        float_type mean() const {
            return (m_n > 0) ? m_newM : 0.0;
        }

        float_type variance() const {
            return ((m_n > 1) ? m_newS / (m_n - 1) : 0.0);
        }

        float_type std() const {
            return std::sqrt(variance());
        }

    private:
        size_type m_n;
        float_type m_oldM{}, m_newM{}, m_oldS{}, m_newS{};
    };

}