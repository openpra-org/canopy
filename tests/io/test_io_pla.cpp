#include <boost/test/unit_test.hpp>

#include <cstring>
#include <fstream>
#include <vector>
#include <cstdint>

#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include "flatbuffers/flatbuffers.h"
#include "io/pla.h"

BOOST_AUTO_TEST_SUITE(IOTestsPLA)

BOOST_AUTO_TEST_CASE(test_build_and_write_pla_to_file)
{
    // Create a FlatBufferBuilder for building the buffer
    flatbuffers::FlatBufferBuilder builder(1024);

    // Products vector as defined in the set_F function
    // Encoding the product terms into bit vectors (uint8_t)
    // Values correspond to:
    // [0b01100111, 0b10011111, 0b11011011, 0b10011011, 0b00110011]
    std::vector<uint8_t> products = {0b01100111, 0b10011111, 0b11011011, 0b10011011, 0b00110011};

    // Create the products vector in the builder
    auto products_offset = builder.CreateVector(products);

    // Build the PLA table
    canopy::io::PLABuilder pla_builder(builder);

    // Set the type to DNF (Disjunctive Normal Form)
    pla_builder.add_type(canopy::io::PLAType_DNF);

    // Add the products vector to the PLA
    pla_builder.add_products(products_offset);

    // Set the number of products
    pla_builder.add_num_products(products.size());

    // Set the number of events per product (number of bits used per product term)
    pla_builder.add_num_events_per_product(8);

    // Finish the PLA table
    auto pla_offset = pla_builder.Finish();

    // Finish the buffer with the PLA as the root object and set the file identifier
    builder.Finish(pla_offset, canopy::io::PLAIdentifier());

    // Write the buffer to a file named 'pla_output.bits'
    std::ofstream ofs("pla_output.bits", std::ios::binary);
    if (!ofs) {
        BOOST_FAIL("Failed to open pla_output.bits for writing");
    } else {
        ofs.write(reinterpret_cast<const char*>(builder.GetBufferPointer()), builder.GetSize());
        ofs.close();
    }

    BOOST_CHECK(true); // The write operation succeeded
}

BOOST_AUTO_TEST_CASE(test_read_and_validate_pla_from_file)
{
    // Read the FlatBuffer from 'pla_output.bits'
    std::ifstream ifs("pla_output.bits", std::ios::binary);
    if (!ifs) {
        BOOST_FAIL("Failed to open pla_output.bits for reading");
    }

    // Get the length of the file
    ifs.seekg(0, std::ios::end);
    const size_t length = ifs.tellg();
    ifs.seekg(0, std::ios::beg);

    // Read the content into a vector
    std::vector<char> buffer(length);
    ifs.read(buffer.data(), static_cast<long>(length));
    ifs.close();

    // Verify the buffer has the correct file identifier
    if (!canopy::io::PLABufferHasIdentifier(buffer.data())) {
        BOOST_FAIL("The buffer does not have the correct file identifier 'BPLA'.");
    }

    // Get the root as a PLA object
    const canopy::io::PLA* pla = canopy::io::GetPLA(buffer.data());

    // Verify the Type
    BOOST_CHECK_EQUAL(pla->type(), canopy::io::PLAType_DNF);

    // Verify the NumProducts
    BOOST_CHECK_EQUAL(pla->num_products(), 5);

    // Verify the NumEventsPerProduct
    BOOST_CHECK_EQUAL(pla->num_events_per_product(), 8);

    // Verify the Products vector
    std::vector<uint8_t> expected_products = {0b01100111, 0b10011111, 0b11011011, 0b10011011, 0b00110011};

    BOOST_CHECK_EQUAL(pla->products()->size(), expected_products.size());

    for (size_t i = 0; i < pla->products()->size(); ++i) {
        uint8_t product = pla->products()->Get(i);
        BOOST_CHECK_EQUAL(product, expected_products[i]);
    }
}

BOOST_AUTO_TEST_CASE(test_read_pla_using_mmap)
{
    // Open the file 'pla_output.bits' for reading
    int fd = open("pla_output.bits", O_RDONLY);
    if (fd == -1) {
        BOOST_FAIL("Failed to open pla_output.bits for reading");
    }

    // Obtain the size of the file using fstat
    struct stat sb{};
    if (fstat(fd, &sb) == -1) {
        close(fd);
        BOOST_FAIL("Failed to get the file size using fstat");
    }

    size_t length = sb.st_size;

    // Memory-map the file
    void* mapped_mem = mmap(nullptr, length, PROT_READ, MAP_PRIVATE, fd, 0);
    if (mapped_mem == MAP_FAILED) {
        close(fd);
        BOOST_FAIL("Failed to mmap the file");
    }

    // Close the file descriptor; the mapping remains valid
    close(fd);

    // Cast the mapped memory to a const char* buffer
    const auto buffer = static_cast<const char*>(mapped_mem);

    // Verify the buffer has the correct file identifier
    if (!canopy::io::PLABufferHasIdentifier(buffer)) {
        munmap(mapped_mem, length);
        BOOST_FAIL("The buffer does not have the correct file identifier 'BPLA'.");
    }

    // Get the root as a PLA object
    const canopy::io::PLA* pla = canopy::io::GetPLA(buffer);

    // Verify the Type
    BOOST_CHECK_EQUAL(pla->type(), canopy::io::PLAType_DNF);

    // Verify the NumProducts
    BOOST_CHECK_EQUAL(pla->num_products(), 5);

    // Verify the NumEventsPerProduct
    BOOST_CHECK_EQUAL(pla->num_events_per_product(), 8);

    // Verify the Products vector
    const std::vector<uint8_t> expected_products = {
        0b01100111,  // 103
        0b10011111,  // 159
        0b11011011,  // 219
        0b10011011,  // 155
        0b00110011   // 51
    };

    BOOST_CHECK_EQUAL(pla->products()->size(), expected_products.size());

    for (size_t i = 0; i < pla->products()->size(); ++i) {
        uint8_t product = pla->products()->Get(i);
        BOOST_CHECK_EQUAL(product, expected_products[i]);
    }

    // Unmap the memory
    if (munmap(mapped_mem, length) == -1) {
        BOOST_FAIL("Failed to munmap the file");
    }
}

BOOST_AUTO_TEST_SUITE_END()
