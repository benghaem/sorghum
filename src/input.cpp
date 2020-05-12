#include "sorghum/input.h"
#include <algorithm>

IVectorGroup::IVectorGroup(){
}

void IVectorGroup::add_vector(std::vector<int>& vec){
    data_.push_back(vec);
    row_len_.push_back(vec.size());
}

IVectorGroup::block_iterator::block_iterator(IterMode im, IterDir dir, int block_size, int idx_x, int idx_y, std::vector<std::vector<int>>& data,
        const std::vector<int>& row_len):
    idx_x_(idx_x),
    idx_y_(idx_y),
    data_(data),
    row_len_(row_len),
    kIMode(im),
    kIDir(dir),
    kBlockSize(block_size){

    //we need to do some prep work to make
    //column iteration work properly

    if (kIMode == IterMode::column){
        prep_column_mode();
    }

};

void IVectorGroup::block_iterator::prep_column_mode(){

    int max_len = *(std::max_element(row_len_.cbegin(), row_len_.cend()));

    for (auto& row : data_){
        int elements_needed = max_len - row.size();

        //fill the extra spots with zero
        for (int i = 0; i < elements_needed; i++){
            row.push_back(0);
        }

        //but note that we do not modify the row_len vector
    }

    //start at the last row
    int last_row = data_.size() - 1;
    int first_row = 0;

    int start_idx;
    int end_idx;
    int step;
    if (kIDir == IterDir::reverse){
        start_idx = first_row;
        end_idx = last_row + 1;
        step = 1;
    } else {
        start_idx = last_row;
        end_idx = first_row - 1;
        step = -1;
    }

    int row_idx = start_idx;

    for (; row_idx != end_idx; row_idx += step){
        int col_cursor = row_len_[row_idx];
        //for all the elements after
        for (; col_cursor < max_len; col_cursor++){
            int new_value;

            //if this is not the first row we have processed
            if (row_idx != start_idx){
                int prev_row_idx = row_idx -1;

                //check if the previous row has a real value
                if (row_len_[prev_row_idx] <= col_cursor){
                    //the gap in the matrix continues
                    new_value = data_[prev_row_idx][col_cursor] + 1;
                } else {
                    //a gap in the matrix starts here
                    new_value = 1;
                }

            //this is the first row we have processed so set the base case
            } else {
                new_value = 1;
            }

            data_[row_idx][col_cursor] = new_value;
        }
    }
}

