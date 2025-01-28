job("canopy-ci") {

    val registry = "packages-space.openpra.org/p/openpra/containers/"
    val image = "canopy"
    val remote = "$registry$image"

    host("Image Tags") {
        // use kotlinScript blocks for usage of parameters
        kotlinScript("Generate slugs") { api ->

            api.parameters["commitRef"] = api.gitRevision()
            api.parameters["gitBranch"] = api.gitBranch()

            val branchName = api.gitBranch()
                .removePrefix("refs/heads/")
                .replace(
                    Regex("[^A-Za-z0-9-]"),
                    "-"
                ) // Replace all non-alphanumeric characters except hyphens with hyphens
                .replace(Regex("-+"), "-") // Replace multiple consecutive hyphens with a single hyphen
                .lowercase() // Convert to lower case for consistency

            val maxSlugLength = if (branchName.length > 48) 48 else branchName.length
            var branchSlug = branchName.subSequence(0, maxSlugLength).toString()
            api.parameters["branchSlug"] = branchSlug

            api.parameters["isMainBranch"] = (api.gitBranch() == "refs/heads/main").toString()

        }
    }

    host("build-image") {
      shellScript {
        interpreter = "/bin/bash"
        content = """
                        docker pull $remote:{{ branchSlug }} || true
                        docker build --tag="$remote:{{ branchSlug }}" --tag="$remote:ci-{{ run:number }}-{{ branchSlug }}" .
                        docker push "$remote:ci-{{ run:number }}-{{ branchSlug }}"
                        """
      }
    }

    parallel {

        host("NVIDIA GPU Tests") {
            requirements {
                workerTags("nvidia_gpu")
            }
            shellScript("clinfo") {
                interpreter = "/bin/bash"
                content = """
                          docker pull "$remote:ci-{{ run:number }}-{{ branchSlug }}"
                          docker run --rm -e ACPP_VISIBILITY_MASK=cuda "$remote:ci-{{ run:number }}-{{ branchSlug }}" clinfo
                          """
            }
            shellScript("acpp-info") {
                interpreter = "/bin/bash"
                content = """
                          docker pull "$remote:ci-{{ run:number }}-{{ branchSlug }}"
                          docker run --rm -e ACPP_VISIBILITY_MASK=cuda "$remote:ci-{{ run:number }}-{{ branchSlug }}" acpp-info
                          """
            }

            shellScript("ctest") {
                interpreter = "/bin/bash"
                content = """
                          docker pull "$remote:ci-{{ run:number }}-{{ branchSlug }}"
                          docker run --rm -e ACPP_VISIBILITY_MASK=cuda "$remote:ci-{{ run:number }}-{{ branchSlug }}" ctest --verbose --output-on-failure || true
                          """
            }

            shellScript("run") {
                interpreter = "/bin/bash"
                content = """
                          docker pull "$remote:ci-{{ run:number }}-{{ branchSlug }}"
                          docker run --rm -e ACPP_VISIBILITY_MASK=cuda "$remote:ci-{{ run:number }}-{{ branchSlug }}" ./src/bool/bool || true
                          """
            }
        }

        host("AMD GPU Tests") {
            requirements {
                workerTags("amd_gpu")
            }
            shellScript("clinfo") {
                interpreter = "/bin/bash"
                content = """
                          docker pull "$remote:ci-{{ run:number }}-{{ branchSlug }}"
                          docker run --rm --device=/dev/dri --device=/dev/kfd --cap-add=ALL --security-opt seccomp=unconfined --group-add video -e ACPP_VISIBILITY_MASK=hip "$remote:ci-{{ run:number }}-{{ branchSlug }}" clinfo
                          """
            }
            shellScript("acpp-info") {
                interpreter = "/bin/bash"
                content = """
                          docker pull "$remote:ci-{{ run:number }}-{{ branchSlug }}"
                          docker run --rm --device=/dev/dri --device=/dev/kfd --cap-add=ALL --security-opt seccomp=unconfined --group-add video -e ACPP_VISIBILITY_MASK=hip "$remote:ci-{{ run:number }}-{{ branchSlug }}" acpp-info
                          """
            }

            shellScript("ctest") {
                interpreter = "/bin/bash"
                content = """
                          docker pull "$remote:ci-{{ run:number }}-{{ branchSlug }}"
                          docker run --rm --device=/dev/dri --device=/dev/kfd --cap-add=ALL --security-opt seccomp=unconfined --group-add video -e ACPP_VISIBILITY_MASK=hip "$remote:ci-{{ run:number }}-{{ branchSlug }}" ctest --verbose --output-on-failure || true
                          """
            }

            shellScript("run") {
                interpreter = "/bin/bash"
                content = """
                          docker pull "$remote:ci-{{ run:number }}-{{ branchSlug }}"
                          docker run --rm --device=/dev/dri --device=/dev/kfd --cap-add=ALL --security-opt seccomp=unconfined --group-add video -e ACPP_VISIBILITY_MASK=hip "$remote:ci-{{ run:number }}-{{ branchSlug }}" ./src/bool/bool || true
                          """
            }
        }

        host("OpenMP CPU Tests") {
            requirements {
                workerTags("swarm-worker")
            }
            shellScript("clinfo") {
                interpreter = "/bin/bash"
                content = """
                          docker pull "$remote:ci-{{ run:number }}-{{ branchSlug }}"
                          docker run --rm -e ACPP_VISIBILITY_MASK=omp "$remote:ci-{{ run:number }}-{{ branchSlug }}" clinfo
                          """
            }
            shellScript("acpp-info") {
                interpreter = "/bin/bash"
                content = """
                          docker pull "$remote:ci-{{ run:number }}-{{ branchSlug }}"
                          docker run --rm -e ACPP_VISIBILITY_MASK=omp "$remote:ci-{{ run:number }}-{{ branchSlug }}" acpp-info
                          """
            }

            shellScript("ctest") {
                interpreter = "/bin/bash"
                content = """
                          docker pull "$remote:ci-{{ run:number }}-{{ branchSlug }}"
                          docker run --rm -e ACPP_VISIBILITY_MASK=omp "$remote:ci-{{ run:number }}-{{ branchSlug }}" ctest --verbose --output-on-failure || true
                          """
            }

            shellScript("run") {
                interpreter = "/bin/bash"
                content = """
                          docker pull "$remote:ci-{{ run:number }}-{{ branchSlug }}"
                          docker run --rm -e ACPP_VISIBILITY_MASK=omp "$remote:ci-{{ run:number }}-{{ branchSlug }}" ./src/bool/bool || true
                          """
            }
        }

        host("OpenCL AMD CPU Tests") {
            requirements {
                workerTags("swarm-worker")
            }
            shellScript("clinfo") {
                interpreter = "/bin/bash"
                content = """
                          docker pull "$remote:ci-{{ run:number }}-{{ branchSlug }}"
                          docker run --rm -e ACPP_VISIBILITY_MASK=ocl "$remote:ci-{{ run:number }}-{{ branchSlug }}" clinfo
                          """
            }
            shellScript("acpp-info") {
                interpreter = "/bin/bash"
                content = """
                          docker pull "$remote:ci-{{ run:number }}-{{ branchSlug }}"
                          docker run --rm -e ACPP_VISIBILITY_MASK=ocl "$remote:ci-{{ run:number }}-{{ branchSlug }}" acpp-info
                          """
            }

            shellScript("ctest") {
                interpreter = "/bin/bash"
                content = """
                          docker pull "$remote:ci-{{ run:number }}-{{ branchSlug }}"
                          docker run --rm -e ACPP_VISIBILITY_MASK=ocl "$remote:ci-{{ run:number }}-{{ branchSlug }}" ctest --verbose --output-on-failure || true
                          """
            }

            shellScript("run") {
                interpreter = "/bin/bash"
                content = """
                          docker pull "$remote:ci-{{ run:number }}-{{ branchSlug }}"
                          docker run --rm -e ACPP_VISIBILITY_MASK=ocl "$remote:ci-{{ run:number }}-{{ branchSlug }}" ./src/bool/bool || true
                          """
            }
        }

        host("OpenCL Intel CPU Tests") {
            requirements {
                workerTags("intel_cpu")
            }
            shellScript("clinfo") {
                interpreter = "/bin/bash"
                content = """
                          docker pull "$remote:ci-{{ run:number }}-{{ branchSlug }}"
                          docker run --rm --device=/dev/dri -e ACPP_VISIBILITY_MASK=ocl "$remote:ci-{{ run:number }}-{{ branchSlug }}" clinfo
                          """
            }
            shellScript("acpp-info") {
                interpreter = "/bin/bash"
                content = """
                          docker pull "$remote:ci-{{ run:number }}-{{ branchSlug }}"
                          docker run --rm --device=/dev/dri -e ACPP_VISIBILITY_MASK=ocl "$remote:ci-{{ run:number }}-{{ branchSlug }}" acpp-info
                          """
            }

            shellScript("ctest") {
                interpreter = "/bin/bash"
                content = """
                          docker pull "$remote:ci-{{ run:number }}-{{ branchSlug }}"
                          docker run --rm --device=/dev/dri -e ACPP_VISIBILITY_MASK=ocl "$remote:ci-{{ run:number }}-{{ branchSlug }}" ctest --verbose --output-on-failure || true
                          """
            }

            shellScript("run") {
                interpreter = "/bin/bash"
                content = """
                          docker pull "$remote:ci-{{ run:number }}-{{ branchSlug }}"
                          docker run --rm --device=/dev/dri -e ACPP_VISIBILITY_MASK=ocl "$remote:ci-{{ run:number }}-{{ branchSlug }}" ./src/bool/bool || true
                          """
            }
        }

        host("Level Zero Intel GPU Tests") {
            requirements {
                workerTags("intel_gpu")
            }
            shellScript("clinfo") {
                interpreter = "/bin/bash"
                content = """
                          docker pull "$remote:ci-{{ run:number }}-{{ branchSlug }}"
                          docker run --rm --device=/dev/dri --e ACPP_VISIBILITY_MASK=ze "$remote:ci-{{ run:number }}-{{ branchSlug }}" clinfo
                          """
            }
            shellScript("acpp-info") {
                interpreter = "/bin/bash"
                content = """
                          docker pull "$remote:ci-{{ run:number }}-{{ branchSlug }}"
                          docker run --rm --device=/dev/dri -e ACPP_VISIBILITY_MASK=ze "$remote:ci-{{ run:number }}-{{ branchSlug }}" acpp-info
                          """
            }

            shellScript("ctest") {
                interpreter = "/bin/bash"
                content = """
                          docker pull "$remote:ci-{{ run:number }}-{{ branchSlug }}"
                          docker run --rm --device=/dev/dri -e ACPP_VISIBILITY_MASK=ze "$remote:ci-{{ run:number }}-{{ branchSlug }}" ctest --verbose --output-on-failure || true
                          """
            }

            shellScript("run") {
                interpreter = "/bin/bash"
                content = """
                          docker pull "$remote:ci-{{ run:number }}-{{ branchSlug }}"
                          docker run --rm --device=/dev/dri -e ACPP_VISIBILITY_MASK=ze "$remote:ci-{{ run:number }}-{{ branchSlug }}" ./src/bool/bool || true
                          """
            }
        }

    }
}
