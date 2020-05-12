#ifndef SORGHUM_INPUT_H
#define SORGHUM_INPUT_H

#include <iterator>
#include <vector>

//This is a collection of variable length vectors
//When the length of all of the vectors is the same length matrix operations
//are allowed
//
// What is the problem we are solving here:
//
// Spatial architectures have a dependency on how data is intially loaded into
// the array. Order matters which means that the user providing input needs to
// know what dataflow they want to use before they provide input examples
// this restricts the usefulness of the search.
//
// To solve this we need an input format that is flexible enough to map to any
// array configuration
//
// | x x x x x
// | x x x x
// | x x x x x x
//
// We can define some operations over these vectors
// | a b c
// | d
// | e f
//
// row iterator // b = num rows
// | a b c
// | d
// | e f
//
// column iterator // b = num cols
// | a d e
// | b f
// | c
//
//
// block row iterator // b = 1 is serialized
// | a b c | e f
// | d     |
//
// block column iterator // b=1 is serialized
// | a d e | c
// | b f   |
//
// each of these iteration strategies
// needs to be reversable
//
//
// We will only implement the two stratgies:
// block row iterator
// block column iterator
//
// as the other stratgies may be derived from this
class IVectorGroup{


    enum class IterMode {
        row,
        column
    };

    enum class IterDir {
        forward,
        reverse
    };

    class block_iterator{
        public:
            block_iterator(IterMode im, IterDir dir, int block_size, int idx_x, int idx_y, std::vector<std::vector<int>>& data, const std::vector<int>& row_len);

            block_iterator& operator++();
            bool operator==(const block_iterator& rhs);

            /* iterator traits */
            using difference_type = long;
            using value_type = int;
            using pointer = const int*;
            using reference = const int&;
            using iterator_category = std::forward_iterator_tag;

        private:

            void prep_column_mode();

            int idx_x_;
            int idx_y_;
            std::vector<std::vector<int>>& data_;
            const std::vector<int>& row_len_;
            const IterMode kIMode;
            const IterDir kIDir;
            const int kBlockSize;
    };

    public:
        IVectorGroup();
        void add_vector(std::vector<int>& vec);

    private:
        std::vector<std::vector<int>> data_;
        std::vector<int> row_len_;

};

#endif /* SORGHUM_INPUT_H */
