#pragma once

#include <sycl/sycl.hpp>
#include <vector>

namespace canopy::queue {

    struct queueable_base {
        virtual ~queueable_base() = default;

        explicit queueable_base(sycl::queue &queue, const std::vector<std::shared_ptr<queueable_base>> &dependencies)
            : queue_(queue), dependencies_(dependencies) {}

        explicit queueable_base(sycl::queue &queue, const std::set<std::shared_ptr<queueable_base>> &dependencies)
            : queue_(queue), dependencies_(dependencies.begin(), dependencies.end()) {}

        explicit queueable_base(sycl::queue &queue)
            : queue_(queue), dependencies_(std::vector<std::shared_ptr<queueable_base>>{}) {}

    protected:
        virtual void handle_submission(sycl::handler& cgh) = 0;

    public:
        void submit() {
            const std::vector<sycl::event> dependencies = fetch_dependencies();
            this->queued_event_ = perform_submission(dependencies);
        }

    private:
        sycl::queue &queue_;
        sycl::event queued_event_;
        std::vector<std::shared_ptr<queueable_base>> dependencies_;

        std::vector<sycl::event> fetch_dependencies() {
            std::vector<sycl::event> dep_events;
            dep_events.reserve(dependencies_.size());
            for (const std::shared_ptr<queueable_base> &dep: this->dependencies_) {
                dep_events.push_back(dep->queued_event_);
            }
            return dep_events;
        }

        sycl::event perform_submission(const std::vector<sycl::event> &dependencies) {
            return queue_.submit([&](sycl::handler &cgh) { // The actual queue submission is done once, here in the base class
              cgh.depends_on(dependencies);
              handle_submission(cgh); // Defer to the derived class for the actual commands
            });
        }
    };

    // Templated derived class
    template<typename kernel_t_, int n_dim_>
    struct ranged_kernel : queueable_base {

        ranged_kernel(
                sycl::queue &queue,
                const kernel_t_ &kernel,
                const sycl::nd_range<n_dim_> &nd_range,
                const std::vector<std::shared_ptr<queueable_base>> &dependencies)
        : queueable_base(queue, dependencies),
        kernel_(kernel), nd_range_(nd_range) {}

        ranged_kernel(
                sycl::queue &queue,
                const kernel_t_ &kernel,
                const sycl::nd_range<n_dim_> &nd_range,
                const std::set<std::shared_ptr<queueable_base>> &dependencies)
        : queueable_base(queue, dependencies),
        kernel_(kernel), nd_range_(nd_range) {}

        ranged_kernel(
                sycl::queue &queue,
                const kernel_t_ &kernel,
                const sycl::nd_range<n_dim_> &nd_range)
        : queueable_base(queue),
        kernel_(kernel), nd_range_(nd_range) {}

        protected:
            kernel_t_ kernel_;
            sycl::nd_range<n_dim_> nd_range_;

    };

    template<typename kernel_t_, int n_dim_>
    struct queueable final : ranged_kernel<kernel_t_, n_dim_> {

        queueable(
                sycl::queue &queue,
                const kernel_t_ &kernel,
                const sycl::nd_range<n_dim_> &nd_range,
                const std::vector<std::shared_ptr<queueable_base>> &dependencies)
        : ranged_kernel<kernel_t_, n_dim_>(queue, kernel, nd_range, dependencies) {}

        queueable(
                sycl::queue &queue,
                const kernel_t_ &kernel,
                const sycl::nd_range<n_dim_> &nd_range,
                const std::set<std::shared_ptr<queueable_base>> &dependencies)
        : ranged_kernel<kernel_t_, n_dim_>(queue, kernel, nd_range, dependencies) {}

        queueable(
                sycl::queue &queue,
                const kernel_t_ &kernel,
                const sycl::nd_range<n_dim_> &nd_range)
        : ranged_kernel<kernel_t_, n_dim_>(queue, kernel, nd_range) {}

        void handle_submission(sycl::handler &cgh) override {
            cgh.parallel_for(this->nd_range_, this->kernel_);
        }
    };

    template<typename kernel_t_, int n_dim_>
    struct iterable_queueable final : ranged_kernel<kernel_t_, n_dim_> {

        std::uint32_t iteration_ = 0;

        iterable_queueable(
                sycl::queue &queue,
                const kernel_t_ &kernel,
                const sycl::nd_range<n_dim_> &nd_range,
                const std::vector<std::shared_ptr<queueable_base>> &dependencies)
        : ranged_kernel<kernel_t_, n_dim_>(queue, kernel, nd_range, dependencies) {}

        iterable_queueable(
                sycl::queue &queue,
                const kernel_t_ &kernel,
                const sycl::nd_range<n_dim_> &nd_range,
                const std::set<std::shared_ptr<queueable_base>> &dependencies)
        : ranged_kernel<kernel_t_, n_dim_>(queue, kernel, nd_range, dependencies) {}

        iterable_queueable(
                sycl::queue &queue,
                const kernel_t_ &kernel,
                const sycl::nd_range<n_dim_> &nd_range)
        : ranged_kernel<kernel_t_, n_dim_>(queue, kernel, nd_range) {}

        void handle_submission(sycl::handler &cgh) override {
            const auto iteration_local = ++iteration_;
            auto kernel = this->kernel_;
            cgh.parallel_for(this->nd_range_, [=](const sycl::nd_item<n_dim_> &item) {
                kernel(item, iteration_local);
            });
        }
    };
}// namespace scram::canopy::queue
